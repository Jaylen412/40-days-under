# DualSense feedback implementation contract

`dualsense_events.json` is the editable tuning source. `hardware/generate_profiles.py` verifies canonical event coverage and generates the C++ table consumed by `hardware/feedback.hpp`. The included mixer is portable C++17 with an application-owned `IFeedbackDevice` interface. It does **not** contain Sony SDK calls, device packets, certified actuator limits, waveform files, or a PS5 executable. The current target is macOS on an M3 Pro MacBook with PS5/DualSense controls retained. A macOS engine backend must translate supported capabilities; no PS5 console SDK is required. See MAC_TARGET.md.

## Event mapping

All intensities are normalized design gains. They are not Newtons or calibrated actuator displacement. Duration is an expiry deadline in **wall milliseconds**, independent of accelerated game time. Event severity scales gain. Direction −1/0/+1 pans the cue left/center/right across the grips; it does not physically reproduce water drag or locate sound in 3D.

| Canonical event | L2 / R2 effect and maximum gain | Haptic cue / gain | Speaker; light | Priority / TTL ms |
|---|---|---|---|---|
| `pump.nominal` | None | Pump thump .16; 1.2 Hz envelope | Teal | 10 / 1200 |
| `pump.low_fuel` | None | Sputter .22; deterministic irregular gaps | Amber | 45 / 1200 |
| `pump.stopped` | None | Pump stop .35 | Pump stopped; crimson | 90 / 2500 |
| `air.co2_warning`, `air.hypoxia_warning` | None | Alarm double .42; 1 Hz | Distinct CO2/O2 alarm; crimson | 100 / 1200 |
| `speargun.load` | Ramp .15 / .65 | Pneumatic ratchet .20 | None | 60 / 350 |
| `speargun.fire` | Break .05 / .80 | Pneumatic release .55 | None | 70 / 140 |
| `valve.turn` | Detent .50 / .50 | Valve ratchet .23 | None | 60 / 350 |
| `stamina.low` | Chug .35 / .55 | None | Amber | 40 / 700 |
| `pressure.unsafe_ascent` | Chug .65 / .75 | Alarm double .40 | Decompression alarm; crimson | 100 / 1200 |
| `pressure.deco_started`, `pressure.deco_progress` | None | None | Start message / amber status | 55 / 1500; 35 / 1200 |
| `pressure.deco_complete` | None | None | Completion; teal | 55 / 1800 |
| `pressure.deco_interrupted` | None | Alarm double .35 | Interrupted; crimson | 95 / 2500 |
| `current.drag` | None | Current texture .18, directional | None | 15 / 600 |
| `hull.groan`, `hull.breach` | None | Resonance .24 / impact .65 | Breach alarm on breach; crimson | 25 / 2500; 95 / 2000 |
| `radio.message` | None | None | Radio line | 50 / 5000 |
| `watch.alarm` | None | Watch tick .22 | Watch alarm; amber | 90 / 1500 |
| `breathing.cycle` | None | None | Regulator breath | 5 / 1200 |
| `isolation.phantom_radio` | None | None | Phantom radio | 5 / 3000 |
| `horde.nearby` | None | Proximity pulse .30 | Watch proximity warning; crimson | 80 / 1200 |
| `player.died`, `ending.reached` | Release all | Stop all | Cancel queued controller cues | Immediate lifecycle reset |

The JSON also defines repair, drone, health and recompression cues. Every remaining canonical event has an explicit zero-output row. `isolation.input_jitter` deliberately has no controller implementation: simulated motor-control difficulty belongs to an optional gameplay accessibility feature, never an accidental input-driver mutation.

## Runtime integration

