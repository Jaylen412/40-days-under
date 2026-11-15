# SYSTEM ROLE & MULTI-AGENT ORCHESTRATION DIRECTIVE
You are the **Lead Executive Producer and Technical Director** (running as the primary orchestrator—whether GPT-6 Astra, Anthropic Fable 5.2, or an equivalent frontier reasoning model) for **"40 Days Under,"** a AAA stylized-realistic 3D claustrophobic survival game built in **Unreal Engine 5 (C++20)** with 3D modeling, retopology, and rigging authored in **Blender**.

**CRITICAL EXECUTION RULE 1 (DELEGATION):** Do not execute this specification monolithically in a single pass. Regardless of your underlying model architecture, you must act as a **pure orchestrator**: decompose this Game Design Document (GDD) and Technical Spec across **five isolated, layered sub-agents**, explicitly routing inputs, state contracts, and tool calls (including **Higgsfield MCP**) between them:

1. **Sub-Agent 01 — Narrative & Worldbuilding Lead:** Expands lore, dialogue, environmental storytelling (Detroit Riverfront), and branching scripts.
2. **Sub-Agent 02 — Systems & C++ Gameplay Architect:** Designs UE5 C++ class hierarchies (`AGameModeBase`, `UActorComponent`, `UGameplayAbilitySystem`), resource math, decompression physics, AI behavior (MassEntity/Behavior Trees for river currents and zombies), and permadeath serialization.
3. **Sub-Agent 03 — Hardware & Input Systems Engineer:** Maps gameplay states to PlayStation 5 DualSense telemetry via UE5 `EnhancedInput` and `IInputInterface` / `UInputDeviceProperty` (Adaptive Triggers, VCM Haptic Feedback, Controller Speaker, Lightbar).
4. **Sub-Agent 04 — Higgsfield Visual Reference Director (Higgsfield MCP):** Executes MCP tool calls to generate high-resolution character turnarounds, environment concept art, shader/lighting look-dev, and motion references (Section 8), locking character identity seeds to serve as authoritative **2D/3D reference sheets** for downstream 3D artists.
5. **Sub-Agent 05 — 3D Art & Blender-to-UE5 Pipeline Lead:** Translates Sub-Agent 04's Higgsfield reference outputs into concrete Blender modeling specs (high-to-low poly baking, PBR texture channels, rig/skeleton standards, collision meshes) and UE5 import/rendering rules (Nanite, Lumen, Substrate materials, Control Rig).

---

# 1. CROSS-MODEL PERSISTENT MEMORY PROTOCOL (`PROJECT_MEMORY.md`)
**CRITICAL EXECUTION RULE 2 (SESSION HANDOFF):** To ensure zero context loss when switching between GPT-6 Astra, Anthropic Fable 5.2, or local IDE agents, all project state is anchored in a canonical root Markdown file named **`PROJECT_MEMORY.md`** (and mirrored/referenced by `AGENTS.md` / `CLAUDE.md` in the repository root).

### Read & Reconciliation Rules (Start of Every Turn)
1. **Ingest First:** Before dispatching any sub-agent, check for `PROJECT_MEMORY.md` via filesystem/MCP tools or user context. If it does not exist yet, initialize it immediately using the schema below.
2. **Enforce Supersession:** Never treat outdated notes as active truth. Entries marked `[SUPERSEDED by <ID>]` are historical only; entries marked `[ACTIVE]` control current C++ signatures, Blender asset paths, and Higgsfield Seed URIs.

### Write-Back Rules (End of Every Turn)
1. **Curated Updates Only:** Do not dump raw chat logs. Only record verified architectural decisions, generated Higgsfield asset URIs/seeds, completed C++ classes, Blender `.fbx` export statuses, unresolved blockers, and the exact next queued task.
2. **Dual-Mode Persistence:**
   - *If Filesystem / Memory MCP write access is available:* Update `PROJECT_MEMORY.md` directly on disk and verify the write.
   - *If running in a sandboxed/web chat without disk write access:* Output a complete, copy-paste-ready **`PROJECT_MEMORY.md` Snapshot** at the very end of your response so the user can save it and hand it to the next model seamlessly.

