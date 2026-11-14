# 40 Days Under

Read `PROJECT_MEMORY.md` first. It is the single source of truth for decisions, asset IDs and the
next queued task. Entries marked `[SUPERSEDED by ...]` are history only. Update it at the end of
every session with verified decisions, finished classes, blockers and the exact next task.

Requirements live in `scope.md`. The Unreal project lives in `unreal/`.

## Build and test

```sh
UE="/Users/Shared/Epic Games/UE_5.8"
"$UE/Engine/Build/BatchFiles/Mac/Build.sh" FortyDaysUnderEditor Mac Development -project="$PWD/unreal/FortyDaysUnder.uproject"
"$UE/Engine/Binaries/Mac/UnrealEditor-Cmd" "$PWD/unreal/FortyDaysUnder.uproject" -ExecCmds="Automation RunTests FortyDaysUnder; Quit" -unattended -nullrhi -nosplash -nosound -stdout
```

Only one build or test run at a time; UnrealBuildTool refuses concurrent runs.

## Rules

- No screen HUD. All telemetry is shown on in-world geometry.
- Survival math stays in the plain structs under `unreal/Source/FortyDaysUnder/Simulation/`;
  other systems read `FVitalsSnapshot` only.
- Automation tests are named `FortyDaysUnder.<Area>.<Name>` and live in
  `unreal/Source/FortyDaysUnder/Tests/`.
