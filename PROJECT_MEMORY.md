project: "40 Days Under"
engine: "Unreal Engine 5.8 (C++20)"
dcc_tool: "Blender 4.x"
mcp_servers: ["higgsfield", "filesystem"]
last_updated_by: "Claude Fable 5.2 — Orchestrator"
timestamp: "2026-11-14T18:06:25Z"

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

## 3. Blender-to-UE5 Asset Pipeline Status

| Asset ID | Asset Name | Higgsfield Ref | Blender `.blend` | `.fbx` Exported | UE5 Imported | Status |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- |
| `CHR_Christian_01` | Christian Thomas (175.26cm) | `8.1, 8.2` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |
| `ENV_Lab04_Int` | Lab 04 Modular Bulkheads & Dome | `8.3` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |
| `PROP_ROV_Scout` | Tethered Inspection ROV | `8.4` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |
| `ENV_RiverWalk_Dock` | Detroit RiverWalk Marina | `8.5` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |

## 4. Sub-Agent Handoff Queue (Pick Up Here Next)

- **Completed This Session:** design and plan written; all five reference plates located and logged.
- **Active Blockers / Open Questions:** none yet.
- **Immediate Next Step for Incoming Model:** execute the plan from Task 1; progress ledger is in `.superpowers/sdd/2026-11-14-first-pass/progress.md` (git-ignored).