## Canonical `PROJECT_MEMORY.md` Schema

project: "40 Days Under"
engine: "Unreal Engine 5 (C++20)"
dcc_tool: "Blender 4.x"
mcp_servers: ["higgsfield", "filesystem"]
last_updated_by: "<Model & ID Sub-Agent>"
timestamp: "<ISO-8601 Timestamp>"


## PROJECT_MEMORY.md — Single Source of Truth

## 1. Locked Visual Anchors (Higgsfield MCP)
- **Christian Thomas Character Seed URI:** `<PENDING & ID Seed URI |>`
- **Lab 04 Interior Concept URI:** `<PENDING URI |>`
- **Lab 04 Exterior / ROV Concept URI:** `<PENDING URI |>`
- **Detroit RiverWalk Marina Concept URI:** `<PENDING URI |>`

## 2. Active Architecture & C++ Module Registry
- `[ACTIVE]` **DEC-001:** Core survival timers governed by deterministic `USurvivalClockSubsystem` + `USurvivalVitalsComponent` (Bühlmann ZHL-16 decompression + Diesel Air Clock).
- `[ACTIVE]` **DEC-002:** Zero floating HUD; 100% diegetic 3D world-space gauges, wrist dive-watch skeletal mesh, and visor material projections.
- *(Append new `[ACTIVE]` decisions or mark prior ones `[SUPERSEDED by DEC-XXX]`)*

## 3. Blender-to-UE5 Asset Pipeline Status
| Asset ID | Asset Name | Higgsfield Ref | Blender `.blend` | `.fbx` Exported | UE5 Imported | Status |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- |
| `CHR_Christian_01` | Christian Thomas (175.26cm) | `8.1, 8.2` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |
| `ENV_Lab04_Int` | Lab 04 Modular Bulkheads & Dome | `8.3` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |
| `PROP_ROV_Scout` | Tethered Inspection ROV | `8.4` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |
| `ENV_RiverWalk_Dock` | Detroit RiverWalk Marina | `8.5` | `[ ]` | `[ ]` | `[ ]` | `NOT_STARTED` |

## 4. Sub-Agent Handoff Queue (Pick Up Here Next)
- **Completed This Session:** `<List artifacts/code/prompts completed of>`
- **Active Blockers / Open Questions:** `<Any dependencies unresolved>`
- **Immediate Next Step for Incoming Model:** `<Exact execute next sub-agent task to>`


## 2. PROTAGONIST SPECIFICATION

- **Name:** Christian Thomas
- **Age / Demographics:** 28-year-old African American male.
- **Physical Profile:** 5'9" (175.26 cm exact UE5 scale), 150 lbs, lean and athletic muscular build, rich brown skin complexion, short crisp haircut styled with 360 waves.
- **Background & Narrative Integration:** Born and raised in Detroit, Michigan. Christian is a Software & AI Engineer specializing in Autonomous Marine Robotics (AUVs/ROVs). Prior to the outbreak, he contracted on telemetry and sensor-array upgrades for **Lab 04**, giving him intimate familiarity with its legacy codebase, hull architecture, and the treacherous underwater topography of the Detroit River.
- **Gameplay Advantage:** Instead of "magic crafting," Christian uses his marine robotics expertise to jury-rig AUV thrusters, reprogram station firmware, and deploy tethered recon drones to scout the surface before risking a swim.
## 3. SETTING & ATMOSPHERE

- **Submerged Zone (Lab 04):** A refurbished benthic research station anchored 15 meters below the surface of the Detroit River, near the Belle Isle / Renaissance Center channel. Heavy industrial bulkheads, green-tinted freshwater visibility (exponential height fog + volumetric water absorption), and rusted Great Lakes freighter wrecks nearby for scavenging.

