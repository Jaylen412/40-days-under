"""Deterministic fictional survival model. NOT a diving/decompression calculator."""
from __future__ import annotations

from dataclasses import asdict, dataclass, field
import json
import math
from pathlib import Path
import sqlite3
import uuid

TERMINAL = {"DEAD", "END_SIGNAL_RESCUED", "END_SIGNAL_ESCAPE", "END_SWIM"}
PHASES = TERMINAL | {"BASE", "CRISIS", "SIGNAL_RENDEZVOUS", "SIGNAL_DEFENSE", "SOLO_PREP",
                     "SOLO_DECO", "SOLO_OUTBOUND", "SOLO_DOCK", "SOLO_RETURN", "RECOMPRESSION"}
DAY_40 = 39 * 24.0
EPS = 1e-9


@dataclass
class State:
    run_id: str = field(default_factory=lambda: str(uuid.uuid4()))
    revision: int = 0
    elapsed_hours: float = 0
    phase: str = "BASE"
    branch: str | None = None
    location: str = "HABITAT"
    diesel_l: float = 970
    battery_kwh: float = 12
    water_l: float = 60
    meals: float = 700
    health: float = 100
    stamina: float = 1
    nitrogen_load: float = 0
    co2_stress: float = 0
    hypoxia: float = 0
    dcs_severity: float = 0
    isolation: float = 0
    deco_elapsed_hours: float = 0
    phase_elapsed_hours: float = 0
    stealth_compromised: bool = False
    population: int = 1
    salvage: int = 12
    umbilical_ready: bool = False
    decoy_ready: bool = False
    route_surveyed: bool = False
    fuel_rig_ready: bool = False
    carried_fuel_l: float = 0
    hull_integrity: float = 1
    terminal: bool = False
    death_reason: str | None = None

    @property
    def day(self) -> int:
        return 1 + math.floor((self.elapsed_hours + EPS) / 24)


class SaveStore:
    """One live slot, tomb records and death commit in a single SQLite transaction.

    Hosts must serialize calls on their simulation thread. This prevents accidental
    in-app resurrection, not external save editing/backup restoration.
    """
    def __init__(self, path: str | Path):
        self.db = sqlite3.connect(path)
        self.db.execute("PRAGMA synchronous=FULL")
        self.db.executescript("""
            CREATE TABLE IF NOT EXISTS live(slot INTEGER PRIMARY KEY CHECK(slot=1), run_id TEXT, revision INTEGER, body TEXT);
            CREATE TABLE IF NOT EXISTS tomb(run_id TEXT PRIMARY KEY, body TEXT NOT NULL);
            CREATE TABLE IF NOT EXISTS completed(run_id TEXT PRIMARY KEY, body TEXT NOT NULL);
        """)

    def save(self, state: State) -> None:
        body = json.dumps(asdict(state), sort_keys=True, allow_nan=False)
        with self.db:
            tomb = self.db.execute("SELECT 1 FROM tomb WHERE run_id=?", (state.run_id,)).fetchone()
            completed = self.db.execute("SELECT 1 FROM completed WHERE run_id=?", (state.run_id,)).fetchone()
            live = self.db.execute("SELECT run_id, revision FROM live WHERE slot=1").fetchone()
            if tomb:
                if state.phase == "DEAD":
                    return  # death retry is idempotent; immutable original remains
                raise ValueError("Cannot resume or overwrite a dead run")
            if completed:
                if state.phase in TERMINAL - {"DEAD"}:
                    return
                raise ValueError("Cannot resume a completed run")
            if live and live[0] != state.run_id:
                raise ValueError("A different live run already occupies the single slot")
            if live and state.revision < live[1]:
                raise ValueError("Stale save revision")
            if state.phase == "DEAD":
                self.db.execute("INSERT INTO tomb VALUES (?, ?)", (state.run_id, body))
                self.db.execute("DELETE FROM live WHERE slot=1 AND run_id=?", (state.run_id,))
            elif state.phase in TERMINAL:
                self.db.execute("INSERT INTO completed VALUES (?, ?)", (state.run_id, body))
                self.db.execute("DELETE FROM live WHERE slot=1 AND run_id=?", (state.run_id,))
            else:
                self.db.execute("INSERT OR REPLACE INTO live VALUES (1, ?, ?, ?)", (state.run_id, state.revision, body))

    def load(self) -> State | None:
        row = self.db.execute("SELECT body FROM live WHERE slot=1").fetchone()
        return State(**json.loads(row[0])) if row else None

    def close(self) -> None:
        self.db.close()


