# 40 Days Under: FBX Export Checklist (Blender 4.x to UE 5.8)

Companion to `Docs/Art/ProductionSheet.md`. Standards come from `scope.md` section 9. Labels: **[SCOPE]** fixed by scope.md, **[PROPOSED]** this document's choice, **[VERIFY]** prove with the smoke test (section 4) before relying on it.

Blender FBX option names below are those of the Blender 4.x exporter (File > Export > FBX). If a label differs in your build, match the intent and note it here.

---

## 1. Pre-export checklist (tick every item per asset)

### 1.1 Scene and units
- [ ] Scene Unit System = Metric, Unit Scale = 1.0, Length = Meters (1 unit = 1 m). [SCOPE]
- [ ] Asset built facing +X, +Z up. [SCOPE]
- [ ] Origin placed at the asset's pivot (characters: between the feet at world origin; modules: grid corner or functional pivot).
- [ ] Rotation and scale applied (Ctrl+A > All Transforms) on every mesh; scale reads 1.0, rotation 0.
- [ ] Christian only: scale-check reads 1.7526 m (175.26 cm) head-top to sole. [SCOPE]

### 1.2 Mesh hygiene
- [ ] No n-gons on deforming areas; triangulation will occur on export (see Triangulate Faces, section 2).
- [ ] Normals correct (face orientation overlay all blue outside); no flipped faces.
- [ ] Custom split normals or weighted normals finalized; Auto Smooth is not used (Blender 4.1 and later removed it; use Shade Smooth plus weighted normals). [PROPOSED]
- [ ] No loose geometry, no zero-area faces, no doubles.
- [ ] Every UV set: UV0 is the texture set (texel density 10.24 px/cm hero, 5.12 px/cm environment, 4096 px); UV1 reserved for lightmaps only if the asset is not Nanite or Lumen-only. [SCOPE for densities]
- [ ] Materials named `M_<Name>` (one slot per material), matching UE slot names.
- [ ] Object names are the final UE names with prefixes (`SM_`, `SK_`, `UCX_`).

### 1.3 Collision (static meshes)
- [ ] Hulls named `UCX_<MeshName>` or `UCX_<MeshName>_NN`, convex, scale applied. [SCOPE]
- [ ] Hulls selected in the export along with their render mesh (they go in the same FBX).

### 1.4 Skeletal (Christian, and ROV if rigged)
- [ ] Armature is a single object at scale 1.0, rotation 0; mesh children parented with an Armature modifier.
- [ ] Bone names match the Manny/Quinn names in ProductionSheet section 3.4 exactly (lowercase, underscores). Unverified items there are checked against `SK_Mannequin` in engine. [VERIFY]
- [ ] Non-deforming IK/utility bones exist and are exported (root, ik_foot_root, ik_foot_l/r, ik_hand_root, ik_hand_gun, ik_hand_l/r, interaction, center_of_mass). [VERIFY parents]
- [ ] Twist bones present: upperarm, lowerarm, thigh, calf, each `_twist_01` and `_twist_02`, `_l` and `_r`.
- [ ] Max 4 skin influences per vertex (LOD0) unless the project enables more. [PROPOSED]
- [ ] Shape keys: Basis plus the 52 ARKit keys named exactly as in ProductionSheet section 3.6. [SCOPE]
- [ ] Rest pose applied as bind pose; no animation baked in the mesh FBX.
- [ ] LODs: Blender cannot write FBX LOD groups. If LODs are authored, export one FBX per LOD (`CHR_Christian_01_LOD1.fbx` etc., same skeleton and bone names) and import each with the UE Skeletal Mesh "Import LOD" workflow after LOD0. Otherwise generate LODs in UE. [PROPOSED]

---

## 2. Blender FBX export settings (tick-list)

### Include
- [ ] Limit to: Selected Objects. [PROPOSED]
- [ ] Object Types: Mesh and Armature only (untick Empty, Camera, Light, Other). Static meshes: Mesh only.
- [ ] Custom Properties: off.

