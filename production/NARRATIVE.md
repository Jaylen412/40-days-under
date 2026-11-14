# 40 Days Under — narrative implementation packet

Status: authored production script and content specification; no playable 3D build is implied. Version 1.0. The executable simulation and `state_contract.json` own survival facts. Narrative never sets fuel, health, nitrogen or branch state directly.

## World and continuity

Christian Thomas is 28, African American, 5′9″ and 150 lb, lean and muscular, with rich brown skin and short 360 waves. His competence is specific: he recognizes Lab 04's telemetry board revision, understands its old service bus and knows which ROV parts can be repurposed. He can still misdiagnose a fault under stress. His dry humor comes from familiarity, not fearlessness.

Lab 04 is a fictional **ambient-pressure** benthic station at a fictional 15 m site in the Detroit River. Its equipment and habitation areas maintain a pressurized atmosphere; the game’s saturation mechanic does not apply to an ordinary one-atmosphere submarine. The fixed 17-game-hour decompression cycle is fiction for balancing, never a real diving procedure. The surface diesel plant supplies the station’s life-support network; powered circulation/scrubbing and breathable-air supply are distinct failures.

A sealed pressure-transfer capsule docks at the airlock. After the fictional decompression cycle it carries Christian to a shallow launch point before he enters the river; it docks again for his return and repressurization. He does not open a depressurized airlock directly into water at 15 m. A dock schematic beside the service interlock establishes this equipment during the opening. The reference simulation abstracts capsule transit within its travel phases; engine implementation must supply the actual sequence.

The RiverWalk, Milliken harbor, Belle Isle channel, Renaissance Center, Ambassador Bridge and Windsor inform a compressed fictional map. They are not one simultaneous postcard sightline. The 600 m extraction route and 2 km trawler contact are level distances, not surveyed directions to a real site. The nearby wreck is fictional. Detroit's river water goes through freshwater treatment; the legacy console's outdated “DESAL” stencil is crossed out by Christian with “FILTER / DISINFECT.”

The infected cannot swim, pursue underwater or propel themselves toward sound while afloat. Current fields carry buoyant bodies; snag points accumulate them. Mobile infected on docks can follow a decoy. The sound the “living ceiling” makes comes from intermittent mouths above water, contact with moorings and hull-transmitted impacts; no explanatory supernatural underwater chorus is required.

Environmental story props:

- A faded maintenance badge: `C. THOMAS / AUTONOMOUS SYSTEMS / CONTRACTOR`. An old handwritten service tag reads `DON'T PATCH THE SENSOR TO HIDE THE LEAK — CT`.
- A scratched route sketch with separate inset cards for the harbor and upriver contacts. It implies local knowledge without claiming a navigation chart.
- Canned meals stacked into an improvised equipment shelf: abundance in one resource, scarcity in fuel and usable parts.
- A family voicemail stored locally: an ordinary request to call back. It does not confirm whether Christian's relatives survived.
- A supply manifest stamped `EXPECTED / DAY 21`. On Day 21, “expected” becomes a word Christian cannot stop looking at.

Voice direction: Christian speaks quietly, usually while doing something. Avoid announcing visible actions repeatedly. He distrusts instruments for identifiable reasons before isolation begins producing false perception. Hallucinations are authored perception layers: they never corrupt authoritative simulation data or permanently conceal an essential alarm. A verified instrument path always remains available.

## Opening: “The Pump Has a Rhythm”

Target: 300 seconds of a first successful playthrough, Day 1. Events gate progression; these are pacing targets, not deadlines. Christian remains inside Lab 04 throughout. External exploration is through the tethered drone. No cutscene takes control away for more than the brief authored wake transition.

The tutorial uses a finite, visibly connected reserve cartridge and an isolated low-load maintenance circuit. The tutorial advances at one game second per wall second. After 20 inactive seconds, it supplies a hint; after 40, an explicit tutorial pause freezes every survival clock until meaningful input. The runtime must implement this adapter policy in `vertical_slice.json`; it is not currently implemented by the reference simulation. Reserve state and completion flags persist so reloading cannot refill supplies. Once the tutorial ends, the regular simulation time scale starts. An accessible wrist/caption objective mirror can be enabled without requiring physical-gauge legibility.

