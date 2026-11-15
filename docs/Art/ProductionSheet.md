# 40 Days Under: Blender-to-UE5 Art Production Sheet

Owner: Sub-Agent 05 (3D Art and Blender-to-UE5 Pipeline Lead). Engine: Unreal Engine 5.8. DCC: Blender 4.x.
Authority: `scope.md` sections 2, 8, 9 and section 10 step 4. Companion: `Docs/Art/FbxExportChecklist.md`.

Labels used in this sheet:
- **[SCOPE]** value fixed by `scope.md`. Do not change without orchestrator sign-off.
- **[PROPOSED]** professional budget or choice made by this sheet. Adjustable by the art lead.
- **[VERIFY]** could not be confirmed from source. Check against `SK_Mannequin` in engine (or the stated test) before relying on it.

---

## 1. Global standards (apply to every asset)

| Topic | Standard |
| :--- | :--- |
| Units | Blender scene Unit System = Metric, Unit Scale = 1.0, so 1 Blender unit = 1 m. UE works in cm (1 UU = 1 cm); the FBX exporter converts. [SCOPE] |
| Axes | +X forward, +Z up. Author every asset facing +X. [SCOPE] (see 1.1 for the export-axis caveat) |
| Christian height | Exactly 175.26 cm (5'9"), measured as soles-on-ground to the top of the head mesh in the Blender rest pose (1.7526 m). [SCOPE] |
| Texel density, hero | 10.24 px/cm (4096 px spans 400 cm). [SCOPE for the value] Which assets belong to the hero set (Christian, ROV, hatch, gauge panel, scrubber, umbilical coil, lamp post, fuel pump, canopy, and any close-inspection prop) is [PROPOSED]; scope.md says only "10.24 px/cm hero, 5.12 px/cm environment". The Density column in every module table below is therefore [PROPOSED] except where it simply restates the 5.12 environment default. |
| Texel density, environment | 5.12 px/cm (4096 px spans 800 cm). [SCOPE for the value] Lab 04 and RiverWalk kit pieces by default; membership is [PROPOSED]. |
| Texture size | 4K (4096 x 4096) texture sets. [SCOPE] |
| Packed ORM | `T_<Name>_ORM`: R = ambient occlusion, G = roughness, B = metallic. Imported with sRGB off, compression "Masks (no sRGB)". [SCOPE] |
| Normal map | DirectX convention (-Y green). Imported with compression "Normalmap", sRGB off. Do not tick "Flip Green Channel" on import if the bake already used DirectX. [SCOPE] |
| Albedo | `T_<Name>_BC`: sRGB on, no baked lighting. [SCOPE] |
| Tangent space and normals | Bake with MikkTSpace. Hard-surface assets keep their Blender custom/weighted normals through import: UE Normal Import Method must be "Import Normals" (or "Import Normals and Tangents"), with tangents computed as MikkTSpace. [PROPOSED] |
| Substrate prerequisite | Translucent glass, acrylic and water materials need Substrate enabled in the project (`r.Substrate=1`, requires an editor restart) before they can be built. Not currently set in `unreal/Config`; the orchestrator or engine owner enables it. This sheet does not edit Config. [VERIFY exact setting in 5.8 Project Settings > Rendering] |
| Hard-surface method | Weighted normals plus bevel modifiers, trim-sheet UVs. Details baked from high poly to low poly. [SCOPE] |
| Collision | Hand-authored `UCX_<MeshName>` convex hulls in Blender. [SCOPE] |

Texel-density arithmetic (so the numbers are checkable): 4096 px / 10.24 px/cm = 400 cm; 4096 px / 5.12 px/cm = 800 cm. In Blender's Texel Density tooling set the target to 10.24 px/cm (hero) or 5.12 px/cm (environment) with the image size set to 4096.

### 1.1 Axis caveat [VERIFY]

`scope.md` says +X forward, +Z up, which matches UE. Blender's own default "forward" in the FBX exporter dialog differs, and the correct Forward/Up pair for a mesh authored facing +X must be proven, not assumed. Before the first real export, run the pipeline smoke test in `FbxExportChecklist.md` section 4: export a 1 m cube with a nose pointing along Blender +X, import it into UE, and confirm the nose points along UE +X with no rotation on the imported actor. Record the working Forward/Up pair in that checklist, then reuse it for every asset.

---

## 2. Naming and folder conventions

### 2.1 Prefixes

| Type | Prefix | Example |
| :--- | :--- | :--- |
| Skeletal mesh | `SK_` | `SK_Christian` |
| Skeleton asset | `SKEL_` (engine auto-name is `<Mesh>_Skeleton`; rename on import) | `SKEL_Christian` |
| Physics asset | `PHYS_` | `PHYS_Christian` |
| Static mesh | `SM_` | `SM_Lab04_Bulkhead_Straight_A` |
| Collision hull (in FBX only) | `UCX_<MeshName>` or `UCX_<MeshName>_NN` | `UCX_SM_Lab04_Bulkhead_Straight_A_01` |
| Material | `M_` | `M_Christian_Skin` |
| Substrate translucent material | `M_<Name>_Glass_Sub` | `M_Lab04_Dome_Acrylic_Sub` |
| Material instance | `MI_` | `MI_Lab04_RustSteel_A` |
| Texture | `T_<Name>_BC`, `_N`, `_ORM`, `_H` (height, optional), `_MSK` | `T_Lab04_Trim_A_ORM` |
| Groom / hair-card assets | `SM_`/`T_` with `Hair` token | `T_Christian_HairCards_Tangent` |

Asset IDs from `PROJECT_MEMORY.md` section 3 (`CHR_Christian_01`, `ENV_Lab04_Int`, `PROP_ROV_Scout`, `ENV_RiverWalk_Dock`) name the Blender files and the UE folders. Mesh and material names inside use the prefixes above.

### 2.2 Blender source layout (proposed; nothing is created in this task)

```
art/
  CHR_Christian_01/
    CHR_Christian_01_highpoly.blend     sculpt, 8.1 / 8.2 blockout
    CHR_Christian_01_lowpoly.blend      retopo, UV, rig, shape keys, LODs
    CHR_Christian_01_bake.blend         bake setup (optional)
    textures/                           source and painted maps
    export/CHR_Christian_01.fbx         single FBX handed to UE
  ENV_Lab04_Int/
    ENV_Lab04_Int_kit.blend             all kit pieces in one file, one collection per piece
    ENV_Lab04_Int_trim.blend            trim-sheet master
    export/                             one FBX per kit piece (SM_<name>.fbx)
  PROP_ROV_Scout/
    PROP_ROV_Scout.blend
    export/PROP_ROV_Scout.fbx
  ENV_RiverWalk_Dock/
    ENV_RiverWalk_Dock_kit.blend
    export/                             one FBX per module
  _shared/
    trims/  matcaps/  scale_references/ (Manny and 175.26 cm reference)
```

Rules:
- `.blend` files are named `<AssetID>[_part].blend`. Versioning is done in git; never use `_final`, `_v2`.
- FBX files are written only to the asset's `export/` folder and are named exactly like the UE asset they produce (`SM_Lab04_Wall_Panel_Straight.fbx`). The one exception is the character, whose single FBX is `CHR_Christian_01.fbx` and imports as `SK_Christian`.
- Inside Blender, every exportable object uses the final UE name (`SM_Lab04_Bulkhead_Straight_A`) so the FBX import produces correct asset names without renaming.
- Whether `.blend`/`.fbx` binaries go to Git LFS is a repo decision for the orchestrator, not covered here.

### 2.3 UE content destinations (proposed)

```
unreal/Content/
  Characters/Christian/
    Meshes/       SK_Christian, SKEL_Christian, PHYS_Christian
    Materials/    M_Christian_Skin, M_Christian_Cloth, M_Christian_Hair, MI_*
    Textures/     T_Christian_*
    Hair/         hair-card meshes and tangent maps
    Rig/          Control Rig and IK Rig assets (retargeted from Manny/Quinn)
  Environments/Lab04/
    Meshes/       SM_Lab04_*
    Materials/    M_Lab04_*, MI_*
    Textures/     T_Lab04_*
  Environments/RiverWalk/
    Meshes/       SM_RW_*
    Materials/    M_RW_*, water Substrate materials
    Textures/     T_RW_*
  Props/ROV_Scout/
    Meshes/  Materials/  Textures/
```

`unreal/Content/` is shared with other agents' folders; stay inside the folders above.

---

## 3. Asset: `CHR_Christian_01` (references 8.1 and 8.2)

**Higgsfield references (`PROJECT_MEMORY.md` section 1, `[ACTIVE]` entries):**
- 8.1 turnaround sheet: image job `58c39f98-e7f8-4a0e-bb18-7a3c97104f78`; locked character reference element `4284877d-9158-4176-98aa-f7344abff977` ("Christian Thomas - Lab 04"); local `assets/hf_20261114_120221_58c39f98-e7f8-4a0e-bb18-7a3c97104f78.png`.
- 8.2 visor portrait video: job `673f6cc5-ad50-45f9-9975-404221147ffd`; local `assets/hf_20261114_120414_673f6cc5-ad50-45f9-9975-404221147ffd_min.mp4`.

**Details taken from the 8.1 image (viewed directly):**
- Layout: front, 3/4 and back views with matching proportions, a height gauge reading 5'9" (175 cm), wireframe silhouettes at the bottom (front, 3/4, back) usable as orthographic blockout overlays.
- Pose: relaxed upright stance, arms hanging slightly away from the body (a mild A-pose), feet roughly shoulder width. Use this rest pose as the bind pose; it is close to the Manny A-pose.
- Face: close-up panel shows a short goatee/chin beard and sharp tapered fade with neat 360 waves. The beard is not in the written prompt but is in the locked image, so it is part of the model (decision for the art lead: keep it, since the image is the locked identity anchor).
- Outfit layers, each a separate material slot candidate: dark navy work jacket with sleeves rolled to the forearm, zipped open over a grey tee, stand collar, a "DETROIT MARINE ROBOTICS ENGINEERING" chest patch with skyline logo on the left chest; modular dive harness over the jacket (chest strap with brass buckle, waist belt with brass buckle, two thigh/leg straps, brass carabiners at the waist, a large brass D-ring on the upper back, a rear plate carrier panel); belt pouches at the lower back and hip; weathered black neoprene drysuit pants with knee pads and a visible side zip; lace-up lugged boots with brass tabs.
- Hard-surface accessories: ruggedized dive watch on the left wrist (screen shows "DIVE 18.4 m 28:17" in the reference, which is an emissive screen for DEC-002 diegetic UI); telemetry tablet mounted on the left thigh (viewer's right in the front view) with a cyan screen.
- Palette strip on the sheet: charcoal, navy, slate, brass/gold, blue accent. Use it as the albedo palette.
- Surface finish: heavy wet look on the pants and jacket (high specular, water droplets); skin with sweat droplets.

**Details taken from 8.2 (not viewable as a still here; derived from the scope.md prompt text only):** commercial dive neck-ring, damp thermal base layer, full-face dive mask with internal cyan HUD reflection, sweat and droplets. Treat as the facial-expression, skin-roughness and hair-wave grooming reference; the neck-ring and full-face mask are separate wearable meshes (see 3.7). [VERIFY: scrub the mp4 in a video player for the facial deformation range before authoring FACS shapes.]

### 3.1 Scale and orientation
- Height 175.26 cm (1.7526 m) exactly, enforced with a scale-check empty in the `.blend` at Z = 1.7526 m. [SCOPE]
- Rest pose A-pose, facing +X, feet on Z = 0, origin at world origin between the feet. Skeleton `root` bone at the origin. The Manny skeleton also has its origin on the ground at the feet; the Manny bind pose is an A-pose.
- Apply scale and rotation (Ctrl+A) on every object before export; scale 1.0 on mesh and armature.

### 3.2 Polycount budgets

| Mesh | Budget (tris) | Status |
| :--- | :--- | :--- |
| LOD0 (body, head, clothing, harness, boots, hair cap) | **45,000 to 65,000** | [SCOPE] |
| LOD1 | 25,000 to 30,000 (about 50% of LOD0) | [PROPOSED] |
| LOD2 | 10,000 to 14,000 | [PROPOSED] |
| LOD3 | 3,500 to 5,000 | [PROPOSED] |
| Hair-card fringe and edge cards (separate mesh/section, LOD0) | 6,000 to 10,000 | [PROPOSED] |
| Hair-card fringe, LOD1 / LOD2 / LOD3 | 4,000 / 1,500 / cap only, cards dropped | [PROPOSED] |
| Dive watch (if built as a separate skeletal mesh per DEC-002) | 3,000 to 6,000 | [PROPOSED] |
| Full-face dive mask (separate wearable) | 8,000 to 12,000 | [PROPOSED] |

Notes:
- The 45k to 65k LOD0 range is the scope value. Whether hair cards, mask and watch count inside it is not stated in scope.md; this sheet counts them separately, so the "body" mesh must itself reach 45k to 65k. Raise with the orchestrator if a single combined figure is required.
- Skeletal meshes use manual LODs LOD0 to LOD3 (either each LOD exported as its own FBX, because Blender writes no FBX LOD groups, and imported with the UE Skeletal Mesh "Import LOD" workflow, or generated in UE with its reduction tools and then hand-checked at the face and hands). Nanite on skeletal meshes is not assumed; leave it off unless verified for the 5.8 skeletal pipeline. [VERIFY]
- Quad-dominant topology; face loops follow the eye, mouth and nasolabial flow for the 52 shape keys; hands and elbows get three edge loops for deformation.

### 3.3 Texturing (hero density)
All at 10.24 px/cm, 4K, ORM + DirectX normal + albedo:

| Texture set | Covers | Notes |
| :--- | :--- | :--- |
| `T_Christian_Head` | face, ears, neck, scalp | skin pore roughness from 8.2; Subsurface Profile skin material; goatee as albedo plus fine cards or mesh |
| `T_Christian_Body` | hands, forearms, any exposed skin | shares the skin material with the head |
| `T_Christian_Cloth` | jacket, tee, drysuit pants, boots | wet-look roughness, metal brass pieces marked metallic in B |
| `T_Christian_Gear` | harness, belt pouches, carabiners, D-ring, watch, tablet | tablet screen and watch face use emissive masks (`_MSK`) |
| `T_Christian_Hair` | hair cap, fringe cards | see 3.5 |

Extra skin maps (as needed): cavity, thickness (for subsurface), and a small wrinkle-map set for shape-key-driven wrinkles. [PROPOSED]

### 3.4 Skeleton and rig

Target: the UE5 Manny/Quinn Mannequin skeleton hierarchy, so Control Rig, IK Rig retargeting and Motion Matching work. [SCOPE]

**Confirmation method.** UE 5.8 is installed; `SK_Mannequin.uasset` and `IK_Mannequin.uasset` under `/Users/Shared/Epic Games/UE_5.8/Engine/Plugins/Experimental/MoverExamples/Content/Characters/Mannequins/` were read (read-only) and their name tables extracted with `strings`. **Bone names below are confirmed present in those name tables.** The name table does not encode parent/child order, so **parenting is listed from the standard Manny layout and is [VERIFY]** unless noted. The authoritative check is to open `SK_Mannequin` in the UE 5.8 Skeleton editor and compare, or run the retarget-chain compare in the IK Rig editor.

**Deform bones, main chain (names confirmed in `SK_Mannequin` and `IK_Mannequin`):**

```
root
  pelvis
    spine_01
      spine_02
        spine_03
          spine_04
            spine_05
              neck_01
                neck_02
                  head
              clavicle_l
                upperarm_l
                  lowerarm_l
                    hand_l
                      (fingers, see below)
                  upperarm_twist_01_l, upperarm_twist_02_l     (twist, children of upperarm_l)
                  lowerarm_twist_01_l, lowerarm_twist_02_l     (twist, children of lowerarm_l)
              clavicle_r   (mirror of clavicle_l, all _r)
    thigh_l
      calf_l
        foot_l
          ball_l
        calf_twist_01_l, calf_twist_02_l        (twist, children of calf_l)
      thigh_twist_01_l, thigh_twist_02_l        (twist, children of thigh_l)
    thigh_r   (mirror of thigh_l, all _r)
```

Fingers, each side (`_l` and `_r`), confirmed names:
`thumb_01`, `thumb_02`, `thumb_03`; `index_metacarpal`, `index_01`, `index_02`, `index_03`; `middle_metacarpal`, `middle_01`, `middle_02`, `middle_03`; `ring_metacarpal`, `ring_01`, `ring_02`, `ring_03`; `pinky_metacarpal`, `pinky_01`, `pinky_02`, `pinky_03`. Thumb has no metacarpal bone in the confirmed list. Each chain parents to `hand_<side>`, `_01` under its metacarpal where one exists.

**Twist bones (confirmed names):** `upperarm_twist_01_l/r`, `upperarm_twist_02_l/r`, `lowerarm_twist_01_l/r`, `lowerarm_twist_02_l/r`, `thigh_twist_01_l/r`, `thigh_twist_02_l/r`, `calf_twist_01_l/r`, `calf_twist_02_l/r`. All 16 appear in `IK_Mannequin`. Weight: `_01` at about one third and `_02` at about two thirds along the limb segment. [VERIFY exact placement ratio in engine.]

**IK, utility and marker bones (confirmed names; non-deforming, must be exported):**

| Bone | Role | Parent |
| :--- | :--- | :--- |
| `ik_foot_root` | root of foot IK | `root` [VERIFY] |
| `ik_foot_l`, `ik_foot_r` | foot IK targets | `ik_foot_root` [VERIFY] |
| `ik_hand_root` | root of hand IK | `root` [VERIFY] |
| `ik_hand_gun` | weapon/held-object IK driver | `ik_hand_root` [VERIFY] |
| `ik_hand_l`, `ik_hand_r` | hand IK targets | `ik_hand_gun` [VERIFY] |
| `interaction` | interaction/attach marker | `root` [VERIFY] |
| `center_of_mass` | center of mass marker | `root` [VERIFY] |

Additional names present in the `SK_Mannequin` name table that this sheet does **not** require for the first pass: `ik_foot_l_ball`, `ik_foot_r_ball`, `ik_hand_l_pivot`, `ik_hand_r_pivot`, `ik_hand_l_wrld`, `ik_hand_r_wrld`, `foot_prediction_l/r`, `l_foot_plant`, `r_foot_plant`, `weapon_l`, `weapon_r`, `weapon_r_muzzle`, `base_pose`, and a large set of corrective/driver bones (`upperarm_*`, `thigh_*`, `clavicle_*`, `lowerarm_*`, `calf_*`, `wrist_*`, `ankle_*`, `spine_04_latissimus_*`, etc.). **Whether these exist as real bones or are only pose-asset/curve names in 5.8 cannot be told from a name table.** Treat them as optional; for the first pass add only the bones in the tables above, and add `weapon_r` as the speargun attach socket candidate. [VERIFY against `SK_Mannequin` in engine]

**Bones this sheet cannot confirm and therefore does not specify:** any facial/eye/jaw/tongue bones (the Manny body skeleton has none in the name table; facial animation here is by shape keys, section 3.6), and any custom bones for Christian's gear (watch, tablet, harness, pouches). For gear secondary motion, add extra bones **below** the mannequin bones with a `gear_` prefix (for example `gear_pouch_back_l`), so they never collide with Manny names. [PROPOSED]

**Marked "verify against `SK_Mannequin` in engine":**
1. Parent of every IK and utility bone in the table above (8 rows).
2. The exact transform and orientation of every bone (roll and primary axis). Match Manny's bone axes; do not rely on Blender defaults.
3. Twist bone placement ratio.
4. Whether the 5.8 Manny keeps `spine_05` and the per-limb corrective bones as real skeletal bones (names were found; hierarchy is unverified).
5. Which Quinn-specific differences exist, if any (Quinn and Manny share this skeleton in recent versions; only Manny was inspected).

**Rig construction rules [PROPOSED]:**
- Build the Blender armature by copying Manny's bone names, hierarchy and roll from a one-time Manny FBX export (UE: right-click `SK_Mannequin` > Asset Actions > Export FBX; do not edit the source asset), then fit to Christian's proportions. Do not rename bones.
- **Orientation rule (one rule):** the Manny FBX is a reference only and, when imported into Blender, will not face +X. Import it, then rotate the whole imported armature and mesh (not individual bones) about Z so it faces +X, and apply rotation and scale, before copying bones. Everything authored afterwards faces +X, exactly as section 3.1 states. The FBX export Forward/Up pair (checklist section 2) is what maps this +X authoring back onto the UE import, so the Manny-derived bone axes must come out identical to Manny's in the smoke test (checklist section 4); if they do not, change the export axis pair, never the authored facing.
- Skin weights: 4 influences per vertex maximum on LOD0 in the exported FBX unless the project later enables 8-influence (verify project setting). [PROPOSED]
- Export every bone, including non-deforming IK bones (so "Only Deform Bones" is **off**; see the checklist).

### 3.5 Hair

Per scope.md: **groomed 360 wave hair cap with directional anisotropic tangent maps, plus a micro hair-card fringe for close-ups.** [SCOPE]
- **Hair cap:** a skin-hugging scalp mesh (part of LOD0) carrying the 360 wave pattern as a normal and height map, with the wave flow direction painted into a tangent (flow) map. The material uses an anisotropic specular response driven by that tangent map. The waves are short, tight, concentric ripples that radiate from the crown and sweep forward and down toward the temples; the back of the head in the 8.1 back view shows the circular wave rings clearly. Use the 8.1 back and 3/4 head panels as the direct reference.
- **Hair-card fringe:** a small set of alpha-clipped cards (or short ribbons) along the hairline at the forehead and temples, with sharp tapered fade into the skin, for close-ups. Hairline is crisp, which matches the 8.1 front and 3/4 views.
- Texture set `T_Christian_Hair`: albedo plus alpha, `_ORM` (cards), `_Tangent` (the flow/tangent map), and a per-strand ID/root-tip mask. Tangent map stored as RG in tangent space; compression "Masks"/"Default (no sRGB)". [PROPOSED]
- Hair cards are a **separate mesh section with its own material slot** so they can be dropped from LOD1 and below.

### 3.6 Facial shape keys: all 52 ARKit names

Created as Blender shape keys with exactly these names (case-sensitive), on the head mesh, and exported with the FBX ("Include morph targets" is by default included when shape keys exist; verify in the checklist). The Basis key is named `Basis`.

Eyes (14): `eyeBlinkLeft`, `eyeLookDownLeft`, `eyeLookInLeft`, `eyeLookOutLeft`, `eyeLookUpLeft`, `eyeSquintLeft`, `eyeWideLeft`, `eyeBlinkRight`, `eyeLookDownRight`, `eyeLookInRight`, `eyeLookOutRight`, `eyeLookUpRight`, `eyeSquintRight`, `eyeWideRight`

Jaw (4): `jawForward`, `jawLeft`, `jawRight`, `jawOpen`

Mouth (23): `mouthClose`, `mouthFunnel`, `mouthPucker`, `mouthLeft`, `mouthRight`, `mouthSmileLeft`, `mouthSmileRight`, `mouthFrownLeft`, `mouthFrownRight`, `mouthDimpleLeft`, `mouthDimpleRight`, `mouthStretchLeft`, `mouthStretchRight`, `mouthRollLower`, `mouthRollUpper`, `mouthShrugLower`, `mouthShrugUpper`, `mouthPressLeft`, `mouthPressRight`, `mouthLowerDownLeft`, `mouthLowerDownRight`, `mouthUpperUpLeft`, `mouthUpperUpRight`

Brows (5): `browDownLeft`, `browDownRight`, `browInnerUp`, `browOuterUpLeft`, `browOuterUpRight`

Cheeks (3): `cheekPuff`, `cheekSquintLeft`, `cheekSquintRight`

Nose (2): `noseSneerLeft`, `noseSneerRight`

Tongue (1): `tongueOut`

Total: 14 + 4 + 23 + 5 + 3 + 2 + 1 = 52.

Notes:
- "Left" and "Right" follow ARKit's convention (the subject's own left and right). Eye-look shapes need real eyeballs; model eyes as separate meshes, skinned to the head bone or to shape keys per the art lead (no eye bones are confirmed in the Manny name table). [VERIFY]
- Shape keys on the head and any facial-hair/teeth/tongue/eyelash meshes must keep identical key names on every mesh that deforms, so UE morph-target curves drive all meshes. [PROPOSED]
- Only the head mesh is split out of the body mesh for LOD0; share one skeleton.

### 3.7 Wearables and diegetic items
- Dive watch on the left wrist: screen on its own material slot (emissive, driven by a Material Parameter Collection or dynamic material instance per DEC-002). Reference shows an analog-digital face with depth and time readouts.
- Telemetry tablet on the left thigh: separate material slot for the screen; reference shows a cyan terrain-style graph.
- Neck-ring and full-face mask (from 8.2): separate meshes parented via sockets (`head` bone). Inside-of-visor curved surface UVs reserved for the O2/nitrogen projection material (DEC-002).

### 3.8 Collision and physics
- Skeletal mesh uses a Physics Asset (`PHYS_Christian`) built in UE, not `UCX_` hulls.
- Capsule for gameplay comes from the C++ character class, not the art asset.

### 3.9 UE destinations
`unreal/Content/Characters/Christian/` per section 2.3. Import settings in `FbxExportChecklist.md` section 3.

---

## 4. Asset: `ENV_Lab04_Int` (reference 8.3)

**Higgsfield reference:** 8.3 interior concept, job `a583d3d9-57d8-4eba-9bab-cd31e6346ac9` (2688x1152), local `assets/hf_20261114_121052_a583d3d9-57d8-4eba-9bab-cd31e6346ac9.png`. Alternate lower-resolution job `59154831-16da-4298-a683-7efa6f4ab992` (1344x576), not active.

**Details taken from the 8.3 image (viewed directly):**
- A circular hab room with a massive riveted steel ring frame holding a huge acrylic ceiling dome; the frame has radial buttress ribs and a flat-sided hexagonal/trapezoidal observation viewport on each side of the dome base, held by thick steel mullions.
- Left wall: large riveted steel panel with stencilled "LAB 04 / DETROIT RIVER BENTHIC RESEARCH STATION / DEPTH: 15 m", bolted rectangular patch plates, rust streaks, heavy bundled yellow cables running down from the ceiling, and a caged bulkhead lamp.
- Pressure manifold panel with four round analog gauges labelled `HAB PRESS`, `AIR SUPPLY`, `SCRUBBER dP`, `EXT PRESS`, each with a valve wheel and a pipe run below it. These are the diegetic gauges for DEC-002.
- Air-scrubber unit: a boxy housing labelled "AIR-SCRUBBER" with louvered intake grilles and a large pipe elbow rising to the ceiling.
- Rear wall: round bulkhead hatch with a central wheel and a small porthole, flanked by a caged lamp; a yellow safety railing in front of it.
- Floor: dark wet concrete/steel deck with a floor drain grate at the lower right, large coiled yellow ROV umbilical lying on the floor at the lower left, caustic light patterns.
- Right side: lab bench with microscope, specimen jars, a monitor, a task lamp and a rolling office chair; shelving with binders; stacked hard-shell equipment cases.

### 4.1 Kit module list (Nanite opaque hull pieces unless noted)

| Module | Description | Tris budget [PROPOSED] | Density [PROPOSED] | Notes |
| :--- | :--- | :--- | :--- | :--- |
| `SM_Lab04_Floor_Deck_4x4` | 4 m x 4 m deck plate | 2,000 | 5.12 | trim sheet plus tiling; floor drain variant `_Drain` |
| `SM_Lab04_Wall_Panel_Straight` | flat riveted wall segment, 4 m | 4,000 to 8,000 | 5.12 | patch-plate and rivet variants via trim |
| `SM_Lab04_Wall_Panel_Curved` | curved segment for the circular hab, fixed arc | 6,000 to 10,000 | 5.12 | angle chosen so a full ring closes |
| `SM_Lab04_Bulkhead_Hatch_Door` | round hatch with wheel, frame, porthole mount | 12,000 to 20,000 | 10.24 | interaction prop, close inspection |
| `SM_Lab04_Hatch_Porthole_Glass` | porthole glass | 300 | 5.12 | **non-Nanite**, Substrate translucent |
| `SM_Lab04_Dome_Ring_Frame` | riveted ring and ribs for the dome | 30,000 to 50,000 | 5.12 | Nanite, split into radial segments (for example 12) |
| `SM_Lab04_Dome_Acrylic` | acrylic dome surface | 1,500 to 3,000 | 5.12 | **non-Nanite**, Substrate translucent, refractive caustics |
| `SM_Lab04_Viewport_Frame` | angled viewport frame | 6,000 to 10,000 | 5.12 | Nanite |
| `SM_Lab04_Viewport_Glass` | viewport pane | 200 | 5.12 | **non-Nanite**, Substrate translucent |
| `SM_Lab04_Gauge_Panel` | manifold panel body, valves, pipe run | 15,000 to 25,000 | 10.24 | hero, gauge faces on separate materials |
| `SM_Lab04_Gauge_Dial` | single analog gauge (x4 instances) | 1,500 to 3,000 | 10.24 | glass lens separate, non-Nanite; needle separate for animation |
| `SM_Lab04_AirScrubber_Unit` | scrubber housing with grille, pipe elbow | 15,000 to 25,000 | 10.24 | hero |
| `SM_Lab04_Pipe_Straight` / `_Elbow` / `_Flange` | pipe kit | 500 to 3,000 each | 5.12 | many instances; Nanite |
| `SM_Lab04_Railing_Yellow` | safety railing 2 m | 1,500 | 5.12 | with `UCX_` hulls |
| `SM_Lab04_Cable_Bundle_Wall` | wall-run yellow cable bundles | 2,000 to 5,000 | 5.12 | spline variants |
| `SM_Lab04_Umbilical_Coil_Floor` | coiled umbilical | 6,000 to 10,000 | 10.24 | hero floor prop |
| `SM_Lab04_Lamp_Caged` | caged bulkhead lamp | 2,000 | 5.12 | emissive bulb slot |
| `SM_Lab04_Sign_Plate` | stencilled wall sign | 800 | 5.12 | decal-style, own texture |
| `SM_Lab04_Bench_Lab`, `_Chair_Rolling`, `_Shelf_Binders`, `_Case_Hardshell`, `_Microscope`, `_Jar_Specimen`, `_Monitor` | set dressing | 1,000 to 8,000 each | 5.12 (microscope 10.24) | set dressing; Nanite for opaque parts |
| `SM_Lab04_Airlock_Door_Frame` | airlock hatch pieces for deco chamber | 12,000 to 20,000 | 10.24 | the airlock exists in the loop (17-hour deco) but was not visible in 8.3; shape to be designed [VERIFY against concept] |

Budget philosophy: Nanite lets the hull stay dense, so budgets are about authoring cost and instance count, not LODs. Tri counts above are [PROPOSED].

### 4.2 Texturing
- A shared **trim sheet** (steel panel, rivet strips, bolt rows, rust-edge, pipe wrap, caged-lamp metal, yellow cable) at 4K and 5.12 px/cm; hero pieces (hatch, gauge panel, scrubber) get unique 4K sets at 10.24 px/cm.
- ORM packed R occlusion, G roughness, B metallic; DirectX normals; albedo. [SCOPE]
- Painted rust and wet-floor variation handled by vertex color or a tiling mask in the master material rather than unique bakes. [PROPOSED]

### 4.3 Nanite and translucency
- Opaque hull plates, ribs, frames, pipes: **Nanite enabled, no manual LODs.** [SCOPE]
- Acrylic dome, viewport panes, porthole and gauge lenses: **Nanite off**, separate mesh/material, translucent **Substrate** material with custom refractive caustics. [SCOPE]
- Keep the glass as separate objects (own FBX or own mesh element in the module), never merged with the Nanite hull.

### 4.4 Collision
- Each module that the player can touch has `UCX_<MeshName>` convex hulls authored in Blender (tight walkway, hatch frames, railings, bench, scrubber). Example: `UCX_SM_Lab04_Railing_Yellow_01`. [SCOPE]
- Hull rules: convex only, under 32 vertices each when possible, no hulls on dome glass (use a simple invisible blocker authored in UE if needed), hatch opening must stay clear for the player capsule. Walkway clearance: player-sized capsule can pass the hatch. Sizes follow the player's 175.26 cm height plus clearance. [PROPOSED]

### 4.5 UE destination
`unreal/Content/Environments/Lab04/` per section 2.3.

---

## 5. Asset: `PROP_ROV_Scout` (reference 8.4)

**Higgsfield reference:** 8.4 exterior establishing video, job `e831062e-80f2-4af1-af80-0f8ea43f6e7a`, seed `1925040229`, local `assets/hf_20261114_121202_e831062e-80f2-4af1-af80-0f8ea43f6e7a_min.mp4`.

**Details taken from 8.4:** this is a video, and no tool to extract frames (no `ffmpeg`) was available, so it was **not viewed**. The ROV description is taken from the scope.md prompt text and section 6: a small yellow autonomous inspection ROV with twin LED spotlights, hovering at an intake valve clearing debris; tethered by an umbilical; sonar pulse sweep and manipulator claw per `AROVCompanionPawn`. Proportions, thruster layout and frame shape are therefore **unverified against the video**. [VERIFY: review the mp4 and update this section; the module list below is a proposal.]

### 5.1 Module list and budgets [PROPOSED]

| Module | Description | Tris budget | Density [PROPOSED] |
| :--- | :--- | :--- | :--- |
| `SM_ROV_Scout_Body` | main yellow frame and shell | 15,000 to 25,000 | 10.24 |
| `SM_ROV_Scout_Thruster` (x instances) | thruster with shroud; propeller separate for rotation | 2,000 to 4,000 each | 10.24 |
| `SM_ROV_Scout_Light` (x2) | twin LED spotlight housing | 1,500 to 3,000 each | 10.24 |
| `SM_ROV_Scout_Light_Lens` | lens | 200 to 400 | 10.24 |
| `SM_ROV_Scout_Claw` | manipulator claw (jaw and arm separate objects for animation) | 6,000 to 10,000 | 10.24 |
| `SM_ROV_Scout_Sonar` | sonar dome/head | 1,500 to 3,000 | 10.24 |
| `SM_ROV_Scout_Tether_Attach` | tether connector | 1,000 to 2,000 | 10.24 |
| `SM_ROV_Scout_Umbilical_Segment` | umbilical cable piece | 500 to 1,500 | 5.12 | 
| **Whole ROV, total** | assembled | 40,000 to 60,000 | |

The umbilical itself is driven by `UCableComponent` in C++ (scope.md section 6); the model provides only the attachment point and a short cable stub. Keep moving parts (thruster props, claw jaws, sonar head) as separate meshes with pivots at their rotation centers, authored as separate objects for socket or bone attach. If the ROV needs a rig, use a simple skeleton (`root`, `body`, `claw_arm`, `claw_jaw_l/r`, `sonar`) named by the art lead; these are new bones and not Mannequin bones. [PROPOSED]

### 5.2 Texturing and Nanite
- Single 4K set at 10.24 px/cm (hero prop, seen close), ORM + DirectX normal + albedo; yellow paint with chipped edges, grime.
- Opaque shell and parts: Nanite, no manual LODs, except parts that deform or animate by bone (verify: Nanite and skeletal meshes). If the ROV is a skeletal mesh use manual LODs instead. [VERIFY]
- Light lenses and any acrylic sonar dome: non-Nanite, translucent Substrate. [SCOPE]
- Emissive masks for LED lights.

### 5.3 Collision
`UCX_` hulls: one main body hull, one per thruster shroud, one for the claw. Example: `UCX_SM_ROV_Scout_Body_01`. [SCOPE]

### 5.4 UE destination
`unreal/Content/Props/ROV_Scout/` per section 2.3.

---

## 6. Asset: `ENV_RiverWalk_Dock` (reference 8.5)

**Higgsfield reference:** 8.5 RiverWalk marina concept, job `1fb9d90f-195c-48cf-bf0f-bc6525146bdb` (2688x1152), local `assets/hf_20261114_121946_1fb9d90f-195c-48cf-bf0f-bc6525146bdb.png`. Alternate lower-resolution job `339edeac-a2c3-4408-a4c6-d8fb66d695d5` (1344x576), not active.

**Details taken from the 8.5 image (viewed directly):**
- Waterline eye-level view at twilight, choppy dark water, corpses floating at the water surface in the foreground.
- A concrete seawall and promenade on the left with the stencilled sign "Detroit RiverWalk" (with a wave logo) on the wall face; above it, wrought-iron railings with ornate scrollwork bands and finials.
- Ornate curved-arm lamp posts with lantern heads (a row receding into fog); a park sign "Milliken State Park" mounted between two posts; overgrown vegetation and tree branches.
- Derelict fuel dock: a rust-streaked red-and-white canopy with "FUEL" lettering held on four steel posts, two vintage fuel pumps on a timber deck, a fluorescent tube light under the canopy.
- Heavy squared timber pilings with mussel encrustation, a plank-deck dock running along the seawall, and several listing white pleasure boats (a cabin cruiser in the foreground, others further right).
- Skyline backdrop: Renaissance Center cylinder towers with blue ring lights in fog plus other downtown towers (not modelled; backdrop only).

### 6.1 Module list and budgets [PROPOSED]

| Module | Description | Tris budget | Density [PROPOSED] |
| :--- | :--- | :--- | :--- |
| `SM_RW_Seawall_Straight` | concrete seawall and promenade curb, 4 m | 2,000 to 4,000 | 5.12 |
| `SM_RW_Seawall_Corner` | corner piece | 2,500 to 5,000 | 5.12 |
| `SM_RW_Promenade_Slab` | broken concrete promenade slab | 1,000 to 3,000 | 5.12 |
| `SM_RW_Railing_Wrought_Straight` | ornate wrought-iron railing, 2 m | 6,000 to 10,000 | 5.12 |
| `SM_RW_Railing_Post` | post with finial | 1,500 to 3,000 | 5.12 |
| `SM_RW_Lamp_Post` | ornate curved-arm lamp | 6,000 to 10,000 | 10.24 |
| `SM_RW_Sign_Milliken` | park sign on posts | 2,000 to 4,000 | 5.12 |
| `SM_RW_Sign_Wall_RiverWalk` | stencilled wall sign decal mesh | 500 | 5.12 |
| `SM_RW_Dock_Deck_Plank_4m` | plank deck section | 3,000 to 6,000 | 5.12 |
| `SM_RW_Piling_A/B/C` | timber piling variants, mussel growth in the texture | 2,000 to 5,000 | 5.12 |
| `SM_RW_FuelDock_Canopy` | red-and-white canopy with "FUEL" sign and posts | 20,000 to 35,000 | 10.24 | 
| `SM_RW_FuelPump` | vintage pump (x2) | 8,000 to 15,000 | 10.24 | 
| `SM_RW_FuelDock_Deck` | dock platform under the canopy | 4,000 to 8,000 | 5.12 |
| `SM_RW_Boat_Cruiser_Listing` | derelict cabin cruiser hull (hero silhouette) | 60,000 to 100,000 | 5.12 |
| `SM_RW_Boat_Small_Listing` | smaller boat variant | 30,000 to 50,000 | 5.12 |
| `SM_RW_Ladder_Dock` | access ladder to water | 1,500 to 3,000 | 5.12 |
| Vegetation, skyline | trees, overgrowth, city towers | not in this asset | not covered; tree and foliage handled in-engine or in a later task |

The fuel pump hose and nozzle are the siphon interaction (Act 2, Choice B); model hose attach points and a nozzle as separate meshes. [PROPOSED]

### 6.2 Texturing
Trim sheets for wood planks, timber with mussel growth, concrete, rusted iron, painted sheet metal (red/white) at 5.12 px/cm, 4K, ORM + DirectX normal + albedo. Fuel pump, lamp post and canopy sign are hero-detail at 10.24 px/cm.

### 6.3 Nanite and water
- Opaque dock, seawall, pilings, boats' hulls: **Nanite, no manual LODs.** [SCOPE]
- Boat windshields, cabin windows, lamp lenses: non-Nanite translucent Substrate. [SCOPE]
- **Water is not modelled in Blender.** The river surface and foreground corpses' water interaction are UE SingleLayerWater / Substrate translucent water materials with refraction and caustics (8.5 reference note). Model only waterline-clipped geometry so no hull geometry pokes through the surface incorrectly. [SCOPE for "water on non-Nanite translucent Substrate"; the no-water-mesh rule is PROPOSED]
- Seawall, pilings and hulls must extend well below the waterline (at least 2 m) so underwater views from the swim route do not show open edges. [PROPOSED]

### 6.4 Collision
`UCX_` hulls per module: walkable deck (single box hull per plank section), seawall, pilings (cylinder-approximating convex), boat hulls (one to three hulls), lamp posts. Example: `UCX_SM_RW_Dock_Deck_Plank_4m_01`. [SCOPE]

### 6.5 UE destination
`unreal/Content/Environments/RiverWalk/` per section 2.3.

---

## 7. Nanite and material rules (all environment and prop assets)

| Material | Nanite | Material model |
| :--- | :--- | :--- |
| Opaque steel, concrete, wood, painted metal, fuel pump, ROV shell | **Yes**, no manual LODs | standard opaque; Substrate if enabled project-wide (see the Substrate prerequisite in section 1) |
| Acrylic dome, viewport glass, portholes, gauge lenses, windshields | **No** | translucent Substrate with custom refractive caustics |
| Water (river surface) | **No** | translucent Substrate / SingleLayerWater |
| Skeletal meshes (Christian) | not assumed; manual LOD0 to LOD3 | Subsurface Profile skin (8.2 reference); hair anisotropic |

Translucent rows require the Substrate prerequisite from section 1.

Rules:
- Never put a translucent surface and an opaque surface on the same mesh element; separate glass into its own mesh or its own FBX so UE can disable Nanite per mesh.
- Nanite static meshes are imported with "Build Nanite" on and LODs not generated.
- Masked/alpha-clipped foliage and hair cards are not Nanite meshes in this sheet. [PROPOSED]

## 8. Collision rules (all assets)

- Prefix `UCX_` followed by the exact name of the render mesh: `UCX_<MeshName>`; when more than one hull is needed append `_01`, `_02`. [SCOPE]
- Convex only; each hull is a separate object in the same FBX as its render mesh.
- Hulls are authored in the same space as the mesh (same origin, same scale 1.0 applied) and hidden from render by being exported only as collision (UE strips them on import).
- Keep hulls inside the visual mesh by a few millimetres, with a minimum 60 cm clear walkway width inside Lab 04 corridors for the capsule. [PROPOSED]
- UE import: "Auto Generate Collision" off for any mesh with `UCX_` hulls.

## 9. Pipeline status mapping (from `PROJECT_MEMORY.md` section 3)

| Asset ID | Higgsfield refs | Active job IDs |
| :--- | :--- | :--- |
| `CHR_Christian_01` | 8.1, 8.2 | `58c39f98-e7f8-4a0e-bb18-7a3c97104f78` (8.1), `673f6cc5-ad50-45f9-9975-404221147ffd` (8.2), element `4284877d-9158-4176-98aa-f7344abff977` |
| `ENV_Lab04_Int` | 8.3 | `a583d3d9-57d8-4eba-9bab-cd31e6346ac9` |
| `PROP_ROV_Scout` | 8.4 | `e831062e-80f2-4af1-af80-0f8ea43f6e7a` (seed `1925040229`) |
| `ENV_RiverWalk_Dock` | 8.5 | `1fb9d90f-195c-48cf-bf0f-bc6525146bdb` |

This task does not update `PROJECT_MEMORY.md`; the orchestrator updates the status table as assets reach `.blend`, `.fbx` and UE-imported.

## 10. Open items summary

- Axis pair for FBX export (section 1.1): prove with the smoke test.
- Manny bone parenting, axes, optional corrective bones: verify in the UE 5.8 Skeleton editor.
- 8.2 and 8.4 are video; 8.2 and ROV details are partly from prompt text only.
- Whether hair cards, mask and watch count inside the Christian 45k to 65k LOD0 figure.
- Nanite on skeletal meshes for the ROV rig and Christian: assumed off.
- All budgets marked [PROPOSED] are this sheet's professional call, except Christian LOD0 45k to 65k tris [SCOPE].