### Transform
- [ ] **Scale = 1.0**, **Apply Scalings = FBX Units Scale**. With Blender scene units at 1 unit = 1 m, this writes centimetre units into the FBX so UE imports at the correct size (a 1.7526 m character becomes 175.26 UU). [SCOPE for the 1 unit = 1 m / 1 cm = 1 UU rule; verify result in the smoke test]
- [ ] **Forward / Up:** set to the pair proven in the smoke test. Authoring convention is +X forward, +Z up. **Starting pair to try first: Forward = X, Up = Z** [PROPOSED starting value, not proven]. The smoke test (section 4) confirms or corrects it; record the final pair here: Forward = ____ , Up = ____ . Reconciling with the Manny reference armature: see ProductionSheet section 3.4 Orientation rule (rotate the imported Manny to face +X once; never change authored facing).
- [ ] **Apply Unit** on (Blender label for the unit conversion; keep consistent with Apply Scalings above). [PROPOSED]
- [ ] **Use Space Transform** on. [PROPOSED]
- [ ] **Apply Transform** on. [SCOPE] (Blender marks this option experimental; if it breaks armature or animation orientation, tell the orchestrator rather than switching it off silently. The smoke test must include a skinned bone to catch this.)

### Geometry
- [ ] **Smoothing = Face.** [SCOPE]
- [ ] Apply Modifiers on (bevel, weighted normal, mirror applied into the export). For skeletal meshes with shape keys, Blender cannot apply modifiers on a mesh that has shape keys without breaking them: apply or remove modifiers on those meshes first, then leave Apply Modifiers off. [PROPOSED]
- [ ] Loose Edges off.
- [ ] Triangulate Faces on (matches Blender and UE triangulation; do before baking normals, so bake and export triangulation agree). [PROPOSED]
- [ ] Tangent Space off. Custom normals are exported; UE imports normals and computes MikkTSpace tangents; baked maps used MikkTSpace. [PROPOSED]
- [ ] Vertex Colors: on for assets using vertex-paint masks, otherwise off.

### Armature
- [ ] **Add Leaf Bones off.** [SCOPE]
- [ ] **Only Deform Bones off** (so IK and utility bones export). [PROPOSED]
- [ ] Primary Bone Axis / Secondary Bone Axis: must match the Mannequin's bone orientation. Blender defaults are Y and X. **[VERIFY]** Compare against an exported Manny; adjust here, not in per-bone rolls, if the smoke-test pose is wrong.
- [ ] Armature FBXNode Type: Null (default). [PROPOSED]

### Animation
- [ ] Bake Animation off for mesh-only FBX. Animation FBX (later) is a separate export with Bake Animation on, NLA Strips off, All Actions off, Force Start/End Keying on. [PROPOSED]

### Path
- [ ] Path Mode: Auto (or Copy only with Embed Textures; do not embed textures, import them as separate files). [PROPOSED]
- [ ] Output to `art/<AssetID>/export/` with the file name from ProductionSheet section 2.2.

---

## 3. UE 5.8 import settings, per asset

- [ ] Prerequisite before any glass, acrylic or water material work: Substrate is enabled in the project (`r.Substrate=1`, editor restart required). Not set in `unreal/Config` at the time of writing; engine owner enables it, this document does not edit Config. [VERIFY]

General for all: Import Uniform Scale 1.0. Material Import Method: "Do Not Create Material" (materials are made in UE; Substrate for translucent). Normal Import Method: **"Import Normals"** (or "Import Normals and Tangents"), so Blender custom/weighted normals survive; Tangents computed as MikkTSpace ("Compute Tangents"/MikkTSpace, exact label per UE 5.8 import dialog [VERIFY]). Do not use "Compute Normals" on hard-surface assets. [PROPOSED]

### 3.1 `CHR_Christian_01` (SK_Christian)

| Setting | Value |
| :--- | :--- |
| Import type | Skeletal Mesh |
| Skeleton | new skeleton on first import (`SKEL_Christian`); reuse it for later imports |
| Import Mesh / Import Animations | Mesh on, Animations off |
| Import Morph Targets | **on** (the 52 ARKit keys) |
| Update Skeleton Reference Pose | off on first import |
| Use T0 as Ref Pose | on [PROPOSED] |
| Preserve Smoothing Groups | on |
| Build Nanite | off [PROPOSED; verify skeletal Nanite status in 5.8] |
| LODs | import LOD0 FBX first, then each `_LODn.fbx` via Skeletal Mesh Import LOD; or off and use UE reduction |
| Physics Asset | create new (`PHYS_Christian`) |
| Vertex Color | Ignore, unless a skin-tone mask is used |
| Material slots | named per Blender materials |
| Collision | none from the FBX (Physics Asset instead) |
| Destination | `unreal/Content/Characters/Christian/Meshes/` |
| Post-import checks | bounding box Z reads about 175.26 cm; mesh faces +X in the preview; open Skeleton editor and compare to `SK_Mannequin`; retarget with the IK Rig [VERIFY] |