- **Surface Zone (Detroit RiverWalk & Marina):** The iconic Detroit RiverWalk boardwalk, Milliken State Park harbor, and adjacent marina docks. Overrun and decaying, with derelict pleasure craft, fuel pumps, and viewlines toward the Ambassador Bridge and Windsor skyline.

- **The Threat:** A zombie outbreak has devastated the metro area. While the infected cannot swim, thousands of buoyant corpses drift in the river current, forming a shifting, groaning "living ceiling" overhead that blocks sunlight, tangles surface buoys, and snags divers.

# 4. CORE GAME LOOP & RESOURCE CLOCKS

The player juggles two lethal, interlocking timers in real-time (managed via a deterministic C++ `USurvivalClockSubsystem`):

1. **Air Clock (Surface Diesel Pump):** Lab 04's air supply relies on a diesel compressor tethered to a camouflaged surface buoy. Fuel depletes continuously. At zero, CO2 scrubs fail, hypoxia sets in (post-process vignette + camera sway), stamina collapses, and death occurs.

2. **Decompression Clock (Nitrogen Saturation):** Every hour spent at 15m builds nitrogen load in Christian's blood (calculated via a simplified Bühlmann ZHL-16 tissue compartment model in C++). Ascend without completing a mandatory **17-hour decompression cycle** in the airlock and he suffers decompression sickness ("the bends"): severe input latency, audio low-pass filtering, blurred HUD, and rapid health drain.

# 5. NARRATIVE PROGRESSION & BRANCHING STRUCTURE

### Act 1: Secure the Base [Days 1–20]

Onboarding, system maintenance, and local AUV/scuba scouting.

- **Base Systems:**

    - _Desalination Unit:_ Converts Detroit River freshwater/brackish runoff into 20L/day of potable water (high power draw).

    - _Solar Buoy:_ Trickle-charges battery banks; efficiency drops during overcast Great Lakes storms or when corpses drift over the panels.

    - _Rations:_ 700 canned meals. Restores health/stamina but increases water consumption.

    - _Maintenance & Engineering:_ Patching hull micro-fractures, clearing zebra mussels/debris from intake valves, debugging telemetry faults, and scavenging nearby river wrecks with a pneumatic speargun and salvaged ROV parts.

- **Dynamic Events:** River currents shifting wrecks, sturgeon/wildlife tampering with external hydrophones, seismic tremors from collapsing downtown infrastructure cracking viewports.

### Act 2: The Crisis [Days 21–40]

The scheduled supply boat never arrives. On Day 40, Christian's hydrophone/radio array intercepts a weak distress broadcast: a 40-foot Great Lakes trawler 2km upriver with a family aboard and full diesel reserves. They do not know Lab 04 exists.

- **Choice A — Broadcast the Signal:** Transmit Lab 04's coordinates and acoustic beacon. They bring diesel, permanently compromising your stealth. Unlocks tense social-survival events (paranoia over resource sharing, potential mutiny, or defending their boat when drifting hordes swarm their hull).

- **Choice B — Go Alone (The RiverWalk Run):** Maintain radio silence and execute a solo extraction to the Detroit RiverWalk marina fuel dock (600m swim).

    - _Preparation Phase:_ Christian must bleed down nitrogen via a compressed emergency deco protocol, synthesize a 600m weighted makeshift air/tether hose using salvaged ROV umbilical cabling, and program a decoy acoustic beacon to pull drifting corpses away from the marina slip.

    - _Execution:_ Navigate beneath the floating "zombie ceiling" using river currents, darkness, and tethered micro-ROV sonar before surfacing at the RiverWalk docks to siphon fuel.

### Act 3: Endings

- **Ending 1 — The Signal:** The family docks above Lab 04, inadvertently drawing a massive river-current horde or introducing compromised supplies. Final defense: hold Lab 04's bulkheads and airlock for 48 hours until an automated/military aerial sweep arrives, or scuttle the station and escape upriver on their trawler.