| Target time | Player verb and gate | Playable staging, exact lines and feedback |
|---|---|---|
| 0:00–1:00 | Wake, stand, inspect pump monitor, compare with mechanical gauge | Pump thumps steadily beneath a hanging mug. On looking at the panel, a stale telemetry reading turns amber; the independent mechanical gauge remains stable. Christian: “Same pump. Same little hitch every fourth stroke.” Then: “Pressure's holding. The screen's lying.” Goal on the physical work card: `COMPARE THE TWO READINGS`. An optional interaction hint appears on the wrist mirror, not in world space. |
| 1:00–2:00 | Open service panel; isolate marked low-voltage diagnostic circuit; reseat connector; run self-test | Three labeled terminals are visible: `POWER`, `RETURN`, `SENSOR`. The service card shows `SENSOR ↔ INPUT 03`; its existing clip sits loose on `INPUT 02`. The player snaps the sensor clip onto 03 using a large interaction target. Christian: “Revision C. They never changed the labels.” Wrong selection gives a reversible diagnostic mismatch and no damage. Successful self-test reads `STALE PACKETS CLEARED / SENSOR AGREES`. Christian: “There you are.” This is a fictional abstract electronics puzzle, not a repair procedure for real life support. |
| 2:00–3:00 | Consult service tablet; choose validated firmware rollback; confirm independent gauge | Tablet offers `REBOOT ALL`, `SUPPRESS ALERT`, `RESTORE SIGNED SENSOR PROFILE`. The first two explain why they are inappropriate and return to selection; only the third executes. The player compares the stable gauge and green heartbeat counter. Christian: “Don't silence it. Fix what it's telling you.” Circuit switch restores the diagnostic channel. A low hull groan arrives from the port side after success; the player can inspect it without a timed penalty. |
| 3:00–4:00 | Release inspection drone from its wet-side rack by remote control; steer camera; tag intake obstruction | Christian stays at the console. A camera feed shows drifting silt and a yellow tether; looking into the current slows drone progress. The player tags a plastic sheet caught on the intake guard and commands the installed grabber to pull it free. No improvised fabrication occurs in this slice. Christian: “Easy. Follow the tether.” On completion: “Flow's back.” A large sturgeon silhouette passes harmlessly, separating wildlife from the real threat. |
| 4:00–5:00 | Raise drone camera; inspect occluded surface; return drone; log fault and next task | Above, silhouettes collect against a mooring line, moving with the current. A hand catches, releases and moves downstream. Christian: “They don't swim.” Beat. “The river brings them.” He recalls the drone before touching the ceiling cluster. Physical task board updates: `NEXT: INSPECT BUOY FUEL LINE`. Christian records: “Lab 04. Christian Thomas. Air is steady. For now.” The return latch clicks; tutorial completes and the full simulation clock begins. |

Recovery and sequencing:

- If the player skips a prop, required facts are repeated once at the relevant interaction; environmental inspection stays optional.
- After 20 seconds without progress at a gate, Christian supplies one contextual hint. After another 20, the service card highlights the relevant connector or option. Assistance can be disabled.
- Invalid diagnostic selections never consume scarce components. A disconnected drone command opens `RECALL / TRY AGAIN`; it cannot kill the run during onboarding.
- The airlock exit remains visibly service-locked until the opening ends. The interlock explains why; it is not an invisible wall.
- A real-world pause freezes all game clocks and controller effects. A focus loss follows the same policy. No offline death.
- Slow reading, remapped inputs, toggle holds and reduced motion do not advance an authored failure timer. “Skip onboarding” resolves the tutorial-only repair prerequisites and enters the same post-tutorial resource state, without a resource bonus.

## Act 2 dialogue and branch logic hooks

Day 21, first inspection of the expected-supply manifest: “They said twenty-one. They knew how much this thing burns.” No radio message explains the absence. Days 22–39 introduce maintenance consequences, not an arbitrary early choice unlock.

At the Day 40 broadcast event, the radio records the message so it cannot be missed:

> MAYA: “Anyone on this channel. Trawler Mercy. We've got a child aboard. Engine runs, but we're pinned at the debris line. We have fuel. We need a clear way through. Please answer.”