### 3.2 `ENV_Lab04_Int` (kit modules, one FBX per module)

| Setting | Opaque hull modules | Glass / acrylic modules |
| :--- | :--- | :--- |
| Import type | Static Mesh | Static Mesh |
| Build Nanite | **on** [SCOPE] | **off** [SCOPE] |
| Generate Lightmap UVs | off (Nanite and Lumen) | off |
| Auto Generate Collision | off (using `UCX_` hulls) | off |
| Collision source | `UCX_` hulls imported [SCOPE] | none, or a simple hull if needed |
| LODs | none (no manual LODs) [SCOPE] | none |
| Combine Meshes | off | off |
| Material | Substrate opaque or standard | translucent Substrate |
| Build Scale / Uniform Scale | 1.0 | 1.0 |
| Destination | `unreal/Content/Environments/Lab04/Meshes/` | same |

### 3.3 `PROP_ROV_Scout`

| Setting | Value |
| :--- | :--- |
| Import type | Static Mesh per module (body, thrusters, claw, sonar); Skeletal Mesh only if rigged |
| Build Nanite | on for opaque static parts [SCOPE]; off for lenses and domes [SCOPE]; off if skeletal [VERIFY] |
| Collision | `UCX_` hulls imported; Auto Generate off |
| Pivots | at rotation centers for thruster, claw, sonar |
| Destination | `unreal/Content/Props/ROV_Scout/Meshes/` |

### 3.4 `ENV_RiverWalk_Dock` (modules)

| Setting | Opaque modules (dock, seawall, pilings, hull) | Glass modules (windshields, lamp lenses) |
| :--- | :--- | :--- |
| Import type | Static Mesh | Static Mesh |
| Build Nanite | **on** [SCOPE] | **off** [SCOPE] |
| Auto Generate Collision | off | off |
| Collision source | `UCX_` hulls [SCOPE] | none |
| LODs | none [SCOPE] | none |
| Generate Lightmap UVs | off | off |
| Water | not exported from Blender; UE SingleLayerWater | n/a |
| Destination | `unreal/Content/Environments/RiverWalk/Meshes/` | same |

### 3.5 Texture import (all assets)

| Map | File suffix | sRGB | Compression |
| :--- | :--- | :--- | :--- |
| Albedo | `_BC` | on | Default (DXT/BC) |
| Normal (DirectX, -Y green) | `_N` | off | Normalmap; Flip Green Channel off [SCOPE] |
| ORM (R occlusion, G roughness, B metallic) | `_ORM` | off | Masks (no sRGB) [SCOPE] |
| Emissive / mask | `_MSK`, `_E` | off for masks | Masks or Default |
| Hair tangent | `_Tangent` | off | Masks or Default (no sRGB) [PROPOSED] |

Texture size 4096 x 4096. Mip generation on, Never Stream off only for hero close-up textures if needed. [PROPOSED]

---

## 4. Pipeline smoke test (run once before the first real export) [VERIFY]

1. In Blender, create a 1 m cube with a visible nose or arrow along +X and a 1.7526 m tall capsule with a single-bone armature.
2. Export with the settings in section 2 and a trial Forward/Up pair.
3. Import into a scratch UE project folder (do not touch production folders).
4. Confirm: size (capsule = 175.26 cm), nose points to UE +X, no -90 or 90 degree rotation on the actor, bone points as the Mannequin's does, shading uses face smoothing, no leaf bones in the skeleton tree.
5. Write the working Forward/Up pair into section 2 of this file.
6. Delete the scratch assets.

## 5. Per-export sign-off

- [ ] Asset ID: ____________  Date: ____________  Blender file: ____________
- [ ] Section 1 ticked  [ ] Section 2 ticked  [ ] Section 3 row applied  [ ] Post-import checks passed
- [ ] `PROJECT_MEMORY.md` section 3 status updated by the orchestrator after import.
