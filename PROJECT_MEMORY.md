project: "40 Days Under"
engine: "Unreal Engine 5.8 (C++20)"
dcc_tool: "Blender 4.x"
mcp_servers: ["higgsfield", "filesystem"]
last_updated_by: "Claude Code"
timestamp: "2026-11-15T01:50:00Z"

# PROJECT_MEMORY.md — Single Source of Truth

Requirements: `scope.md`. Current design: `docs/superpowers/specs/2026-11-14-first-pass-design.md`.
Current plan: `docs/superpowers/plans/2026-11-14-first-pass.md`.

## 1. Locked Visual Anchors (Higgsfield MCP)

Local copies live in `assets/`. IDs are Higgsfield job IDs.

- **Christian Thomas Character Seed (8.1):** `[ACTIVE]` reference element `4284877d-9158-4176-98aa-f7344abff977` ("Christian Thomas — Lab 04"); turnaround sheet image job `58c39f98-e7f8-4a0e-bb18-7a3c97104f78`; local `assets/hf_20261114_120221_58c39f98-e7f8-4a0e-bb18-7a3c97104f78.png`
- **Visor Portrait Video (8.2):** `[ACTIVE]` job `673f6cc5-ad50-45f9-9975-404221147ffd`; local `assets/hf_20261114_120414_673f6cc5-ad50-45f9-9975-404221147ffd_min.mp4`
- **Lab 04 Interior Concept (8.3):** `[ACTIVE]` job `a583d3d9-57d8-4eba-9bab-cd31e6346ac9` (2688×1152); local `assets/hf_20261114_121052_a583d3d9-57d8-4eba-9bab-cd31e6346ac9.png`. Alternate, lower resolution (1344×576): job `59154831-16da-4298-a683-7efa6f4ab992`
- **Lab 04 Exterior / ROV Concept (8.4):** `[ACTIVE]` video job `e831062e-80f2-4af1-af80-0f8ea43f6e7a`, seed `1925040229`; local `assets/hf_20261114_121202_e831062e-80f2-4af1-af80-0f8ea43f6e7a_min.mp4`
- **Detroit RiverWalk Marina Concept (8.5):** `[ACTIVE]` job `1fb9d90f-195c-48cf-bf0f-bc6525146bdb` (2688×1152); local `assets/hf_20261114_121946_1fb9d90f-195c-48cf-bf0f-bc6525146bdb.png`. Alternate, lower resolution (1344×576): job `339edeac-a2c3-4408-a4c6-d8fb66d695d5`

## 2. Active Architecture & C++ Module Registry

- `[ACTIVE]` **DEC-001:** Core survival timers governed by deterministic `USurvivalClockSubsystem` + `USurvivalVitalsComponent` (Bühlmann ZHL-16 decompression + Diesel Air Clock).
- `[ACTIVE]` **DEC-002:** Zero floating HUD; 100% diegetic 3D world-space gauges, wrist dive-watch skeletal mesh, and visor material projections.
- `[ACTIVE]` **DEC-003:** The Unreal project lives in `unreal/` (`unreal/FortyDaysUnder.uproject`, module `FortyDaysUnder`).
- `[ACTIVE]` **DEC-004:** Airlock deco gas is 40% O2 / 60% N2 with a linear 17-hour pressure bleed from 15 m to the surface; under ZHL-16C this clears a saturated diver between hour 16 and hour 17.
- `[ACTIVE]` **DEC-005:** DualSense feedback uses UE's generic `UInputDeviceProperty` classes; Sony-only hooks (controller speaker) are a guarded stub until a PS5 SDK is available.
- `[ACTIVE]` **DEC-006:** Diesel tank 2000 L, burn 2 L/h: full tank runs dry on day 41.7, sputters below 15% from day 35.
- `[ACTIVE]` **DEC-007:** All survival math lives in plain C++ structs (`FBuhlmannModel`, `FAirSupplyModel`, `FBodyModel`); other systems read only `FVitalsSnapshot`.

### Implemented module registry