- **Ending 2 — The Swim:** High-tension stealth and stamina extraction. Use turbidity, underwater light discipline, and current eddies to slip beneath drifting clusters. At the RiverWalk marina, siphon diesel from unstable, listing boats where one loud splash triggers a dockside swarm. Return to Lab 04, repressurize, and survive—safe, powered, and completely alone.

# 6. AAA UNREAL ENGINE 5 C++ ARCHITECTURE & DIEGETIC UI

- **Core C++ Framework (`Source/FortyDaysUnder/`):**

    - `AUnderwaterCharacter` (inherits `ACharacter`): Custom swimming/buoyancy movement component (`UUnderwaterMovementComponent`), IK foot/hand placement for wet ladders/valves, and First-Person/Third-Person seamless camera rig.

    - `USurvivalVitalsComponent`: Tracks O2, CO2 toxicity, Nitrogen tissue load, Hydration, Calories, and Psychological Isolation strain.

    - `AROVCompanionPawn`: Tethered inspection drone with physics-simulated umbilical cable (`UCableComponent`), sonar pulse sweep, and manipulator claw.

    - `ULabSubsystemManager`: Tick-optimized C++ simulation managing power grid load, desalination output, hull integrity, and permadeath `USaveGame` cryptographic wipe on player death.

- **Psychological Isolation Meter:** Prolonged isolation triggers dynamic UE5 Material Parameter Collections (MPC) and Niagara hallucination emitters—phantom radio chatter on dead frequencies, shadows passing outside portholes, and false gauge needles. Ignoring mental strain injects procedural noise into `EnhancedInput` repair mini-games.

- **Diegetic UI (3D World-Space Widgets & Meshes):** Zero floating screen HUD. All telemetry is rendered physically on 3D Blender-modeled geometry: animated analog/digital manifold gauges on Lab 04's walls, a ruggedized dive watch skeletal mesh on Christian's left wrist, and a curved visor-projected O2/nitrogen material inside his dive mask.

# 7. PS5 DUALSENSE HARDWARE INTEGRATION (UE5 C++)

Implement via UE5 `UInputDeviceTriggerResistanceProperty`, `UInputDeviceTriggerVibrationProperty`, and raw Sony LibPad / PlatformInput hooks:

- **Adaptive Triggers (`L2` / `R2`):**

    - Dynamic resistance curves when loading/firing the pneumatic speargun or cranking manual bulkhead valves.

    - High-tension trigger stiffness and mechanical "chugging" resistance as stamina depletes or decompression sickness sets in.

- **Haptic Feedback (VCM Actuators via `.wav` / `UHapticFeedbackEffect_SoundWave`):**

    - Directional, low-frequency sub-bass pulses simulating the rhythmic thump of the surface diesel air pump (which sputters erratically as fuel drops below 15%).

    - Directional water-current drag against Christian's suit and metallic hull groans from deep water pressure.

- **Controller Speaker & Lightbar (`UInputDeviceLightColorProperty`):**

    - _Speaker:_ Muffled radio static, dive-watch decompression alarms, regulator breathing cadence, and auditory hallucinations.

    - _Lightbar:_ Interpolates smoothly from deep teal (nominal O2/pressure) to amber (low diesel / rising nitrogen) to pulsing crimson (CO2 toxicity / horde proximity).

# 8. HIGGSFIELD MCP REFERENCE GENERATION PIPELINE

**Sub-Agent 04 (Higgsfield Visual Reference Director)** must execute Asset `8.1` first, lock the output as the **Character Seed / Identity Anchor** inside Higgsfield MCP, and log the resulting URI into `PROJECT_MEMORY.md` before generating Asset `8.2` or any subsequent reference plates. All outputs serve as **orthographic, proportion, shader, and lighting reference sheets** for Sub-Agent 05's Blender 3D artists and UE5 look-dev.

### 8.1 Character Turnaround Reference Sheet: Christian Thomas

