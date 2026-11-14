# 40 Days Under — First Implementation Pass (Design)

Date: 2026-11-14. Source of requirements: `scope.md` (repo root).

## Intent

Restart the project on Unreal Engine 5 and Blender as `scope.md` describes, replacing the
scrapped Swift/Python build. This pass delivers the items in `scope.md` section 10, steps 1–5,
without spending new Higgsfield credits.

## Goals

- A real UE 5.8 C++ project in this repo under `unreal/` (`unreal/FortyDaysUnder.uproject`,
  `unreal/Source/FortyDaysUnder/`), matching the existing `.gitignore`, that compiles on this Mac.
- `PROJECT_MEMORY.md` in the schema from `scope.md` section 1, with `CLAUDE.md` and `AGENTS.md`
  pointing at it.
- Systems code: `USurvivalClockSubsystem`, `USurvivalVitalsComponent` (diesel air clock,
  simplified Bühlmann ZHL-16 nitrogen model, 17-hour deco cycle), `AUnderwaterCharacter` with
  `UUnderwaterMovementComponent`. Automated tests on the clock and tissue math.
- DualSense layer: trigger resistance curves, haptic event mappings, lightbar ramp
  (teal → amber → pulsing crimson) driven by vitals state.
- Narrative: Act 2 Choice A/B state machine, and the opening five-minute slice inside Lab 04 as
  logic, resource formulas and dialogue script.
- Art production sheet: polycount budgets, texel density (10.24 px/cm hero, 5.12 px/cm
  environment), Manny/Quinn bone hierarchy, FBX export checklist, mapped to assets 8.1–8.5.

## Non-goals (this pass)

- No Higgsfield generations beyond 8.3 and 8.5, which the owner asked for on 2026-11-14.
- No Blender models, rigs or FBX exports.
- No built level, materials, Niagara, or audio content.
- No ROV pawn, lab subsystem manager, permadeath save wipe, or isolation meter effects beyond
  the vitals value itself.

## Anti-goals

- Another standalone prototype outside Unreal.
- Code that is written but never compiled against the engine.

## Constraints

- UE 5.8, Blender and Xcode are installed locally. Sony's PS5 SDK is not, so LibPad hooks
  cannot be built here.
- Feel and visuals cannot be judged by the agent; only compilation and automated tests can be
  verified.

## Approaches

1. **Follow section 10 in a real UE project, five sub-agents with separate folders (chosen).**
   Matches the spec's delegation rule and produces something that opens in the editor.
2. **Documents only.** Cheaper, but is not implementation.
3. **Standalone simulation first, port later.** This is what was just scrapped.

## Existing Higgsfield assets (found in the account)

| Spec asset | Status | ID |
| :--- | :--- | :--- |
| 8.1 turnaround sheet | exists | image job `58c39f98-e7f8-4a0e-bb18-7a3c97104f78`; saved as reference element `4284877d-9158-4176-98aa-f7344abff977` ("Christian Thomas — Lab 04") |
| 8.2 visor portrait video | exists | job `673f6cc5-ad50-45f9-9975-404221147ffd` |
| 8.3 Lab 04 interior | generated 2026-11-14 | image job `59154831-16da-4298-a683-7efa6f4ab992` |
| 8.4 exterior establishing video | exists | job `e831062e-80f2-4af1-af80-0f8ea43f6e7a` (seed 1925040229) |
| 8.5 RiverWalk marina | generated 2026-11-14 | image job `339edeac-a2c3-4408-a4c6-d8fb66d695d5` |

## Sub-agent ownership

| Sub-agent | Owns |
| :--- | :--- |
| 01 Narrative | `Docs/Narrative/`, `Content/Data/Narrative/` (dialogue and state-machine data) |
| 02 Systems | `Source/FortyDaysUnder/` except `Input/`; project and build files |
| 03 Hardware | `Source/FortyDaysUnder/Input/` |
| 04 Visual reference | no generation; the orchestrator logs existing IDs |
| 05 Art pipeline | `Docs/Art/` |

Sub-agent 02 defines the vitals state contract first (the public header of
`USurvivalVitalsComponent`); 01 and 03 build against it.

## Calls made by the builder, not the owner

- DualSense uses UE's generic `UInputDeviceProperty` classes; Sony-only hooks are a guarded stub.
- The 17-hour deco cycle falls out of the Bühlmann model: the airlock bleeds pressure linearly
  from 15 m to the surface over 17 hours while Christian breathes 40% oxygen. Simulated, that
  clears him between hour 16 and hour 17; on plain air it would not.
- Diesel tank 2000 L burning 2 L per hour, so a full tank runs dry on day 41.7 and drops below
  15% on day 35.
- The slice is code and data only.
- Simulation time is scaled through `USurvivalClockSubsystem` so tests can run days in seconds.

## Left to the builder

Class internals, test layout, data file formats, and module split.
