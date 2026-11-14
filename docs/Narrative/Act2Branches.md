# Act 2 Branches and Endings

States follow `EAct2State`. Dialogue is spoken by Christian unless noted. No on-screen text appears anywhere; Christian reads the dive watch, the wall gauges and the mask projection.

## Act1
Days 1-20. Maintenance, scouting, quiet. Christian logs each night. Transition: Day 21 passes with no boat.

## SupplyBoatOverdue (day 21)
- Scene: Christian waits at the viewport with the radio. Nothing.
- "Supply boat's late. Boats get late. I'll check again at dusk." Later: "Dusk. Nothing. Okay. We ration."
- He cuts his meal and water plan. Isolation strain begins: phantom radio chatter on dead frequencies.
- Transition: Day 40.

## DistressIntercepted (day 40)
- The hydrophone/radio array picks up a weak broadcast. Distress text, read by a woman's voice through static:
  "This is the trawler Marguerite Anne, forty feet, two kilometers upriver of the Ambassador Bridge. Family of four aboard. We have diesel, full tanks. We can't stay here. If anyone can hear this, please answer. Repeat, family of four, full tanks, please answer."
- Christian, to the log: "Full tanks. They don't know we're here." Choice: A, broadcast the signal, or B, go alone.

## Branch A: Broadcast

### A_Broadcasting
- He transmits Lab 04's coordinates and acoustic beacon. "I'm telling a stranger where I sleep."
- Stealth is permanently compromised: the beacon cannot be unsent.
- Transition event: `FamilyArrived`.

### A_FamilyDocked
- The trawler docks over Lab 04; the family boards the airlock. Diesel is transferred.
- Social-survival scenes: sharing water (20 L a day for more people), sharing the 700 meals, paranoia, an argument over the airlock hatch, a possible mutiny, a defense of their hull when drifting hordes swarm it.
- The father: "We brought fuel. We didn't bring them." Christian: "Nobody brings them. They just come."
- Transition event: `HordeArrived`.

### A_Siege48h
- The beacon draws a river-current horde onto the station. Christian must hold bulkheads and the airlock for 48 hours.
- Lines: "Forty-eight hours. Count them out with me." "Hull's holding. It's loud, that's all."
- Outcomes: reach the 48 hours, or trigger the `Scuttle` event.

### A_EndingRescued
- At hour 48 an automated aerial sweep passes over and finds the survivors. Quiet, flat, no triumph.
- Christian: "Engines. Not ours. ...Somebody's still out there."

### A_EndingScuttled
- `Scuttle`: Christian flooding the compartments, abandoning the station, and escaping upriver on the family's trawler.
- Christian, to Lab 04: "Sorry. You were a good house." The trawler pulls away; the river closes over the station.

## Branch B: Go Alone (the RiverWalk run)

### B_Preparing
Three preparation tasks, in any order:
1. `EmergencyDeco`: bleed down nitrogen via a compressed emergency decompression protocol. "Seventeen hours is a long time. I'm squeezing it."
2. `TetherHose600m`: build a 600 m weighted makeshift air and tether hose from salvaged ROV umbilical cabling. "Six hundred meters. Don't kink it."
3. `DecoyBeacon`: program a decoy acoustic beacon to pull drifting corpses away from the marina slip. "Come on. Go listen to something else."

Transition: all three done, radio silence kept.

### B_Swim
- Christian swims 600 m to the Milliken State Park harbor and RiverWalk marina fuel dock, hidden under the living ceiling. Tethered micro-ROV sonar scouts ahead.
- Light discipline, turbidity, current eddies. "Lights off. Let the river do the work."
- Transition event: `ReachedMarina`.

### B_Siphon
- At the docks he siphons diesel from unstable, listing boats. One loud splash brings a dockside swarm.
- Lines: "Slow hands. Easy. One drop at a time." Silence, then the pump gurgles.
- Failure leads to `Dead`. Transition event: `FuelSiphoned`.

### B_Return
- The swim back along the tether, hauling fuel. He reaches Lab 04 and reseals the hatch.
- Transition event: `Repressurised`. "Pressure's back. Lights are on. I'm home."

### B_EndingAlone
- Safe, powered and completely alone. The pump thumps, the lights hold, the dome shows the ceiling drifting past.
- Final line to the log: "I'm okay. I'm alone. Both of those are true."

## Dead
- Permadeath. Hypoxia, decompression sickness, or a swarm. The screen goes dark and the save is wiped.
- Christian's last moment is quiet, no gore: a held breath, the sound of the pump fading. Cut to black.

## Event map

| From | Event | To |
|---|---|---|
| Act1 | day 21 | SupplyBoatOverdue |
| SupplyBoatOverdue | day 40 | DistressIntercepted |
| DistressIntercepted | choice A | A_Broadcasting |
| A_Broadcasting | FamilyArrived | A_FamilyDocked |
| A_FamilyDocked | HordeArrived | A_Siege48h |
| A_Siege48h | 48 hours held | A_EndingRescued |
| A_Siege48h | Scuttle | A_EndingScuttled |
| DistressIntercepted | choice B | B_Preparing |
| B_Preparing | all three tasks | B_Swim |
| B_Swim | ReachedMarina | B_Siphon |
| B_Siphon | FuelSiphoned | B_Return |
| B_Return | Repressurised | B_EndingAlone |
| any | player dies | Dead |