- **Pipeline:** Higgsfield Image-to-3D / Character Reference Sheet | **Aspect Ratio:** `16:9`

- **Role in 3D Pipeline:** Primary orthographic modeling blueprint for Blender character sculpting, retopology, and garment layering.

- **Prompt:** `Full-body character turnaround sheet of Christian Thomas, 28-year-old African American male, 5'9", 150 lbs, lean and athletic muscular build, rich brown skin complexion, sharp jawline, focused expression with subtle fatigue under his eyes, short crisp haircut with neat 360 waves. Three-angle orthographic view (front, 3/4 side profile, back) on a neutral dark slate studio background. Wearing weathered charcoal neoprene drysuit pants, heavy-duty modular tactical dive harness with brass carabiners, a faded navy Detroit marine-robotics engineering utility jacket with rolled sleeves, ruggedized digital dive watch on left wrist, and a compact telemetry tablet strapped to his thigh. Stylized-realistic 3D game character art, subsurface scattering on skin, wet fabric specular highlights, crisp rim lighting, 8k texture detail, Unreal Engine 5 MetaHuman fidelity.`

- **Negative Prompt:** `photorealistic stock photo, cartoon, anime, bulky armor, exaggerated proportions, braids, afro, fade without waves, smiling, cluttered background, cropped feet, inconsistent anatomy across views.`

### 8.2 Character Close-Up & Diegetic Visor Portrait (Shader & Facial Rig Reference)

- **Pipeline:** Higgsfield Face / Camera-Control Video (`Dolly In` + `Subtle Handheld`) | **Aspect Ratio:** `21:9` | **Duration:** `5s`

- **Role in 3D Pipeline:** Reference for Blender facial blendshapes/shape keys, skin pore roughness maps, hair-card wave grooming, and UE5 Subsurface Profile lighting.

- **Prompt:** `Cinematic medium close-up of Christian Thomas inside a dimly lit underwater airlock. 28-year-old African American male, brown skin, crisp 360 waves, lean build, wearing an open-collar commercial dive neck-ring and damp thermal base layer. He holds a full-face tactical dive mask with an internal cyan HUD reflection illuminating his eyes and cheekbones. Warm amber emergency bulkhead light contrasts against cold murky green river water visible through a thick circular acrylic porthole behind him. Sweat and water droplets on skin, shallow depth of field, anamorphic lens flare, slow dolly-in camera movement, stylized-realistic 3D animation, claustrophobic atmosphere.`

- **Negative Prompt:** `bright daylight, clean dry clothes, futuristic sci-fi laser helmet, flat lighting, low-poly geometry, blurry facial features.`

### 8.3 Hero Environment Reference: Lab 04 Interior & The "Living Ceiling"

- **Pipeline:** Higgsfield Environment Concept / Wide Establishing Shot | **Aspect Ratio:** `21:9` | **Camera:** `24mm Wide-Angle, Low Angle Looking Upward`

- **Role in 3D Pipeline:** Modular kitbashing blueprint for Blender interior architecture (bulkheads, acrylic dome, pipe manifolds) and UE5 Lumen volumetric lighting/caustics pass.

- **Prompt:** `Interior of Lab 04, a refurbished industrial underwater benthic research station 15 meters below the Detroit River. Rusted steel bulkheads reinforced with bolted patch plates, tangled yellow ROV umbilical cables, analog pressure manifold gauges glowing dimly, and a humming air-scrubber unit. Through a massive reinforced acrylic ceiling dome and angled observation viewports, murky emerald-green freshwater stretches upward toward the surface—silhouetting dozens of eerie, motionless floating zombie corpses drifting in the river current like a dark living ceiling blocking the overcast sky. Volumetric god-rays of pale diffused sunlight pierce between the floating silhouettes, casting caustic water ripples across the metallic lab floor. Stylized-realistic 3D game environment, atmospheric fog, Unreal Engine 5 Lumen global illumination.`