The chart estimates `CONTACT: APPROX. 2 KM UPRIVER / LOCATION UNVERIFIED`. Listening is not consent to transmit. The decision screen is the physical radio/route tablet, with exact consequences visible in concise text. Cancel returns to listening while resources continue according to ordinary gameplay time.

**Choice A — Broadcast.** Prompt: `TRANSMIT LAB 04 COORDINATES? Your location cannot be made secret again. Survivors bring finite fuel and new resource demands.` On committed broadcast only:

> CHRISTIAN: “Mercy, this is Lab 04. I'm below you, downriver. I can mark a route. Keep your engine quiet.”
>
> MAYA: “Below us?”
>
> CHRISTIAN: “I'll explain when you're clear.”

The initial contact accepts or rejects the route based on the systems guard, not a dialogue dice roll. Christian's beacon guides the family; it does not make corpses swim toward them. Their arrival displaces snagged bodies and increases noise; moving infected and drifting congestion escalate through separate systems. Fuel is measured on transfer and remains finite. Social pressure surfaces through attributable incidents: a missing water canister, an unauthorized airlock request, disagreement over departure. The family is not automatically treacherous. A contaminated-supply variant requires observable evidence and a mitigation choice.

Defense-start line: “Forty-eight hours. We close what we can and watch every seal.” The 48-hour clock starts at the authoritative defense transition. The aerial-sweep resolution is established by a verified repeating evacuation bulletin before committing to hold; there is no unexplained rescue. If the player instead commits to scuttle and trawler escape, show the irreversible-loss confirmation on the station console. A future engine integration must check and consume vessel resources at the escape threshold; that guard is a design requirement, not implemented in the reference simulation. Scuttling records a station-ending archive once, distinct from a player-death tomb.

**Choice B — Go alone.** Prompt: `KEEP RADIO SILENCE? Prepare the marina route, a rated breathing-umbilical kit, and the game's 17-hour decompression cycle. You must return with fuel.` On committed route preparation:

> CHRISTIAN: “If I answer, this stops being a hiding place.”
>
> CHRISTIAN: “The marina still has tanks. I know the intake side.”

The engineering checklist names verified salvage, never instructs the player to breathe through electrical cabling. Reclaimed ROV cable provides tether/strain relief; breathing components are a separately rated fictional kit. The emergency cycle is described as `GAME DECOMPRESSION CYCLE / 17 GAME HOURS`. It accelerates presentation only: the reference simulation does not implement a second, shorter safe protocol. No real pressure schedule or treatment advice is given. The decoy lures mobile dockside infected away from the slip; a separate current forecast predicts floating clusters. Leaving with an incomplete checklist fails the gate with the missing component stated; it does not silently teleport Christian to safety.

Before departure: “One route out. One line home.” At the first diesel transfer: “Slow. Don't let the can touch the rail.” On a loud impact, a dockside infected turns and then nearby mobile infected respond; floating bodies continue their own drift. On return, only after authoritative repressurization and life-support restoration: “Lab 04. Air is steady.” A long, quiet beat. “Just me.”

Ending B records a finite secured reserve and the player’s survival achievement; it does not grant infinite fuel or erase future maintenance. Ending A records who survived and whether the station was held or scuttled. Death suppresses pending victory dialogue immediately. A future-run tomb may retain equipment silhouettes and a short last-log fragment, never transfer live resources or undo permadeath.

## Content integration and acceptance

`vertical_slice.json` is the machine-readable beat plan. Systems transition IDs, events and guards remain authoritative; the narrative adapter consumes them and emits presentation/interaction requests only. Audio/caption lines have stable IDs for localization. Every spoken line has a caption and speaker label; important positional sounds have a visual origin cue on demand. Controller speaker playback is optional and mirrored to main audio. Reduced hallucinations, input-jitter disable and full UI accessibility do not change survival math.

Acceptance: a novice can complete all five gates without external explanation; an idle player is not killed by a tutorial deadline; the drone never requires Christian to exit; the final journal exists exactly once; the branch can only be committed after the Day 40 signal; replaying an event cannot duplicate dialogue, diesel transfer or tomb creation. Narrative content must never invoke a direct resource mutation.
