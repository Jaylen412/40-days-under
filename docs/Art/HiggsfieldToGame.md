# From Higgsfield output to a playable game

Higgsfield output comes in two kinds, and they reach the game in different ways.

| Kind | Example | How it gets into the game |
| :--- | :--- | :--- |
| Pre-rendered footage | The realism clips | Played as video (done: the intro cold open, below) |
| 3D geometry | GLB meshes from image-to-3D, Scene Builder `.glb` / `.blend` | Cleaned in Blender, exported as FBX, imported into UE as real, walkable assets |

A video can't be walked through. To play the game *in* the Higgsfield look, the stills have to become
3D assets: meshes, PBR textures and a rigged character. The realism stills then serve as the target
look and the 3D sources.

## Done: Higgsfield clips in the game

- Clips: `unreal/Content/Movies/Higgsfield/*.mp4` (H.264/AAC, 1280x720, from `realism-results.json`).
- Order: `unreal/Content/Data/Narrative/opening_slice_footage.json`, listing all 13 planned clips. Missing
  files are skipped, so a newly finished clip only needs dropping into the folder under its planned name.
- Playback: `UIntroFootageComponent` on `AOpeningSliceActor` plays them full screen with audio when
  `L_OpeningSlice` starts. Then it returns control and starts the slice director.
  Space / gamepad A skips one clip. Esc / gamepad Start skips them all. To turn it off, untick
  `IntroFootage > bEnabled` on the level's `OpeningSliceDirector`. This also applies in `L_OpeningSlice_Preview`.
- Packaging: `Config/DefaultGame.ini` stages `Content/Data` and `Content/Movies` as loose files.

## Progress (2026-11-15)

- Step 2 done: layout sheet `assets/hf_20261115_101346_1e5964b6-….png`.
- Step 3 proven on the valve wheel: still, then `sam_3_3d`, then `blender/props/build_valve_wheel.py`, then
  `unreal/Scripts/import_valve_wheel.py`. It's in `L_OpeningSlice`. Cost: 3 credits in total for the layout sheet, the still and the 3D lift.
- Cot, bench, log recorder and air scrubber done the cheaper way: one prop sheet image with all four, then a
  SAM 3D lift per prop picked out by text prompt (5 credits for the four). Scripts:
  `blender/props/build_lab04_props.py`, then `unreal/Scripts/import_lab04_props.py`. They're placed in both opening-slice levels.
- The manifold, telemetry panel and watch are modelled by hand (DEC-013), because their gauges, lamp and screen are live.

## To build the game in the Higgsfield look

1. **Finish and lock the footage.** Generate the 6 missing clips (06 part 2 through 10) before the free
   trial ends on 2026-11-17 at 06:17 EST. Regenerate 05 (camera moves) and 06 part 1 (lamp goes green too
   early). Pick one room layout. The stills disagree on it, and a 3D level can only have one.
2. **Lock a Lab 04 layout sheet.** Generate one wide, top-down or orthographic plan of Lab 04 from the
   interior concept (8.3), placing every station: cot, manifold, bench, telemetry panel, intake valve and dome.
   It becomes the blockout that replaces the placeholder graybox layout.
3. **Props to 3D (Higgsfield `generate_3d`, image_to_3d with PBR).** Generate one clean isolated still per
   hero prop on a plain background: the manifold with four gauges, the valve wheel, the telemetry panel and
   bus tray, the recorder, the dive watch, the cot and the bench. Convert each still to a GLB. Use
   multi_image_to_3d when 2 to 4 views exist. Output is a single-object mesh with textures.
4. **Room shell.** Build the bulkheads, deck and dome in Blender as modular pieces, sized to the layout sheet.
   Image-to-3D isn't good at whole interiors. Generate tileable PBR texture sets (painted steel, rubber
   deck, acrylic) as images, matched to the realism stills. Higgsfield Scene Builder can assemble a scene and
   export `.blend`/GLB as a starting point.
5. **Christian.** Run image-to-3D on the turnaround sheet (anchor 8.1), using multi-view if front, side and
   back can be cropped apart. Then rig it to the Manny skeleton, because DEC-009 and the existing Blender
   animations depend on that skeleton. Use Higgsfield `3d_rigging` or the Blender retarget. In first person,
   only the arms and the wrist watch are on screen, so they get the most quality.
6. **Blender cleanup and export.** For every GLB: check scale (Christian is 175.26 cm), set pivots,
   cut the polygon count, add LODs and simple collision, and check the UVs. Then export
   FBX per `docs/Art/FbxExportChecklist.md`. Track each asset in the `PROJECT_MEMORY.md` section 3 table.
7. **Unreal look-dev.** Import into `Content/Environments/Lab04/` and `Content/Characters/Christian/`.
   Swap the graybox meshes in `L_OpeningSlice` one at a time, keeping the `INT_NN_*` interactables on the
   new props. Match the realism stills with Lumen, post-process (colour grade, film grain, underwater
   fog) and the green dome light. The stills are the target: compare screenshots side by side.
8. **Decide the clips' long-term role.** Once the level looks like the footage, a full-length intro that
   replays the same beats becomes redundant. Options: keep only one establishing clip, play the clips on
   in-world screens (CCTV, log playback), or keep them as a trailer.

## Costs and limits to know up front

- Image-to-3D gives one object per image. Plan on hand-built room geometry.
- Generated meshes are dense, with baked lighting in the textures. Expect cleanup on every asset.
- Faces and hands generated as 3D are the weakest part. Close-ups of Christian may need a hand-sculpted pass.
- Each `generate_3d` call costs credits. Use `get_cost:true` to get a quote before submitting.
  Whether the free trial covers 3D hasn't been checked. Assume it doesn't until a quote shows otherwise.