- `[ACTIVE]` `FBuhlmannModel`, `FAirSupplyModel`, `FBodyModel`: nitrogen tissues, diesel/air chemistry, body reserves and isolation.
- `[ACTIVE]` `FVitalsSnapshot`: Blueprint-readable contract for consumers.
- `[ACTIVE]` `USurvivalClockSubsystem`, `USurvivalVitalsComponent`: fixed-minute clock, model ownership, decompression cycle and death notification.
- `[ACTIVE]` `AUnderwaterCharacter`, `UUnderwaterMovementComponent`, `AFortyDaysGameMode`: dry-lab walking, submerged movement/current/buoyancy, camera selection, survival-driven movement and input latency.
- `[ACTIVE]` `FDualSenseFeedbackProfile`, `UDualSenseFeedbackComponent`, `FDualSenseDeviceRoute`: trigger/lightbar/pump feedback and device-routing filter. `SonyPadHooks.h` is a stub.
- `[ACTIVE]` `AOpeningSliceActor`, `FSlicePlaybackSync`: level actor that owns a `USliceDirector`, loads `Data/Narrative/opening_slice.json`, ticks it, and keeps a level sequence on the current beat. `USliceDirector` gained `BeatStartSeconds(Id)` and `TotalSeconds()`.
- `[ACTIVE]` `UAct2StateMachine`, `USliceDirector`: Act 2 branching choices, siege countdown, prep tasks, and JSON-driven Day 1 opening slice playback.

## 3. Blender-to-UE5 Asset Pipeline Status

| Asset ID | Asset Name | Higgsfield Ref | Blender `.blend` | `.fbx` Exported | UE5 Imported | Status |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- |
| `CHR_Christian_01` | Christian Thomas (175.26cm) | `8.1, 8.2` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |
| `ENV_Lab04_Int` | Lab 04 Modular Bulkheads & Dome | `8.3` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |
| `PROP_ROV_Scout` | Tethered Inspection ROV | `8.4` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |
| `ENV_RiverWalk_Dock` | Detroit RiverWalk Marina | `8.5` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |

### Opening-slice animation source (Blender)