1. A gameplay adapter reads the authoritative state and emits the canonical IDs with severity in [0,1], direction in [−1,1] and source classification. Convert to `Submit(id, monotonicWallMs, severity, direction, isPresentation)`. The event's `game_hour` remains for logs; do not use it as feedback time.
2. Refresh sustained state cues every 250 ms while true. Fuel warning follows the canonical simulation threshold: below 145.5 L, or 15% of the initial 970 L reserve. Initial detection and refill transitions are simulation responsibilities; do not recalculate this against a different tank capacity in the feedback adapter. Refresh `pressure.unsafe_ascent` from ongoing DCS symptom severity, not solely the one-time ascent transition. Clear a condition with `Release(id)` immediately and call `Tick` every rendered frame.
3. Refresh load/valve cues only while their mapped action is held, with severity derived from pneumatic load or valve torque. Cancel on release, object loss or action interruption. Stamina chug is a lower-priority fallback; a deliberate weapon or valve effect takes precedence, while severe decompression symptoms override both.
4. `Tick(monotonicWallMs)` returns and sends a complete frame. Implement `IFeedbackDevice::Apply` so zero outputs explicitly release hardware state. The backend chooses actual authored trigger curves and haptic waveforms by the symbolic modes/assets; these are not SDK enum names.
5. Route application pause, lost focus, controller disconnect and player handoff to `SetActive(false)`. This calls `StopAll` immediately and clears queued cues and timers. Resume/reconnect with `SetActive(true)` and re-emit current authoritative conditions; never replay old events. Death latches the mixer inactive until an explicit new-run `BeginRun()`; it cannot be cleared by a reconnect. An ending disables output as well.

Pump nominal/low/stop events replace each other. Other resolved states must be explicitly released; TTL is a fail-safe if the producer disappears. Same-event refresh extends expiry without resetting pulse phase. Backward timestamps, unknown events and presentation-sourced critical alarms are rejected. Non-finite telemetry becomes neutral; numeric gains and direction are clamped. This mixer never changes health, saves, nitrogen, stamina or input response.

## Arbitration and sensory behavior

Each channel chooses its highest-priority active profile, with event ID as a stable tie-breaker. Gains are not summed, preventing simultaneous hull, pump and current cues from exceeding the authored ceiling. Trigger, haptic and light winners may be different events. The sample envelopes express cadence only: production audio/haptic assets must supply the felt texture. A controller cannot reproduce literal sub-bass pressure or underwater force.

Nominal teal RGB is (.02, .28, .30), amber is (.65, .25, .01), and crimson is (.65, .01, .04). State colors interpolate with a 180 ms time constant; crimson has a gentle 1 Hz brightness modulation. Expiry, disabling and lifecycle resets clear light immediately. Critical audio and haptic cues do not wait for the color interpolation.

The controller speaker plays at most one selected voice from each frame. Per-event cooldowns suppress duplicate state notifications: 6 seconds for life-support alarms, 20 seconds for phantom radio, 2 seconds for regulator breath. Expired or explicitly released pending cues are discarded. A current critical condition suppresses phantom speech; critical speech wins over normal radio. All accepted cues carry main-audio and caption requests, so critical information is never speaker-only. The game audio layer resolves cue IDs to localized content and subtitles, performs line-level deduplication, and mixes main-channel sound. `radio_line` is an asset slot, not supplied recorded dialogue.

## Accessibility and platform gate

Independent trigger, haptic, speaker and light toggles and strength scales are available in `Settings`. Disabling them never suppresses main-audio/caption requests or changes the simulation. The shipped game must also provide visible diegetic alarm indicators, readable gauge modes, remapping, an input-jitter toggle and alternatives to color-only signals. Respect system-level intensity and mute preferences in the macOS backend. UI pause must release resistance, not merely pause audio.

Validate on actual controllers: held-trigger transitions, disconnect/reconnect, headset/main-audio routing, settings changes, prolonged play comfort, competing alerts, and all accessibility channels disabled. The target is a Mac game played with PS5 controls, not a PS5 console release. Validate each optional feedback channel on the actual macOS backend and connection; the desktop C++ tests establish only application behavior.

Public feature grounding: [PlayStation's PS5 Ultimate FAQ](https://blog.playstation.com/2020/11/09/ps5-the-ultimate-faq/) describes DualSense adaptive triggers, haptic feedback, integrated speaker and user-adjustable feedback strength. It does not publish the application interfaces or physical tuning used here.

## Verification

From the repository root:

```sh
python3 hardware/generate_profiles.py --check
c++ -std=c++17 -Wall -Wextra -Werror -pedantic hardware/test_feedback.cpp -o /private/tmp/lab04-feedback-test
/private/tmp/lab04-feedback-test
```

The test covers priority, smooth color progression, source classification, expiry, pending-audio cancellation, duplicate alarms, bounded/NaN inputs, disabled-channel fallbacks, pause, death latch, ending reset and rejection of stale timestamps. To update tuning, edit JSON, run `python3 hardware/generate_profiles.py`, then repeat verification. A generated table without JSON parity is not an approved deliverable.