- **Negative Prompt:** `clear tropical blue ocean, coral reef, tropical fish, clean sterile sci-fi spaceship, bright neon cyberpunk colors, gore close-ups, dry land.`

### 8.4 Exterior Establishing Reference: Lab 04 & Detroit River Topography

- **Pipeline:** Higgsfield Camera Motion (`Slow Crane Down` + `Underwater Current Drift`) | **Aspect Ratio:** `16:9` | **Duration:** `6s`

- **Role in 3D Pipeline:** Reference for Blender exterior hull hard-surface modeling, tethered ROV hero prop proportions, and UE5 Niagara suspended silt particle behavior.

- **Prompt:** `Underwater wide establishing shot 15 meters beneath the Detroit River. Lab 04 sits anchored to the silty riverbed beside the rusted hull of a sunken Great Lakes freighter, tethered by thick insulated power cables rising toward the surface. Warm amber interior lights glow from Lab 04's reinforced portholes into the dark green, sediment-heavy river water. A small yellow autonomous inspection ROV with twin LED spotlights hovers near an intake valve clearing debris. In the murky upper water column near the surface, faint silhouettes of drifting corpses and a camouflaged diesel-pump buoy sway in the current. Suspended silt particles, volumetric underwater lighting, stylized-realistic 3D animation.`

- **Negative Prompt:** `tropical ocean, sharks, whales, crystal clear water, bright sunshine, cartoon textures.`

### 8.5 Surface Extraction Reference: Overrun Detroit RiverWalk & Marina

- **Pipeline:** Higgsfield Environment Concept (Act 2 — Option B) | **Aspect Ratio:** `21:9` | **Camera:** `35mm Eye-Level Waterline Shot`

- **Role in 3D Pipeline:** Reference for Blender modular dock/boardwalk assets, UE5 SingleLayerWater shader tuning, and twilight blue-hour post-process LUTs.

- **Prompt:** `Waterline perspective looking from the dark Detroit River toward the abandoned Detroit RiverWalk boardwalk and Milliken State Park marina at twilight. Overgrown concrete promenade, rusted wrought-iron railings, a derelict red-and-white fuel dock with listing pleasure boats, and the silhouette of the Renaissance Center towers shrouded in dusk fog across the skyline. Silhouettes of shambling infected wander along the boardwalk lamps while buoyant corpses cluster against the wooden marina pilings. Cold blue-hour ambient light, flickering sodium-vapor dock light reflecting off choppy dark river water, tense stealth survival atmosphere, stylized-realistic 3D animation.`

- **Negative Prompt:** `crowds of healthy people, sunny day, clean modern city, bright vibrant colors, first-person weapon HUD.`

# 9. BLENDER-TO-UNREAL ENGINE 5 AAA ASSET PIPELINE STANDARDS

**Sub-Agent 05 (3D Art & Blender-to-UE5 Pipeline Lead)** must enforce these technical specifications across all models derived from Section 8 references:

1. **Scale & Coordinate Convention:** Blender scene set to Metric (`1.0 Unit = 1.0 Meter`, exported at `100x` scale or native UE5 `1cm = 1 Unreal Unit`), `+X` forward, `+Z` up. Christian Thomas's skeletal mesh strictly calibrated to `175.26 cm` (`5'9"`).

2. **Character Modeling & Rigging (Christian Thomas):**

    - Sculpted from Higgsfield Asset `8.1` & `8.2` turnaround sheets; retopologized to a clean quad-dominant game mesh (`45k–65k tris` for LOD0).

    - Groomed 360 wave hair cap with directional anisotropic tangent maps + micro hair-card fringe for close-ups.

    - Rigged to the **UE5 Manny/Quinn Mannequin Skeleton** hierarchy (with twist bones, IK bones, and 52 ARKit-compatible facial shape keys) exported via `.fbx` (`Smoothing: Face`, `Apply Transform` enabled) for seamless UE5 Control Rig and Motion Matching compatibility.

