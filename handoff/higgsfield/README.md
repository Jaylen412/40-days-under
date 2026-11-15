# Higgsfield handoff: Day 1 opening slice, realism pass

Purpose: turn the graybox blocking of the ten opening beats into realistic footage. The stills in
`frames/` are composition and pose references only (box stand-in, flat-colour set). Identity, costume
and environment come from the locked anchors below, not from these stills.

Nothing has been sent to Higgsfield from this folder yet.

## Locked anchors (from `PROJECT_MEMORY.md` section 1)

| Anchor | Higgsfield ID | Local copy |
| :--- | :--- | :--- |
| Christian Thomas reference element | `4284877d-9158-4176-98aa-f7344abff977` | `assets/hf_20261114_120221_58c39f98-e7f8-4a0e-bb18-7a3c97104f78.png` (turnaround, job `58c39f98-e7f8-4a0e-bb18-7a3c97104f78`) |
| Visor portrait video | job `673f6cc5-ad50-45f9-9975-404221147ffd` | `assets/hf_20261114_120414_673f6cc5-ad50-45f9-9975-404221147ffd_min.mp4` |
| Lab 04 interior concept | job `a583d3d9-57d8-4eba-9bab-cd31e6346ac9` | `assets/hf_20261114_121052_a583d3d9-57d8-4eba-9bab-cd31e6346ac9.png` |

Base prompt text and negative prompts: `scope.md` section 8 (8.1 character, 8.3 interior). Reuse them
unchanged and append the per-shot action below.

## How to use each still

1. Start frame: the still for the shot, restyled with the Christian element and the Lab 04 interior
   concept as references (image-to-image), so pose and framing hold while the look becomes real.
2. Video: image-to-video from that restyled frame with the shot's action line as the motion prompt.
3. Keep the camera as listed. Only shots 08 and 09 move (slow push-in); all others are locked off.

The stills are 1920x1080 (16:9). The concepts are 21:9; crop top and bottom if 21:9 output is wanted.

## Shots

Frames are Blender frames at 30 fps; subtract 1 for the Unreal sequence `LS_OpeningSlice`.

| # | Beat id | Still | Frames | Secs | Lens | Action for the motion prompt | Line |
| :-- | :--- | :--- | :--- | :-- | :-- | :--- | :--- |
| 1 | `wake_pump` | `01a_wake_pump_lying.jpg`, `01b_wake_pump_sitting.jpg` | 1-600 | 20 | 22 mm | Lies on a cot, turns his head toward the hull to listen, sits up, rubs his face with both hands, stands. | That thump. Pump's still running. Okay. Still running. |
| 2 | `read_manifold` | `02_read_manifold.jpg` | 601-1650 | 35 | 28 mm | Walks to the wall manifold, scans four analog gauges left to right, taps one glass with a finger. | Manifold first. Pressure's holding, scrubber needle is in the green. Good. Boring is good. |
| 3 | `check_watch` | `03_check_watch.jpg` | 1651-2400 | 25 | 35 mm | Raises his left wrist, presses the dive watch twice with his right hand, looks down at it, exhales. | Watch says fifteen meters, and the nitrogen is climbing like it does every hour. Seventeen hours to decompress when I finally go up. Not today. |
| 4 | `log_stock` | `04_log_stock.jpg` | 2401-3300 | 30 | 32 mm | At the lab bench, lifts a small recorder to his mouth and speaks, counting items off with his free hand. | Day one log. Diesel tank, 2000 litres, burning 2 litres an hour. Desalinator makes 20 litres of water a day. Pantry count: 700 meals. Do the math later. Math is for later. |
| 5 | `telemetry_fault` | `05_telemetry_fault.jpg` | 3301-4200 | 30 | 20 mm | Looks up at an announcement, turns toward a wall panel where a red fault lamp blinks, lowers the recorder, walks over. | (Station) Hydrophone array, channel three, no signal. |
| 6 | `clear_fault` | `06_clear_fault.jpg` | 4201-5400 | 40 | 32 mm | Works at the telemetry panel, pulls a bus tray out and pushes it back with both hands, presses the lamp; it turns green. | Channel three. I wrote this firmware. Of course it's channel three. Reseat the bus, clear the fault, don't take it personally. |
| 7 | `valve_stuck` | `07_valve_stuck.jpg` | 5401-6150 | 25 | 30 mm | Grips a yellow valve wheel on the intake bulkhead, strains twice, it does not move; steps back and shakes out his hand. | Intake bulkhead valve is seized. Zebra mussels, probably. Manual crank, then. |
| 8 | `crank_valve` | `08_crank_valve.jpg` | 6151-7500 | 45 | 28 mm, slow push-in | Braced with knees bent, hauls the wheel hand over hand; it moves in jerks that get faster. | Come on. Come on. There. It's moving. Slow and steady, nobody's timing me. |
| 9 | `dome_look` | `09_dome_look.jpg` | 7501-8400 | 30 | 18 mm low angle, slow push-in | Walks to the centre of the room, looks up through the acrylic dome, takes half a step back and holds still. Dark human silhouettes drift against pale green light above. No gore, shapes only. | Okay. Look up. Just once. ...They're still up there. Hundreds of them, blocking the light. Not coming down. They can't come down. |
| 10 | `log_close` | `10_log_close.jpg` | 8401-9000 | 20 | 35 mm | Looks down, walks back to the bench, speaks into the recorder, sets it down, glances up over his shoulder at the dome. | End of entry. Lab 04 is stable. Supply boat comes in three weeks. I just have to keep the lights on until then. |

## Constraints to carry into every prompt

- No screen HUD or overlay text. Readouts live on the wall gauges, the wrist watch and the mask.
- Tone: calm, domestic, slightly funny. Shot 9 is the only time the horror shows.
- Christian is 175.26 cm; the stand-in in these stills is 180.6 cm, so do not take scale from it.
- Spoken numbers must stay exact: 2000 L, 2 L per hour, 20 L of water a day, 700 meals, 17 hours.

## Known limits of the reference stills

- The cot and where each station sits around the wall are placeholder layout, not from the interior concept.
- Feet are not planted and hands do not lock to props; treat poses as intent, not final contact.
- The recorder stays on the bench in shots 4 and 10 although he "lifts" it.
- One still per beat (two for beat 1). No reference video was rendered.

## Sources

- Animation and cameras: `blender/starting-animation.blend`
- Beat data and lines: `unreal/Content/Data/Narrative/opening_slice.json`, `docs/Narrative/OpeningSlice.md`
- In-engine preview: `unreal/Content/Maps/L_OpeningSlice_Preview` with `Cinematics/LS_OpeningSlice`
