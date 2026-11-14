# 40 Days Under

Production design and executable reference systems for the Detroit River survival game starring Christian Thomas. Target: native macOS on the user’s M3 Pro MacBook, with PS5/DualSense controls as the primary input scheme.

## Deliverables

| Package | Entry point |
|---|---|
| Dependency graph, shared decisions and engine integration gates | [Orchestration](production/ORCHESTRATION.md) |
| Canonical gameplay fields, time units and event envelope | [State contract](production/state_contract.json) |
| Act 2 state machine, resource formulas and balance | [Systems](production/SYSTEMS.md) |
| Executable survival and single-save persistence model | [Python simulation](simulation/game.py) |
| DualSense event mapping and portable C++ implementation | [Hardware](production/HARDWARE.md) / [JSON mapping](production/dualsense_events.json) |
| Opening five-minute script, dialogue and branch narrative | [Narrative](production/NARRATIVE.md) / [Scene data](production/vertical_slice.json) |
| Higgsfield Assets 7.1–7.5, original prompts and generation status | [Asset manifest](production/assets/manifest.json) / [Pipeline](production/assets/PIPELINE.md) |

## Run the reference simulation

From this directory, using Python 3.10 or newer:

```sh
python3 -m simulation.game
python3 -m unittest discover -s simulation -p 'test_*.py' -v
```

The demo advances to the Day 40 choice, completes the solo branch and prints its ending and finite remaining diesel. The tests exercise both branches, clock boundaries, life-support failures and persistence. See the hardware guide for C++ build and verification commands.

## Current delivery status

This is a production specification with runnable reference logic, not yet a playable macOS game. The opening is an interaction script ready for greyboxing. Keep PS5/DualSense button mappings on Mac; a PS5 console build or Sony console SDK is not required by this target. The controller adapter needs a macOS engine backend and device testing. Advanced feedback is capability-gated; see [Mac target and controls](production/MAC_TARGET.md).

The user approved free generations for all eligible Higgsfield assets. The asset manifest records current job status and the identity anchor as generation proceeds in strict 7.1 → 7.2 → 7.3 → 7.4 → 7.5 order. Concept images and motion references are separate from rigged production meshes.

All decompression, exposure and emergency timing in this game is fictional balancing, not diving guidance. The ambient-pressure premise and other factual corrections are documented with sources in the orchestration file.