- `[ACTIVE]` **DEC-008:** `blender/starting-animation.blend` is the export source for the Day 1 slice animation, not previs only (user decision, 2026-11-14). Blender 5.3, 30 fps, frames 1-9000 = 300 s, one timeline marker per beat named `NN_<beat id>` to match `opening_slice.json`, each bound to camera `CAM_NN_<beat id>`.
- `[ACTIVE]` **DEC-009 (verified in UE 5.8):** character animation is authored on the stock Manny skeleton exactly as Blender imports it from a UE FBX export: armature object named `root` (it becomes the UE `root` bone; do not rename it), object scale 0.01, bones in cm, rest pose facing Blender -Y. Locomotion is keyed on that object. Export with Blender FBX defaults Forward `-Z`, Up `Y`, Apply Scalings `All Local`, Add Leaf Bones off, Only Deform Bones off, armature only, Bake Animation on. Mapping is Blender (x, y, z) m to UE (100x, -100y, 100z) cm. Proof: `A_Christian_Slice_01_wake_pump.fbx` imported onto `SK_Mannequin` in a scratch project gave 599 frames, root scale 1.0, no added bones, and root/pelvis/head/hand_l positions equal to Blender at frames 0, 299, 598. This supersedes the unproven "author facing +X, Forward X / Up Z" starting value in `docs/Art/FbxExportChecklist.md` for animation on the Manny skeleton.
- Collections: `SET_Lab04_Graybox` (`GB_` graybox, not export assets), `CHR_Christian_Rig` (`root` armature with 88 Manny bones, box stand-in `CHR_Christian_Proxy`), `CAM_Shots`, `LGT_Blocking`, `FX_DomeSilhouettes`. Default startup `Collection` is excluded from the view layer, not deleted.
- Exports: `blender/export/A_Christian_Slice_NN_<beat id>.fbx`, one per beat, armature animation only. Plus `SK_Christian_Proxy.fbx` (stand-in mesh on the Manny skeleton, rest pose).
- `[ACTIVE]` Imported into the game project (2026-11-14, headless, refinement pass 1): `unreal/Content/Characters/Christian/Meshes/SK_Christian_Proxy` and `SKEL_Christian_Proxy` (own skeleton, Manny hierarchy, 180.6 cm tall bounds), and `unreal/Content/Characters/Christian/Animations/A_Christian_Slice_01..10_<beat id>`. For every beat the first and last frame root and pelvis positions in UE equal Blender under the DEC-009 mapping. Frame counts are beat seconds x 30 minus 1. The assets are not referenced by any level, Blueprint or `USliceDirector` yet, and have not been viewed in the editor viewport. This is a placeholder, so the `CHR_Christian_01` row in section 3 stays `NOT_STARTED`.
- **Frame-rate trap (found and fixed 2026-11-14):** Blender's FBX importer set the scene to 25 fps when the Manny FBX was imported, so commit `83ea477` holds animations that play 20% slow (beat 1 was 23.96 s). The scene is back at 30 fps, all ten FBX files were re-exported and re-imported, and UE now reports 19.97, 34.97, 24.97, 29.97, 29.97, 39.97, 24.97, 44.97, 29.97, 19.97 s. After any FBX import into this file, check `scene.render.fps` is still 30.
- `[ACTIVE]` Preview in UE: `unreal/Content/Maps/L_OpeningSlice_Preview` and `unreal/Content/Cinematics/LS_OpeningSlice` (30 fps, frames 0-9000). The sequence actor has Auto Play on, so the toolbar Play button runs it. The user confirmed character playback on 2026-11-14, before the set and cameras were added.
- `[ACTIVE]` Lab 04 graybox in UE (2026-11-14, built headless from `blender/export/lab04_manifest.json`): meshes and 16 flat-colour two-sided materials under `unreal/Content/Environments/Lab04/Graybox/`. `SM_Lab04_Graybox_Static` is the whole static set merged in world space (actor at the origin); `SM_Lab04_Graybox_DomeFX` is the glow plane plus silhouettes frozen at Blender frame 8000; `SM_GB_ValveWheel`, `SM_GB_GaugeNeedle` (x4) and `SM_GB_FaultLamp` (green and red copies) are separate movable actors. The template sky, sun, fog and floor were removed; four warm point lights and one green rect light above the dome replace them. Ten `CineCameraActor`s named like the Blender cameras (36 mm sensor, Blender focal lengths, focus disabled).
- Sequence additions: transform tracks for the valve wheel and four needles (sampled from Blender, linear keys), visibility tracks that swap the green and red fault lamps (red blinks every 15 frames from 3300 to 5279), a camera cut track with ten sections on the beat boundaries, and transform keys for the two moving cameras (shots 08 and 09). Imported mesh bounds equal the Blender bounds, and stills from `CAM_02` and `CAM_05` match the Blender framing. The user watched the full sequence in Play on 2026-11-14 after an editor restart and confirmed it looks right. Note: an editor that already has `LS_OpeningSlice` loaded keeps the old copy after headless edits; reloading the level alone is not enough, restart the editor.
- Graybox is not a kit: no collision was authored, nothing follows the `SM_Lab04_*` module list, and the `ENV_Lab04_Int` row in section 3 stays `NOT_STARTED`. The silhouettes do not drift in UE. The stand-in now uses `M_CHR_Proxy_Body` and `M_CHR_Proxy_WatchScreen`.
- `[ACTIVE]` **DEC-010:** the director drives the sequence, not the other way round. `LS_OpeningSlice` must keep the beats back to back in JSON order, each exactly as long as its `timeoutSeconds` (tested by `FortyDaysUnder.Narrative.BeatStartTimes`). A beat that ends on its timeout leaves playback untouched; a beat completed early through `AOpeningSliceActor::NotifyEvent` jumps the sequence to the next beat's start (tolerance 0.25 s). `L_OpeningSlice_Preview` has an `OpeningSliceDirector` actor pointing at the sequence actor, whose Auto Play is now off and Pause At End on. Verified headless in Play: `wake_pump` at 7 s with the sequence at 7.01 s; `read_manifold` at 24 s with the sequence at 24.01 s (no jump at the 20 s rollover); `NotifyEvent("GaugeRead")` moved to `check_watch` and the sequence to 55.00 s; a wrong event was rejected. Nothing in the game fires the completion events yet, and an early jump cuts the character mid-motion because the animations are one continuous performance.
- Build trap: building while the editor has the project open writes a hot-reload `libUnrealEditor-FortyDaysUnder-000N.dylib` and leaves `UnrealEditor.modules` on the old one, so headless test runs silently use stale code. Close the editor before building; check the test count went up when tests were added (now 35).
- Known gaps: the stand-in has Manny proportions (1.805 m), not Christian's 1.7526 m, until `SKEL_Christian` exists; first refinement pass only (see below); prop animation (valve wheel, needles, fault lamp), cameras and set are not exported; cot and station layout are placeholder calls, not from concept 8.3; no audio. UE logs "invalid bind poses" on import because the FBX carries no mesh.
- Refinement pass 1 (2026-11-14): root stays on the deck and the `pelvis` bone carries lying, sitting, crouching and stride bob. Wake is lie, listen, sit up, rub face, stand. Walks are four-phase steps at 1.25 m/s. Added gauge tap, watch button presses, counting gestures at the log, two-handed bus reseat then lamp reset, failed valve attempts with a step back, braced ratchet cranking that speeds up (wheel ends at about 403 degrees), a half step back under the dome, a last glance up at the close, and breathing on `spine_03`. All ten FBX files were re-exported after this pass; the UE import test predates it and was not repeated.
- Still rough: forward kinematics only, so feet are not planted (up to 3 cm through the deck at walk starts, 2 to 4 cm of float at contact); hands do not lock to props; the log recorder stays on the bench when he "lifts" it; no fingers or face.
- The Manny reference FBX was exported from the engine templates into a temp folder and is not in the repo. To regenerate: copy `Templates/TemplateResources/High/Characters/Content/Mannequins` into a blank project and export `SKM_Manny_Simple` with `SkeletalMeshExporterFBX` from an editor session (the `pythonscript` commandlet and `-nullrhi` both assert in `SkinnedMeshComponent`; use `-ExecCmds="py <script>"` with `-RenderOffscreen` and quit from the script).