class Game:
    MAX_STEP_HOURS = 1 / 60  # one simulated minute; explicitly bounded integration

    def __init__(self, state: State | None = None, store: SaveStore | None = None):
        self.s = state or State()
        self.store = store
        self.events: list[dict] = []
        self._validate()

    def _validate(self) -> None:
        for key, value in asdict(self.s).items():
            if isinstance(value, (int, float)) and not isinstance(value, bool) and (not math.isfinite(value) or value < 0):
                raise ValueError(f"Invalid {key}")
        if self.s.phase not in PHASES or self.s.branch not in (None, "A", "B"):
            raise ValueError("Unknown phase or branch")
        if self.s.location not in {"HABITAT", "AIRLOCK", "RIVER", "SURFACE", "BOAT"}:
            raise ValueError("Unknown location")
        if not isinstance(self.s.population, int) or self.s.population < 1:
            raise ValueError("Invalid population")
        for key in ("stamina", "nitrogen_load", "co2_stress", "hypoxia", "dcs_severity", "isolation", "hull_integrity"):
            if getattr(self.s, key) > 1:
                raise ValueError(f"Invalid normalized field {key}")
        if self.s.health > 100 or self.s.deco_elapsed_hours > 17:
            raise ValueError("Invalid health/decompression range")
        if self.s.terminal and self.s.phase not in TERMINAL:
            raise ValueError("Terminal flag disagrees with phase")
        if self.s.phase in TERMINAL:
            self.s.terminal = True

    def emit(self, event: str, severity: float = 0, **payload) -> None:
        self.events.append({"id": event, "game_hour": self.s.elapsed_hours,
                            "severity": max(0, min(1, severity)), "direction": 0,
                            "source": "authoritative", "payload": payload})

    def _persist(self) -> None:
        self.s.revision += 1
        if self.store:
            self.store.save(self.s)

    def _transition(self, phase: str, location: str | None = None) -> None:
        self.s.phase = phase
        self.s.phase_elapsed_hours = 0
        if location:
            self.s.location = location
        if phase in TERMINAL:
            self.s.terminal = True
            if phase != "DEAD":
                self.emit("ending.reached", ending=phase)

    def die(self, reason: str) -> None:
        if self.s.terminal:
            return
        self.s.health = 0
        self.s.death_reason = reason
        self._transition("DEAD")
        self._persist()  # write tomb before any death presentation is emitted
        self.emit("player.died", 1, reason=reason)

    def commit_choice(self, branch: str) -> None:
        if self.s.phase != "CRISIS" or branch not in ("A", "B") or self.s.branch:
            raise ValueError("Choice requires day-40 crisis and an uncommitted A/B branch")
        self.s.branch = branch
        self.s.stealth_compromised = branch == "A"
        self._transition("SIGNAL_RENDEZVOUS" if branch == "A" else "SOLO_PREP")
        self.emit("choice.committed", branch=branch)
        self._persist()

    def prepare_solo(self) -> None:
        if self.s.phase != "SOLO_PREP" or self.s.salvage < 8:
            raise ValueError("Need solo preparation phase and 8 salvage units")
        if self.s.umbilical_ready:
            raise ValueError("Preparation already installed")
        self.s.salvage -= 8
        self.s.umbilical_ready = self.s.decoy_ready = True
        self.s.route_surveyed = self.s.fuel_rig_ready = True
        self.emit("repair.completed", system="rated_umbilical_and_fuel_sled")
        self._persist()

    def start_deco(self) -> None:
        s = self.s
        ready = s.umbilical_ready and s.decoy_ready and s.route_surveyed and s.fuel_rig_ready
        if s.phase != "SOLO_PREP" or not ready or s.diesel_l < 24 or s.battery_kwh < 2:
            raise ValueError("Need prepared route/kit, 24 L diesel runway, 2 kWh reserve")
        if s.co2_stress >= .2 or s.hypoxia >= .2 or s.health < 50 or s.dcs_severity > 0:
            raise ValueError("Life-support/health clearance failed")
        s.deco_elapsed_hours = 0
        self._transition("SOLO_DECO", "AIRLOCK")
        self.emit("pressure.deco_started")
        self._persist()

    def depart_solo(self) -> None:
        if self.s.phase != "SOLO_DECO" or self.s.deco_elapsed_hours < 17 - EPS:
            raise ValueError("Seventeen fictional game hours must complete before departure")
        if self.s.diesel_l < 7 or self.s.health < 50 or self.s.dcs_severity > 0:
            raise ValueError("Insufficient excursion reserve")
        self._transition("SOLO_OUTBOUND", "RIVER")
        self.emit("solo.departed")
        self._persist()

    def secure_fuel(self) -> None:
        if self.s.phase != "SOLO_DOCK" or self.s.phase_elapsed_hours < 1 - EPS:
            raise ValueError("Fuel transfer requires one game hour at dock")
        self.s.carried_fuel_l = 60
        self._transition("SOLO_RETURN", "RIVER")
        self.emit("solo.fuel_secured", liters=60)
        self._persist()

    def escape_trawler(self) -> None:
        if self.s.phase != "SIGNAL_DEFENSE" or self.s.phase_elapsed_hours < 2 - EPS or self.s.dcs_severity > 0:
            raise ValueError("Trawler evacuation requires 2 hours of stabilization")
        self._transition("END_SIGNAL_ESCAPE", "BOAT")
        self._persist()

    def unsafe_ascent(self) -> None:
        if self.s.terminal:
            raise ValueError("Run has ended")
        self.emit("pressure.unsafe_ascent", self.s.nitrogen_load)
        # Fictional symptom channel, not a clinical model or safe alternative.
        self.s.dcs_severity = 1
        self.s.location = "SURFACE"
        self._persist()

    def advance(self, game_hours: float, paused: bool = False) -> None:
        if not math.isfinite(game_hours) or game_hours < 0:
            raise ValueError("Time must be finite and nonnegative")
        if paused or self.s.terminal or game_hours == 0:
            return
        remaining = game_hours
        while remaining > EPS and not self.s.terminal:
            step = min(remaining, self.MAX_STEP_HOURS)
            # Split exactly at diesel depletion; never grant an entire tick of air
            # just because the pump had a positive amount at the tick's start.
            burn = 1 + .1 * (self.s.population - 1)
            if self.s.diesel_l > EPS:
                step = min(step, self.s.diesel_l / burn)
            if self.s.phase == "BASE" and self.s.elapsed_hours < DAY_40:
                step = min(step, DAY_40 - self.s.elapsed_hours)
            deadlines = {"SIGNAL_RENDEZVOUS": 6, "SIGNAL_DEFENSE": 48,
                         "SOLO_OUTBOUND": 1.5, "SOLO_RETURN": 1.5, "RECOMPRESSION": .25}
            if self.s.phase in deadlines and self.s.dcs_severity == 0:
                step = min(step, max(EPS, deadlines[self.s.phase] - self.s.phase_elapsed_hours))
            if self.s.phase == "SOLO_DECO" and self.s.deco_elapsed_hours < 17 - EPS:
                step = min(step, 17 - self.s.deco_elapsed_hours)
            self._tick(step, burn)
            remaining -= step
        self._persist()

    def _tick(self, dt: float, burn: float) -> None:
        s = self.s
        s.elapsed_hours += dt
        pump_on = s.diesel_l > EPS
        fuel_before = s.diesel_l
        s.diesel_l = max(0, s.diesel_l - burn * dt) if pump_on else 0
        solar_kw = .1  # reference weather input; production multiplies by cloud/occlusion
        demand_kw = .48 + .04 * (s.population - 1)
        charge_kw = (.55 if pump_on else 0) + solar_kw
        net_kw = charge_kw - demand_kw
        powered_dt = dt if net_kw >= 0 else min(dt, s.battery_kwh / -net_kw)
        s.battery_kwh = max(0, min(12, s.battery_kwh + net_kw * dt))
        scrubber_off_dt = dt - powered_dt
        s.co2_stress = max(0, min(1, s.co2_stress + scrubber_off_dt / 2 - powered_dt / 4))
        s.hypoxia = max(0, min(1, s.hypoxia + (-dt / 2 if pump_on else dt / 3)))
        # Water production requires circulation power; freshwater treatment, not desalination.
        water_delta = 20 / 24 * powered_dt - 3 * s.population / 24 * dt
        s.water_l = max(0, min(100, s.water_l + water_delta))
        s.meals = max(0, s.meals - 3 * s.population / 24 * dt)
        s.isolation = max(0, min(1, s.isolation + (dt / 960 if s.population == 1 else -dt / 48)))
        if s.phase == "SOLO_DECO":
            if pump_on and powered_dt >= dt - EPS and s.co2_stress < .2 and s.hypoxia < .2:
                before = s.deco_elapsed_hours
                s.deco_elapsed_hours = min(17, before + dt)
                s.nitrogen_load = max(0, 1 - s.deco_elapsed_hours / 17)
                if before < 17 - EPS <= s.deco_elapsed_hours:
                    self.emit("pressure.deco_complete")
            else:
                s.deco_elapsed_hours = 0
                self._transition("SOLO_PREP", "HABITAT")
                self.emit("pressure.deco_interrupted", 1)
        elif s.location == "HABITAT":
            s.nitrogen_load = 1 - (1 - s.nitrogen_load) * math.exp(-dt / 8)
        # Excursion abstracts sealed one-atmosphere capsule transfer to a shallow
        # route after decompression. Return capsule docks before repressurization;
        # there is no open-door deep exposure or second deep ascent hidden here.
        damage = (max(0, s.hypoxia - .5) * 80 + max(0, s.co2_stress - .4) * 80) * dt
        damage += s.dcs_severity * 1200 * dt  # 100 HP / five game minutes
        damage += (2 if s.water_l <= EPS else 0) * dt
        damage += (.25 if s.meals <= EPS else 0) * dt
        if s.phase == "SIGNAL_DEFENSE":
            s.hull_integrity = max(0, s.hull_integrity - .006 * dt)
        if s.hull_integrity <= 0:
            damage += 100 * dt
        s.health = max(0, s.health - damage)
        s.phase_elapsed_hours += dt
        if fuel_before >= 145.5 > s.diesel_l:
            self.emit("pump.low_fuel", .5)
        if pump_on and s.diesel_l <= EPS:
            self.emit("pump.stopped", 1)
        if s.health <= EPS:
            self.die("decompression_injury" if s.dcs_severity > 0 else
                     "life_support_failure" if s.hull_integrity > 0 else "flooding")
            return
        if s.dcs_severity > 0:
            return  # cannot bypass a lethal symptom sequence via a success deadline
        if s.phase == "BASE" and s.elapsed_hours >= DAY_40 - EPS:
            self._transition("CRISIS")
            self.emit("radio.message", message="family_trawler_distress")
            self.emit("choice.available")
        elif s.phase == "SIGNAL_RENDEZVOUS" and s.phase_elapsed_hours >= 6 - EPS:
            s.diesel_l += 240
            s.population = 4
            self._transition("SIGNAL_DEFENSE")
            self.emit("family.arrived", added_people=3, diesel_l=240)
            self.emit("defense.started")
        elif s.phase == "SIGNAL_DEFENSE" and s.phase_elapsed_hours >= 48 - EPS:
            self.emit("defense.completed")
            self._transition("END_SIGNAL_RESCUED")
        elif s.phase == "SOLO_OUTBOUND" and s.phase_elapsed_hours >= 1.5 - EPS:
            self._transition("SOLO_DOCK", "SURFACE")
        elif s.phase == "SOLO_RETURN" and s.phase_elapsed_hours >= 1.5 - EPS:
            s.diesel_l += s.carried_fuel_l
            s.carried_fuel_l = 0
            self._transition("RECOMPRESSION", "AIRLOCK")
        elif s.phase == "RECOMPRESSION" and s.phase_elapsed_hours >= .25 - EPS:
            self.emit("pressure.recompression_complete")
            self._transition("END_SWIM", "HABITAT")


def demo() -> dict:
    game = Game()
    game.advance(DAY_40)
    game.commit_choice("B")
    game.prepare_solo()
    game.start_deco()
    game.advance(17)
    game.depart_solo()
    game.advance(2.5)
    game.secure_fuel()
    game.advance(1.75)
    return {"phase": game.s.phase, "day": game.s.day,
            "diesel_l": round(game.s.diesel_l, 3), "events": len(game.events)}


if __name__ == "__main__":
    print(json.dumps(demo(), indent=2))
