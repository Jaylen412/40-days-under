import tempfile
import unittest
from pathlib import Path

from simulation.game import DAY_40, Game, SaveStore, State, demo


class GameplayTests(unittest.TestCase):
    def crisis(self):
        return Game(State(elapsed_hours=DAY_40, phase="CRISIS", diesel_l=34, nitrogen_load=1))

    def test_day40_broadcast_and_fuel_budget(self):
        game = Game()
        game.advance(DAY_40 - .01)
        self.assertEqual(game.s.phase, "BASE")
        game.advance(.01)
        self.assertEqual(game.s.phase, "CRISIS")
        self.assertEqual(game.s.day, 40)
        self.assertAlmostEqual(game.s.diesel_l, 34, places=6)
        self.assertEqual(sum(e["id"] == "choice.available" for e in game.events), 1)

    def test_choices_cannot_be_reversed(self):
        game = self.crisis()
        game.commit_choice("A")
        with self.assertRaises(ValueError):
            game.commit_choice("B")
        self.assertTrue(game.s.stealth_compromised)

    def test_signal_has_finite_delivery_and_population_cost(self):
        game = self.crisis()
        game.commit_choice("A")
        game.advance(6)
        self.assertEqual(game.s.population, 4)
        self.assertAlmostEqual(game.s.diesel_l, 268, places=6)
        game.advance(48)
        self.assertEqual(game.s.phase, "END_SIGNAL_RESCUED")
        self.assertAlmostEqual(game.s.diesel_l, 205.6, places=6)
        fuel = game.s.diesel_l
        game.advance(999)
        self.assertEqual(fuel, game.s.diesel_l)

    def test_solo_guards_and_complete_loop(self):
        game = self.crisis()
        game.commit_choice("B")
        with self.assertRaises(ValueError):
            game.start_deco()
        game.prepare_solo()
        game.start_deco()
        game.advance(16.99)
        with self.assertRaises(ValueError):
            game.depart_solo()
        game.advance(.01)
        game.depart_solo()
        game.advance(1.5)
        with self.assertRaises(ValueError):
            game.secure_fuel()
        game.advance(1)
        game.secure_fuel()
        game.advance(1.75)
        self.assertEqual(game.s.phase, "END_SWIM")
        self.assertAlmostEqual(game.s.diesel_l, 72.75, places=6)
        self.assertFalse(game.s.stealth_compromised)

    def test_deco_fuel_loss_invalidates_clearance(self):
        game = Game(State(phase="SOLO_DECO", location="AIRLOCK", diesel_l=.005,
                          deco_elapsed_hours=16.9, branch="B"))
        game.advance(.02)
        self.assertEqual(game.s.phase, "SOLO_PREP")
        self.assertEqual(game.s.deco_elapsed_hours, 0)
        self.assertTrue(any(e["id"] == "pressure.deco_interrupted" for e in game.events))

    def test_mid_tick_fuel_depletion_is_integrated(self):
        game = Game(State(diesel_l=.005))
        game.advance(.015)
        self.assertAlmostEqual(game.s.hypoxia, .01 / 3, places=9)
        self.assertAlmostEqual(game.s.diesel_l, 0)
        self.assertEqual(sum(e["id"] == "pump.stopped" for e in game.events), 1)

    def test_battery_reserve_delays_co2_but_not_hypoxia(self):
        game = Game(State(diesel_l=0, battery_kwh=.0038))
        game.advance(.02)
        self.assertGreater(game.s.hypoxia, 0)
        self.assertGreater(game.s.co2_stress, 0)
        self.assertAlmostEqual(game.s.battery_kwh, 0)

    def test_pause_and_bad_time(self):
        game = Game()
        game.advance(24, paused=True)
        self.assertEqual(game.s.elapsed_hours, 0)
        for value in (-1, float("nan"), float("inf")):
            with self.assertRaises(ValueError):
                game.advance(value)

    def test_death_preempts_story_transition(self):
        game = Game(State(phase="SIGNAL_DEFENSE", phase_elapsed_hours=47.99,
                          health=.001, diesel_l=0, battery_kwh=0, hypoxia=1, co2_stress=1))
        game.advance(1)
        self.assertEqual(game.s.phase, "DEAD")
        self.assertFalse(any(e["id"] == "ending.reached" for e in game.events))

    def test_transition_deadline_not_lost_with_irregular_ticks(self):
        one = self.crisis()
        many = self.crisis()
        one.commit_choice("A")
        many.commit_choice("A")
        one.advance(6.007)
        for hours in (2.003, 1.001, 3.003):
            many.advance(hours)
        self.assertEqual(one.s.phase, many.s.phase)
        self.assertAlmostEqual(one.s.phase_elapsed_hours, .007, places=7)
        self.assertAlmostEqual(one.s.diesel_l, many.s.diesel_l, places=7)

    def test_atomic_tomb_blocks_resurrection(self):
        with tempfile.TemporaryDirectory() as directory:
            store = SaveStore(Path(directory) / "save.db")
            game = Game(store=store)
            game.advance(.1)
            stale = store.load()
            game.die("test_failure")
            self.assertIsNone(store.load())
            self.assertEqual(store.db.execute("SELECT COUNT(*) FROM tomb").fetchone()[0], 1)
            with self.assertRaises(ValueError):
                store.save(stale)
            store.save(game.s)
            self.assertEqual(store.db.execute("SELECT COUNT(*) FROM tomb").fetchone()[0], 1)
            store.close()

    def test_store_enforces_single_live_slot(self):
        store = SaveStore(":memory:")
        store.save(State())
        with self.assertRaises(ValueError):
            store.save(State())
        store.close()

    def test_demo(self):
        self.assertEqual(demo()["phase"], "END_SWIM")

    def test_unsafe_ascent_has_symptom_window_then_death(self):
        game = Game(State(nitrogen_load=1))
        game.unsafe_ascent()
        self.assertFalse(game.s.terminal)
        self.assertEqual(game.s.dcs_severity, 1)
        game.advance(1 / 60)
        self.assertAlmostEqual(game.s.health, 80)
        game.advance(4 / 60)
        self.assertEqual(game.s.phase, "DEAD")
        self.assertEqual(game.s.death_reason, "decompression_injury")

    def test_completed_run_archives_and_allows_new_run(self):
        store = SaveStore(":memory:")
        state = State(phase="END_SWIM", terminal=True)
        store.save(state)
        self.assertIsNone(store.load())
        self.assertEqual(store.db.execute("SELECT COUNT(*) FROM completed").fetchone()[0], 1)
        state.phase = "BASE"
        state.terminal = False
        with self.assertRaises(ValueError):
            store.save(state)
        store.save(State())
        self.assertIsNotNone(store.load())
        store.close()

    def test_reject_invalid_saved_state(self):
        for state in (State(diesel_l=-1), State(phase="TYPO"), State(stamina=2), State(population=0)):
            with self.assertRaises(ValueError):
                Game(state)

    def test_injury_cannot_cross_success_deadline(self):
        game = Game(State(phase="SIGNAL_DEFENSE", phase_elapsed_hours=47.999))
        game.unsafe_ascent()
        game.advance(.2)
        self.assertEqual(game.s.phase, "DEAD")
        self.assertFalse(any(e["id"] == "ending.reached" for e in game.events))


if __name__ == "__main__":
    unittest.main()