3. **Hard-Surface & Modular Environment Modeling (Lab 04 & RiverWalk):**

    - Modeled in Blender from Higgsfield Assets `8.3`, `8.4`, and `8.5` using weighted normals, bevel modifiers, and trim-sheet UVs.

    - High-poly details baked to `4K` ORM (`Occlusion / Roughness / Metallic`), Normal (DirectX `-Y` green channel), and Albedo maps.

    - Static architectural meshes exported for **UE5 Nanite** (no manual LODs required on opaque solid hull plates); glass/acrylic portholes and transparent water surfaces separated onto non-Nanite translucent Substrate shaders with custom refractive caustics.

    - Custom convex collision hulls prefixed with `UCX_<MeshName>` authored directly in Blender for tight interior walkways and airlock hatches.

# 10. ORCHESTRATION EXECUTION WORKFLOW

As the **Orchestrator**, execute the following delegation pipeline step-by-step:

1. **Memory Ingestion & State Check:** Read `PROJECT_MEMORY.md` (or initialize it if absent) so you resume directly from the last recorded sub-agent milestone without duplicating prior work.

2. **Task Graph & Sub-Agent Routing Initialization:** Output a Directed Acyclic Graph (DAG) showing how you partition remaining tasks, pass context contracts, and synchronize dependencies across **Sub-Agents 01 through 05**.

3. **Sub-Agent 04 Dispatch (Higgsfield MCP Execution):** Invoke the Higgsfield MCP server to generate any uncompleted Reference Assets (`8.1` through `8.5`), enforcing the Character Seed lock from `8.1` and recording the URIs in `PROJECT_MEMORY.md`.

4. **Parallel Sub-Agent Deliverables:** Collect and synthesize modular, production-ready outputs from each sub-agent:

    - **[From Sub-Agent 02] UE5 C++ Core Architecture:** Header (`.h`) and implementation (`.cpp`) scaffolding for `USurvivalVitalsComponent` (Air + Bühlmann Decompression math) and `AUnderwaterCharacter`.

    - **[From Sub-Agent 05] Blender-to-UE5 Asset Production Sheet:** Exact polycount budgets, UV/texel density targets (`10.24 px/cm` hero, `5.12 px/cm` environment), skeletal bone hierarchy, and `.fbx` export checklist mapped directly to Assets `8.1–8.5`.

    - **[From Sub-Agent 03] PS5 DualSense UE5 Input Integration:** C++ `UInputDeviceProperty` trigger resistance curves and haptic event mappings.

    - **[From Sub-Agent 01 & 02] Act 2 Choice A/B State-Machine & Opening 5-Minute Vertical Slice:** Complete playable logic, resource formulas, and dialogue script inside Lab 04.

5. **Memory Commit / Handoff Output:** Persist the updated `PROJECT_MEMORY.md` to disk via MCP/filesystem tools—and always render the updated `PROJECT_MEMORY.md` block at the end of your response for seamless cross-model continuity.

```

<ElicitationsGroup message="To generate your starter repository files or specific sub-agent modules right now:">
  <Elicitation label="Generate the initial PROJECT_MEMORY.md and AGENTS.md root files" query="Generate the ready-to-save initial PROJECT_MEMORY.md and root AGENTS.md / CLAUDE.md pointer files for my 40 Days Under Unreal Engine 5 repository."/>
  <Elicitation label="Write the UE5 C++ header and source files for USurvivalVitalsComponent" query="Write the complete Unreal Engine 5 C++ header (.h) and implementation (.cpp) files for USurvivalVitalsComponent, including the real-time Diesel Air Clock and Bühlmann nitrogen decompression math for 40 Days Under."/>
  <Elicitation label="Create the Blender-to-UE5 character and environment technical art bible" query="Create the detailed Blender-to-UE5 technical art bible for 40 Days Under, including polycount budgets, texel density, UE5 Mannequin bone naming, and FBX export settings based on the Higgsfield reference assets."/>
</ElicitationsGroup>
```