## 4. Sub-Agent Handoff Queue (Pick Up Here Next)

- **Verified prior work:** Tasks 0 through 6 of the First Pass plan are committed on branch `first-pass`.
  - Task 0: Project memory files (`PROJECT_MEMORY.md`, `CLAUDE.md`, `AGENTS.md`).
  - Task 1: Survival simulation models, clock subsystem, and vitals component (`240aa76`).
  - Task 2: Underwater character, movement component, and game mode (`ab645ed`, `dec7c73`).
  - Task 3: DualSense feedback layer, device-routing filter, and tests (`5c55737`, `c2df65b`, `76a4884`).
  - Task 4: Act 2 state machine, opening-slice director, and tests (`51d04af`).
  - Task 5: Narrative world bible, opening slice script, Act 2 branches, and `opening_slice.json` (`7e9f201`).
  - Task 6: Art production sheet and FBX export checklist (`8fb6530`).
- **Task 7 status (2026-11-14):** the editor target did not build at `76a4884`. Two fixes, uncommitted: `DualSenseFeedbackComponent.cpp` called the protected `APlayerController::GetInputIndex()` (now `GetLocalPlayer()->GetControllerId()`, the same lookup), and `FortyDaysUnder.Build.cs` lacked `EngineSettings` for `UGameMapsSettings`. After them an incremental build succeeds and all `FortyDaysUnder.*` tests (33 then, 35 now) log `Result={Success}`, none `Fail`. A from-scratch clean build was not run.
- **Known limitations:** No authored Lab 04 3D level or Blender assets; no authored haptic WAVs; Sony controller-speaker hooks require PS5 SDK; real controller feel has not been verified on hardware.
- **Animation next step:** beats are wired to `USliceDirector` (DEC-010). Next either add the interactions that fire the completion events (`WokeUp`, `GaugeRead`, ...) and split the performance so early completion does not pop, or continue refining in Blender (foot and hand IK, recorder pickup) and re-import.
- **Next immediate step:** commit the two build fixes, then the next phase: authoring the graybox Lab 04 level, placing `AUnderwaterCharacter`, and implementing `AROVCompanionPawn`. Detailed prior ledger: `.superpowers/sdd/2026-11-14-first-pass/progress.md` (git-ignored).
