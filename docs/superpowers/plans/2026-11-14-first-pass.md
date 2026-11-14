# 40 Days Under — First Pass Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A compiling UE 5.8 C++ project with the two survival clocks, the underwater character, the DualSense feedback layer, the Act 2 state machine and opening-slice logic, plus the narrative script, art production sheet and project memory files.

**Architecture:** All survival math lives in plain C++ structs with no engine object dependencies, so it is tested headless. `USurvivalClockSubsystem` advances game time in fixed one-minute steps; `USurvivalVitalsComponent` owns the models and publishes one `FVitalsSnapshot` per step. Everything else (DualSense feedback, narrative, character effects) reads the snapshot and never touches the models.

**Tech Stack:** Unreal Engine 5.8.4 (C++20), Xcode 27.2, macOS arm64, UE Automation tests, Markdown and JSON for narrative and art documents.

**Spec:** `docs/superpowers/specs/2026-11-14-first-pass-design.md` (which implements `scope.md`).

## Global Constraints

- The Unreal project lives in `unreal/` (`unreal/FortyDaysUnder.uproject`); the existing `.gitignore` already ignores `unreal/Binaries`, `unreal/Intermediate`, `unreal/Saved`.
- Module name `FortyDaysUnder`, source root `unreal/Source/FortyDaysUnder/`.
- Engine root: `UE="/Users/Shared/Epic Games/UE_5.8"`.
- Build: `"$UE/Engine/Build/BatchFiles/Mac/Build.sh" FortyDaysUnderEditor Mac Development -project="$PWD/unreal/FortyDaysUnder.uproject"` — pass means exit code 0.
- Test: `"$UE/Engine/Binaries/Mac/UnrealEditor-Cmd" "$PWD/unreal/FortyDaysUnder.uproject" -ExecCmds="Automation RunTests FortyDaysUnder; Quit" -unattended -nullrhi -nosplash -nosound -stdout` — pass means every `FortyDaysUnder.*` test logs `Result={Success}` and none logs `Result={Fail}`.
- Only one agent may run the build or test command at a time (UnrealBuildTool refuses concurrent runs). C++ tasks therefore run one after another.
- All automation tests are named `FortyDaysUnder.<Area>.<Name>` and live in `unreal/Source/FortyDaysUnder/Tests/`.
- Zero screen HUD: no `UUserWidget` added to the viewport anywhere.
- Christian Thomas is 175.26 cm; capsule half-height 87.63.
- Depth of Lab 04: 15 m of fresh water. Surface pressure 1.01325 bar; 0.0980665 bar per metre.
- No Higgsfield generation calls. No Blender files.
- Work on the local branch `first-pass`; one commit per task, no attribution lines in commit messages. Never commit to `main`, never push. Do not `git add` `scope.md`.

## Review Focus

1. **Large time jumps** (sleeping, test fast-forward): advancing 24 h in one call must give the same tissue and fuel values as 1440 one-minute steps, within 1e-6. Pinned in Task 1 (`Clock.LargeJumpMatchesSteps`).
2. **Zero or negative delta, paused clock:** state unchanged, no NaN. Pinned in Task 1 (`Clock.ZeroAndNegativeDelta`).
3. **Fuel out of range:** refuelling past capacity clamps to capacity; fuel never goes below 0. Pinned in Task 1 (`Air.FuelClamps`).
4. **No DualSense attached** (keyboard, Xbox pad, controller unplugged mid-play): feedback component does nothing and does not crash. Pinned in Task 3 (`Input.NoDeviceIsNoOp`).
5. **Bad narrative input:** missing or malformed slice JSON, a choice made before Day 40, or a second choice after the first: rejected with an error return, state unchanged. Pinned in Task 4 (`Narrative.RejectsBadInput`).

---

## Execution order

| Wave | Tasks | Mode |
| :--- | :--- | :--- |
| 1 | Task 0 (orchestrator), Task 1 (Sub-Agent 02), Task 5 (Sub-Agent 01), Task 6 (Sub-Agent 05) | parallel; only Task 1 builds |
| 2 | Task 2 (Sub-Agent 02) → Task 3 (Sub-Agent 03) → Task 4 (Sub-Agent 02, using Task 5 data) | one after another |
| 3 | Task 7 (orchestrator) | last |

---

### Task 0: Project memory files (orchestrator)

**Files:**
- Create: `PROJECT_MEMORY.md`, `CLAUDE.md`, `AGENTS.md`

- [ ] **Step 1:** Write `PROJECT_MEMORY.md` using the schema in `scope.md` section 1 verbatim, with these values filled in:
  - Christian Thomas Character Seed: reference element `4284877d-9158-4176-98aa-f7344abff977`, sheet image job `58c39f98-e7f8-4a0e-bb18-7a3c97104f78`
  - 8.2 portrait video job `673f6cc5-ad50-45f9-9975-404221147ffd`
  - Lab 04 Interior (8.3) image job `59154831-16da-4298-a683-7efa6f4ab992`
  - Lab 04 Exterior / ROV (8.4) video job `e831062e-80f2-4af1-af80-0f8ea43f6e7a`, seed `1925040229`
  - RiverWalk Marina (8.5) image job `339edeac-a2c3-4408-a4c6-d8fb66d695d5`
  - DEC-001, DEC-002 from the schema, plus DEC-003: "Unreal project lives in `unreal/`"; DEC-004: "Airlock deco gas is 40% O2 / 60% N2, linear 17 h pressure bleed"; DEC-005: "DualSense via UE generic `UInputDeviceProperty`; Sony hooks stubbed".
- [ ] **Step 2:** Write `CLAUDE.md` and `AGENTS.md`, each a short pointer: read `PROJECT_MEMORY.md` first, update it at the end of every session, plus the build and test commands from Global Constraints.
- [ ] **Step 3:** Commit: `git add PROJECT_MEMORY.md CLAUDE.md AGENTS.md docs && git commit -m "Add project memory, design and plan"`.

---

### Task 1: Project skeleton, survival models, clock, vitals (Sub-Agent 02)

**Files:**
- Create: `unreal/FortyDaysUnder.uproject`, `unreal/Config/DefaultEngine.ini`, `unreal/Config/DefaultGame.ini`, `unreal/Config/DefaultInput.ini`
- Create: `unreal/Source/FortyDaysUnder.Target.cs`, `unreal/Source/FortyDaysUnderEditor.Target.cs`
- Create: `unreal/Source/FortyDaysUnder/FortyDaysUnder.Build.cs`, `FortyDaysUnder.h`, `FortyDaysUnder.cpp`
- Create: `unreal/Source/FortyDaysUnder/Simulation/BuhlmannModel.{h,cpp}`, `AirSupplyModel.{h,cpp}`, `BodyModel.{h,cpp}`, `VitalsSnapshot.h`
- Create: `unreal/Source/FortyDaysUnder/Survival/SurvivalClockSubsystem.{h,cpp}`, `SurvivalVitalsComponent.{h,cpp}`
- Test: `unreal/Source/FortyDaysUnder/Tests/BuhlmannModelTest.cpp`, `AirSupplyModelTest.cpp`, `BodyModelTest.cpp`, `ClockTest.cpp`

**Interfaces — Produces:**

```cpp
// Simulation/BuhlmannModel.h — plain struct, no UObject
struct FBuhlmannModel
{
    static constexpr int32 NumCompartments = 16;
    static constexpr double SurfacePressureBar = 1.01325;
    static constexpr double BarPerMetreFresh = 0.0980665;
    static constexpr double WaterVapourBar = 0.0627;
    static constexpr double AirFN2 = 0.79;
    static constexpr double DecoGasFN2 = 0.60;
    static constexpr double LabDepthM = 15.0;
    static constexpr double DecoCycleHours = 17.0;

    double TissueN2Bar[NumCompartments];

    static double AmbientBarAtDepth(double DepthM);
    void ResetToSurfaceEquilibrium();
    void ResetToSaturationAt(double DepthM, double FN2);
    void Advance(double Seconds, double AmbientBar, double FN2);   // exact exponential, any step size
    double CeilingBar() const;                 // max over i of (P_i - a_i) * b_i
    bool IsSafeAt(double AmbientBar) const;    // CeilingBar() <= AmbientBar
    double DcsSeverityAt(double AmbientBar) const; // clamp((Ceiling - Ambient) / 0.6, 0, 1)
    static double DecoAmbientBar(double HoursIntoCycle); // linear 15 m -> 0 m over DecoCycleHours, clamped
};
```

ZHL-16C nitrogen constants (minutes; a in bar; b dimensionless):

| i | half-time | a | b |
| :- | :- | :- | :- |
| 1 | 5 | 1.1696 | 0.5578 |
| 2 | 8 | 1.0 | 0.6514 |
| 3 | 12.5 | 0.8618 | 0.7222 |
| 4 | 18.5 | 0.7562 | 0.7825 |
| 5 | 27 | 0.62 | 0.8126 |
| 6 | 38.3 | 0.5043 | 0.8434 |
| 7 | 54.3 | 0.441 | 0.8693 |
| 8 | 77 | 0.4 | 0.8910 |
| 9 | 109 | 0.375 | 0.9092 |
| 10 | 146 | 0.35 | 0.9222 |
| 11 | 187 | 0.3295 | 0.9319 |
| 12 | 239 | 0.3065 | 0.9403 |
| 13 | 305 | 0.2835 | 0.9477 |
| 14 | 390 | 0.261 | 0.9544 |
| 15 | 498 | 0.248 | 0.9602 |
| 16 | 635 | 0.2327 | 0.9653 |

Tissue update: `P += ((Ambient - WaterVapourBar) * FN2 - P) * (1 - 2^(-minutes / halfTime))`.

```cpp
// Simulation/AirSupplyModel.h
struct FAirSupplyModel
{
    static constexpr double TankCapacityL = 2000.0;
    static constexpr double BurnLPerHour = 2.0;        // dry at day 41.67 from full
    static constexpr double SputterFraction = 0.15;
    static constexpr double Co2RisePctPerHour = 0.5;   // pump dead
    static constexpr double O2FallPctPerHour = 0.5;    // pump dead
    static constexpr double Co2ScrubPctPerHour = 2.0;  // pump running, down to 0.04
    static constexpr double O2RecoverPctPerHour = 2.4; // pump running, up to 20.9

    double FuelL = TankCapacityL;
    double Co2Pct = 0.04;
    double O2Pct = 20.9;

    void Advance(double Seconds);
    double AddFuel(double Litres);          // returns litres accepted; clamps to capacity; negative input accepted as 0
    double FuelFraction() const;
    bool IsPumpRunning() const;             // FuelL > 0
    bool IsSputtering() const;              // running and FuelFraction() < SputterFraction
    EAirStage Stage() const;
};
enum class EAirStage : uint8 { Nominal, Impaired /*CO2>=3*/, Hypoxic /*CO2>=5*/, Critical /*CO2>=8*/, Dead /*CO2>=10 or O2<=10*/ };
```

`Advance` with a step that crosses fuel-zero must split at the crossing so the result is step-size independent.

```cpp
// Simulation/BodyModel.h
struct FBodyModel
{
    static constexpr double WaterNeedLPerDay = 3.0;
    static constexpr double CalorieNeedPerDay = 2400.0;
    static constexpr double MealKcal = 800.0;
    static constexpr double MealWaterCostL = 0.25;
    static constexpr int32  StartingMeals = 700;
    static constexpr double DesalLPerDay = 20.0;
    static constexpr double IsolationPerDay = 0.025;   // reaches 1.0 on day 40

    double Health = 100.0;         // 0..100
    double Stamina = 100.0;        // 0..100
    double HydrationL = 3.0;       // body reserve, 0..3
    double CaloriesKcal = 2400.0;  // body reserve, 0..2400
    double Isolation = 0.0;        // 0..1
    double PotableWaterL = 20.0;   // station tank, 0..200
    int32  MealsLeft = StartingMeals;

    void Advance(double Seconds, bool bDesalPowered, EAirStage Air, double DcsSeverity);
    bool EatMeal();     // false if none left
    bool Drink(double Litres); // false if tank lacks it
    bool IsDead() const;
};
```

Health drain per second: dehydrated (HydrationL == 0) 100/(3×86400); starving 100/(21×86400); `Critical` air 100/3600; DCS `0.5 × severity`; `Dead` air sets Health to 0. Stamina regen is halved at `Impaired`, zero at `Hypoxic` and above.

```cpp
// Simulation/VitalsSnapshot.h — the state contract every other system reads
USTRUCT(BlueprintType)
struct FVitalsSnapshot
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) double GameSeconds = 0;
    UPROPERTY(BlueprintReadOnly) int32  Day = 1;                 // 1-based
    UPROPERTY(BlueprintReadOnly) double FuelFraction = 1;
    UPROPERTY(BlueprintReadOnly) bool   bPumpRunning = true;
    UPROPERTY(BlueprintReadOnly) bool   bPumpSputtering = false;
    UPROPERTY(BlueprintReadOnly) double Co2Pct = 0.04;
    UPROPERTY(BlueprintReadOnly) double O2Pct = 20.9;
    UPROPERTY(BlueprintReadOnly) EAirStage AirStage = EAirStage::Nominal;
    UPROPERTY(BlueprintReadOnly) double AmbientBar = 0;
    UPROPERTY(BlueprintReadOnly) double CeilingBar = 0;
    UPROPERTY(BlueprintReadOnly) double DcsSeverity = 0;         // 0..1
    UPROPERTY(BlueprintReadOnly) bool   bInDecoCycle = false;
    UPROPERTY(BlueprintReadOnly) double DecoHoursRemaining = 0;
    UPROPERTY(BlueprintReadOnly) double Health = 100;
    UPROPERTY(BlueprintReadOnly) double Stamina = 100;
    UPROPERTY(BlueprintReadOnly) double HydrationL = 3;
    UPROPERTY(BlueprintReadOnly) double CaloriesKcal = 2400;
    UPROPERTY(BlueprintReadOnly) double Isolation = 0;
    UPROPERTY(BlueprintReadOnly) bool   bDead = false;
};
```

`EAirStage` is a `UENUM(BlueprintType)` declared in `VitalsSnapshot.h`.

```cpp
// Survival/SurvivalClockSubsystem.h
UCLASS() class USurvivalClockSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    static constexpr double StepSeconds = 60.0;
    UPROPERTY(EditAnywhere) double TimeScale = 60.0;   // game seconds per real second; 0 pauses
    double GetGameSeconds() const;
    int32  GetDay() const;                              // 1 + floor(GameSeconds / 86400)
    void   AdvanceBy(double GameSeconds);               // runs whole StepSeconds steps, carries remainder
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSimStep, double /*StepSeconds*/);
    FOnSimStep OnSimStep;
};

// Survival/SurvivalVitalsComponent.h
UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent))
class USurvivalVitalsComponent : public UActorComponent
{
    UFUNCTION(BlueprintCallable) const FVitalsSnapshot& GetSnapshot() const;
    UFUNCTION(BlueprintCallable) void SetDepthMetres(double DepthM);   // sets ambient pressure, breathing air
    UFUNCTION(BlueprintCallable) void BeginDecoCycle();                // airlock: ambient follows DecoAmbientBar, gas DecoGasFN2
    UFUNCTION(BlueprintCallable) void AbortDecoCycle();
    UFUNCTION(BlueprintCallable) double AddFuel(double Litres);
    UFUNCTION(BlueprintCallable) bool EatMeal();
    UFUNCTION(BlueprintCallable) bool Drink(double Litres);
    void SimStep(double Seconds);       // public so tests drive it without a world
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVitalsChanged, const FVitalsSnapshot&, Snapshot);
    UPROPERTY(BlueprintAssignable) FOnVitalsChanged OnVitalsChanged;
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDied);
    UPROPERTY(BlueprintAssignable) FOnDied OnDied;   // fires once
};
```

The component starts saturated at 15 m on air (`ResetToSaturationAt(15, 0.79)`), binds to `OnSimStep` in `BeginPlay`. `SimStep` splits any input into 60-second sub-steps plus a remainder, so one large call and many small calls give identical results; zero or negative input does nothing.

`Build.cs` public dependencies: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `ApplicationCore`, `Json`, `JsonUtilities`. `.uproject`: engine association `5.8`, one Runtime module, plugins `EnhancedInput`, `CableComponent`, `GameplayAbilities` enabled.

- [ ] **Step 1: Create the project skeleton** (uproject, targets, Build.cs, module files, configs) and run the Build command. Expected: exit 0. If Xcode 27.2 is rejected by UE 5.8, stop and report the exact error instead of working around it.

- [ ] **Step 2: Write the failing model tests**

```cpp
// FortyDaysUnder.Deco.SaturatedDiverCannotSurface
FBuhlmannModel M; M.ResetToSaturationAt(15.0, 0.79);
TestTrue("ceiling ~1.622 bar", FMath::IsNearlyEqual(M.CeilingBar(), 1.622, 0.002));
TestFalse("unsafe at surface", M.IsSafeAt(FBuhlmannModel::SurfacePressureBar));
TestTrue("safe at depth", M.IsSafeAt(FBuhlmannModel::AmbientBarAtDepth(15.0)));

// FortyDaysUnder.Deco.SeventeenHourCycleClears
// one-minute steps along DecoAmbientBar with DecoGasFN2; assert IsSafeAt(DecoAmbientBar(t)) at every step
// after 16 h: CeilingBar() > 1.01325 (≈1.028).  after 17 h: CeilingBar() <= 1.01325 (≈0.985).

// FortyDaysUnder.Deco.OnGassingFromSurface
// from surface equilibrium, 2 h at 15 m on air: ceiling ≈ 1.076 (±0.005), unsafe to surface;
// 1 h at 15 m: ceiling ≈ 0.857, safe to surface.

// FortyDaysUnder.Deco.SeverityRange
// saturated model: DcsSeverityAt(1.01325) ≈ (1.622-1.01325)/0.6 clamped to 1.0; DcsSeverityAt(2.48) == 0

// FortyDaysUnder.Air.FortyDayTank
// full tank, Advance(40 days): FuelL == 80 (±0.01), sputtering, pump running, Co2Pct == 0.04
// Advance to day 41.667: FuelL == 0, pump not running
// +6 h: Co2Pct ≈ 3.04 → Impaired.  +10 h: ≈ 5.04 → Hypoxic.  +16 h: ≈ 8.04 → Critical.  +20 h: Dead.

// FortyDaysUnder.Air.FuelClamps
// AddFuel(5000) on 100 L returns 1900, FuelL == 2000; AddFuel(-5) returns 0; fuel never < 0 after long Advance

// FortyDaysUnder.Air.RefuelRecovers
// from Hypoxic, AddFuel(500), Advance(3 h): Stage == Nominal, Co2Pct == 0.04, O2Pct == 20.9

// FortyDaysUnder.Body.Needs
// 1 day, desal on, no eating/drinking: HydrationL == 0, CaloriesKcal == 0, PotableWaterL == 40, Isolation == 0.025
// EatMeal: MealsLeft 699, CaloriesKcal +800 (cap 2400), PotableWaterL −0.25
// 3 more days dehydrated: IsDead()

// FortyDaysUnder.Body.DcsDrain
// Advance(200 s, air Nominal, DcsSeverity 1.0): Health == 0
```

- [ ] **Step 3: Write the failing clock and component tests**

```cpp
// FortyDaysUnder.Clock.LargeJumpMatchesSteps
// component A: SimStep(60) × 1440.  component B: SimStep(86400) once.
// every double in the two snapshots equal within 1e-6.

// FortyDaysUnder.Clock.ZeroAndNegativeDelta
// SimStep(0) and SimStep(-10): snapshot unchanged, no NaN in any field.
// subsystem with TimeScale 0: Tick(1.0) fires no OnSimStep.

// FortyDaysUnder.Clock.FixedSteps
// AdvanceBy(150): OnSimStep fired twice with 60; AdvanceBy(30): fires once more (remainder carried). GetDay()==1; AdvanceBy(86400) → 2.

// FortyDaysUnder.Vitals.DecoCycleThroughComponent
// BeginDecoCycle; SimStep 17 h in minutes: bInDecoCycle false at end, DcsSeverity == 0 throughout, DecoHoursRemaining == 0, AmbientBar == 1.01325.
// second component: BeginDecoCycle, 8 h, AbortDecoCycle, SetDepthMetres(0): DcsSeverity > 0, Health falling.

// FortyDaysUnder.Vitals.DeathFiresOnce
// drive to death; OnDied broadcast count == 1; bDead stays true.
```

Tests build components with `NewObject<USurvivalVitalsComponent>()` and call `SimStep` directly; no world needed.

- [ ] **Step 4:** Run Build then Test. Expected: build fails (types missing).
- [ ] **Step 5:** Implement the three models, the snapshot, the subsystem and the component to the signatures above.
- [ ] **Step 6:** Run Build then Test. Expected: all `FortyDaysUnder.Deco.*`, `Air.*`, `Body.*`, `Clock.*`, `Vitals.*` log `Result={Success}`.
- [ ] **Step 7:** Commit: `git add unreal && git commit -m "Add UE project, survival models, clock and vitals component"`.

---

### Task 2: Underwater character and movement (Sub-Agent 02)

**Files:**
- Create: `unreal/Source/FortyDaysUnder/Character/UnderwaterCharacter.{h,cpp}`, `UnderwaterMovementComponent.{h,cpp}`
- Create: `unreal/Source/FortyDaysUnder/Game/FortyDaysGameMode.{h,cpp}`
- Modify: `unreal/Config/DefaultEngine.ini` (set `GlobalDefaultGameMode=/Script/FortyDaysUnder.FortyDaysGameMode`)
- Test: `unreal/Source/FortyDaysUnder/Tests/CharacterTest.cpp`

**Interfaces:**
- Consumes: `USurvivalVitalsComponent`, `FVitalsSnapshot` (Task 1).
- Produces:

```cpp
UCLASS() class UUnderwaterMovementComponent : public UCharacterMovementComponent
{
    UPROPERTY(EditAnywhere) FVector WaterCurrentCmPerSec = FVector::ZeroVector;
    UPROPERTY(EditAnywhere) float NetBuoyancy = 0.0f;         // -1 sinks, +1 floats, 0 neutral
    UPROPERTY(EditAnywhere) float BaseSwimSpeed = 180.0f;     // cm/s
    float GetEffectiveSwimSpeed(float Stamina, EAirStage Air) const; // Base × lerp(0.4,1,Stamina/100); ×0.5 at Hypoxic or worse
    virtual void PhysSwimming(float DeltaTime, int32 Iterations) override; // adds current and buoyancy
};

UCLASS() class AUnderwaterCharacter : public ACharacter
{
    AUnderwaterCharacter(const FObjectInitializer&);          // swaps in UUnderwaterMovementComponent
    UPROPERTY(VisibleAnywhere) TObjectPtr<USurvivalVitalsComponent> Vitals;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> FirstPersonCamera;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> ThirdPersonBoom;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> ThirdPersonCamera;
    UFUNCTION(BlueprintCallable) void SetCameraBlend(float Alpha);   // 0 first person, 1 third person
    UFUNCTION(BlueprintCallable) float GetInputLatencySeconds() const; // 0.25 × DcsSeverity
    UFUNCTION(BlueprintCallable) float GetAudioLowPassHz() const;      // lerp(20000, 800, DcsSeverity)
    UFUNCTION(BlueprintCallable) float GetHypoxiaVignette() const;     // 0 Nominal, 0.3 Impaired, 0.6 Hypoxic, 1 Critical
    UFUNCTION(BlueprintCallable) double GetDepthMetres() const;        // (WaterSurfaceZ − ActorZ)/100, min 0
    UPROPERTY(EditAnywhere) double WaterSurfaceZ = 0.0;
    UPROPERTY(EditAnywhere) bool bInsidePressurisedLab = true;         // when true, depth sent to Vitals is 15
};
```

Input: `UInputAction` properties `MoveAction`, `LookAction`, `AscendAction`, `ToggleCameraAction` bound through `UEnhancedInputComponent`; move and look inputs are delayed by `GetInputLatencySeconds()` using a small timestamped queue. Capsule half-height 87.63, radius 34. On tick, when not inside the lab, the character calls `Vitals->SetDepthMetres(GetDepthMetres())`.

- [ ] **Step 1: Write the failing tests**

```cpp
// FortyDaysUnder.Character.Dimensions — CDO capsule half-height == 87.63; movement component class is UUnderwaterMovementComponent
// FortyDaysUnder.Character.SwimSpeed — GetEffectiveSwimSpeed(100, Nominal)==180; (0, Nominal)==72; (100, Hypoxic)==90
// FortyDaysUnder.Character.DcsEffects — with a snapshot of DcsSeverity 1: latency 0.25, low-pass 800; severity 0: 0 and 20000
// FortyDaysUnder.Character.Vignette — 0 / 0.3 / 0.6 / 1 for the four living air stages
```

The effect getters must be computable from a snapshot so tests need no world: implement them as static helpers `static float LatencyFor(const FVitalsSnapshot&)` etc., with the member functions forwarding.

- [ ] **Step 2:** Build then Test; expected: build fails.
- [ ] **Step 3:** Implement to the signatures above.
- [ ] **Step 4:** Build then Test; expected: all `FortyDaysUnder.*` pass.
- [ ] **Step 5:** Commit: `git commit -am` is not enough for new files — `git add unreal && git commit -m "Add underwater character, movement and game mode"`.

---

### Task 3: DualSense feedback layer (Sub-Agent 03)

**Files:**
- Create: `unreal/Source/FortyDaysUnder/Input/DualSenseFeedbackProfile.{h,cpp}`, `DualSenseFeedbackComponent.{h,cpp}`, `SonyPadHooks.h`
- Create: `Docs/Hardware/DualSenseMapping.md`
- Test: `unreal/Source/FortyDaysUnder/Tests/DualSenseTest.cpp`

**Interfaces:**
- Consumes: `FVitalsSnapshot`, `EAirStage`, `USurvivalVitalsComponent::OnVitalsChanged` (Task 1).
- Produces:

```cpp
UENUM() enum class ETriggerAction : uint8 { None, SpeargunLoad, SpeargunFire, ValveCrank };

struct FTriggerFeedback { int32 StartPosition; int32 EndPosition; int32 StartStrength; int32 EndStrength; // 0..8
                          bool bVibrate; int32 VibrationFrequencyHz; int32 VibrationAmplitude; };
struct FPumpHaptic { float PulseHz; float Amplitude; float Jitter; };   // Jitter 0 = steady

struct FDualSenseFeedbackProfile   // pure functions, no engine state
{
    static FLinearColor LightbarColor(const FVitalsSnapshot&, float HordeProximity01, double RealTimeSeconds);
    static FTriggerFeedback TriggerFor(ETriggerAction, float Progress01, const FVitalsSnapshot&);
    static FPumpHaptic PumpHaptic(const FVitalsSnapshot&);
};

UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class UDualSenseFeedbackComponent : public UActorComponent
{
    UFUNCTION(BlueprintCallable) void SetTriggerAction(ETriggerAction Action, float Progress01);
    UFUNCTION(BlueprintCallable) void SetHordeProximity(float Proximity01);
    UPROPERTY(EditAnywhere) TObjectPtr<UHapticFeedbackEffect_Base> PumpThumpEffect;   // asset assigned later
    bool HasSupportedDevice() const;
};
```

Pinned values:
- Lightbar: teal `(0.0, 0.5, 0.5)`, amber `(1.0, 0.6, 0.0)`, crimson `(0.86, 0.08, 0.24)`. Warning level `W = max(clamp(1 − FuelFraction/0.15, 0, 1), clamp((CeilingBar − AmbientBar + 0.3)/0.3, 0, 1))`: the nitrogen term rises as the tissue ceiling comes within 0.3 bar of ambient, so living saturated at 15 m stays teal. Colour = lerp(teal, amber, W). If `AirStage >= Hypoxic` or `HordeProximity01 >= 0.7`: crimson × `(0.6 + 0.4 × sin(2π × 1.5 Hz × t))`.
- Triggers: `SpeargunLoad` resistance ramps `StartStrength 2 → EndStrength 8` across positions 2..8 scaled by progress; `SpeargunFire` is a hard wall at position 6 strength 8 then release; `ValveCrank` vibrates at 12 Hz amplitude 5. Fatigue stiffness added to every action: `+round(3 × (1 − Stamina/100))` strength, capped at 8. When `DcsSeverity > 0` add chugging vibration at `8 Hz`, amplitude `round(8 × DcsSeverity)`.
- Pump haptic: steady `1.6 Hz`, amplitude `0.35` when running; when sputtering, jitter `1 − FuelFraction/0.15` and amplitude `0.5`; pump dead → amplitude `0`.

The component applies results through `UInputDeviceSubsystem` with `UInputDeviceTriggerResistanceProperty`, `UInputDeviceTriggerVibrationProperty` and `UInputDeviceLightColorProperty`. `SonyPadHooks.h` declares `namespace SonyPad { void PlaySpeakerCue(FName Cue); }` with an empty inline body outside `PLATFORM_PS5`, and a comment that the real body needs the Sony SDK.

`Docs/Hardware/DualSenseMapping.md`: a table of every gameplay state → trigger, haptic, speaker cue and lightbar response, using the values above, plus the speaker cue names (`RadioStatic`, `DecoAlarm`, `RegulatorBreath`, `Hallucination`).

- [ ] **Step 1: Write the failing tests**

```cpp
// FortyDaysUnder.Input.LightbarRamp
// saturated in the lab (CeilingBar 1.622, AmbientBar 2.484), fuel 1.0 → teal
// FuelFraction 0.075 → halfway teal/amber; 0.0 → amber
// CeilingBar 1.35, AmbientBar 1.5, fuel full → exactly halfway teal/amber
// AirStage Hypoxic → red channel dominant, and value at t=0 differs from t=1/6 s (pulsing)
// HordeProximity 0.8 with nominal vitals → crimson family

// FortyDaysUnder.Input.TriggerCurves
// SpeargunLoad progress 0 → strength 2; progress 1 → 8; Stamina 0 adds 3 (capped 8)
// DcsSeverity 0.5 → bVibrate true, 8 Hz, amplitude 4
// ETriggerAction::None with full stamina, no DCS → all strengths 0, bVibrate false

// FortyDaysUnder.Input.PumpHaptic
// FuelFraction 1.0 → 1.6 Hz, 0.35, jitter 0;  0.075 → jitter 0.5, amplitude 0.5;  pump dead → amplitude 0

// FortyDaysUnder.Input.NoDeviceIsNoOp
// NewObject component with no world/owner: SetTriggerAction, SetHordeProximity and the vitals handler run without crash; HasSupportedDevice() == false
```

- [ ] **Step 2:** Build then Test; expected: build fails.
- [ ] **Step 3:** Implement profile, component, stub header and the mapping document.
- [ ] **Step 4:** Build then Test; expected: all `FortyDaysUnder.*` pass.
- [ ] **Step 5:** Commit: `git add unreal Docs/Hardware && git commit -m "Add DualSense feedback layer and mapping doc"`.

---

### Task 4: Act 2 state machine and opening-slice director (Sub-Agent 02, with Task 5 data)

**Files:**
- Create: `unreal/Source/FortyDaysUnder/Narrative/Act2StateMachine.{h,cpp}`, `SliceDirector.{h,cpp}`
- Test: `unreal/Source/FortyDaysUnder/Tests/NarrativeTest.cpp`

**Interfaces:**
- Consumes: `FVitalsSnapshot` (Task 1); `unreal/Content/Data/Narrative/opening_slice.json` (Task 5).
- Produces:

```cpp
UENUM(BlueprintType) enum class EAct2State : uint8 {
    Act1, SupplyBoatOverdue /*day 21*/, DistressIntercepted /*day 40*/,
    A_Broadcasting, A_FamilyDocked, A_Siege48h, A_EndingRescued, A_EndingScuttled,
    B_Preparing, B_Swim, B_Siphon, B_Return, B_EndingAlone,
    Dead };

UENUM(BlueprintType) enum class EPrepTask : uint8 { EmergencyDeco, TetherHose600m, DecoyBeacon };

UCLASS(BlueprintType) class UAct2StateMachine : public UObject
{
    UFUNCTION(BlueprintCallable) EAct2State GetState() const;
    UFUNCTION(BlueprintCallable) void OnDayChanged(int32 Day);          // Act1→SupplyBoatOverdue at 21, →DistressIntercepted at 40
    UFUNCTION(BlueprintCallable) bool ChooseBroadcast();                // only from DistressIntercepted
    UFUNCTION(BlueprintCallable) bool ChooseGoAlone();                  // only from DistressIntercepted
    UFUNCTION(BlueprintCallable) bool CompletePrepTask(EPrepTask Task); // only in B_Preparing; all three → B_Swim
    UFUNCTION(BlueprintCallable) bool Advance(FName Event);             // see table
    UFUNCTION(BlueprintCallable) void NotifyDeath();                    // any state → Dead
    UFUNCTION(BlueprintCallable) double SiegeHoursRemaining() const;    // 48 at entry to A_Siege48h
    void TickSiege(double GameSeconds);                                 // reaches 0 → A_EndingRescued
};
```

`Advance` events: `FamilyArrived` (A_Broadcasting→A_FamilyDocked), `HordeArrived` (A_FamilyDocked→A_Siege48h), `Scuttle` (A_Siege48h→A_EndingScuttled), `ReachedMarina` (B_Swim→B_Siphon), `FuelSiphoned` (B_Siphon→B_Return), `Repressurised` (B_Return→B_EndingAlone). Any other event/state pair returns false and changes nothing.

```cpp
USTRUCT() struct FSliceBeat { FName Id; FString Speaker; FString Line; FName CompleteOnEvent; double TimeoutSeconds; };

UCLASS(BlueprintType) class USliceDirector : public UObject
{
    bool LoadFromJsonString(const FString& Json, FString& OutError);   // false + message on malformed or empty
    bool LoadFromFile(const FString& Path, FString& OutError);
    UFUNCTION(BlueprintCallable) int32 NumBeats() const;
    UFUNCTION(BlueprintCallable) FName CurrentBeatId() const;           // NAME_None before load or after end
    UFUNCTION(BlueprintCallable) bool NotifyEvent(FName Event);         // advances if it matches current beat
    void Tick(double RealSeconds);                                      // advances on timeout
    UFUNCTION(BlueprintCallable) bool IsFinished() const;
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeat, FName, BeatId);
    UPROPERTY(BlueprintAssignable) FOnBeat OnBeatStarted;
};
```

JSON shape (Task 5 writes it): `{"beats":[{"id":"...","speaker":"...","line":"...","completeOnEvent":"...","timeoutSeconds":30}]}`.

- [ ] **Step 1: Write the failing tests**

```cpp
// FortyDaysUnder.Narrative.DayTriggers — OnDayChanged(20) stays Act1; 21 → SupplyBoatOverdue; 40 → DistressIntercepted
// FortyDaysUnder.Narrative.BranchA — ChooseBroadcast → A_Broadcasting; FamilyArrived; HordeArrived → A_Siege48h, SiegeHoursRemaining()==48;
//     TickSiege(48 h) → A_EndingRescued.  Separate run: Scuttle during siege → A_EndingScuttled
// FortyDaysUnder.Narrative.BranchB — ChooseGoAlone → B_Preparing; two prep tasks → still B_Preparing; third → B_Swim;
//     ReachedMarina, FuelSiphoned, Repressurised → B_EndingAlone
// FortyDaysUnder.Narrative.RejectsBadInput — ChooseBroadcast on day 39 returns false, state Act1;
//     after ChooseBroadcast, ChooseGoAlone returns false; Advance("FuelSiphoned") in A_Broadcasting false;
//     CompletePrepTask twice with same task: second returns false;
//     LoadFromJsonString("{not json") false with non-empty error; LoadFromJsonString("{\"beats\":[]}") false; missing file false
// FortyDaysUnder.Narrative.SliceFile — LoadFromFile(project Content/Data/Narrative/opening_slice.json) true;
//     sum of timeoutSeconds == 300; NotifyEvent with each beat's completeOnEvent in order reaches IsFinished();
//     a wrong event returns false and does not advance; Tick past a timeout advances
// FortyDaysUnder.Narrative.DeathFromAnyState — NotifyDeath from B_Swim → Dead; further Advance returns false
```

- [ ] **Step 2:** Build then Test; expected: build fails.
- [ ] **Step 3:** Implement both classes.
- [ ] **Step 4:** Build then Test; expected: all `FortyDaysUnder.*` pass.
- [ ] **Step 5:** Commit: `git add unreal && git commit -m "Add Act 2 state machine and opening-slice director"`.

---

### Task 5: Narrative script and slice data (Sub-Agent 01)

**Files:**
- Create: `Docs/Narrative/WorldBible.md`, `Docs/Narrative/OpeningSlice.md`, `Docs/Narrative/Act2Branches.md`
- Create: `unreal/Content/Data/Narrative/opening_slice.json`

**Interfaces — Produces:** `opening_slice.json` in the shape given in Task 4, with 8–12 beats whose `timeoutSeconds` sum to exactly 300, every `id` unique, every `completeOnEvent` a non-empty name.

Content requirements:
- `WorldBible.md`: Lab 04 history, Christian's background, the outbreak timeline, Detroit riverfront landmarks used (Belle Isle, Renaissance Center, Milliken State Park harbor, RiverWalk, Ambassador Bridge, Windsor skyline), the "living ceiling", and ten environmental-storytelling props for Lab 04. Facts must match `scope.md` sections 2, 3 and 5.
- `OpeningSlice.md`: the five-minute Day 1 slice inside Lab 04 as a beat sheet matching the JSON one-to-one: wake to the pump thump, read the wall manifold gauges and dive watch (no screen HUD is ever described), clear a telemetry fault, crank a bulkhead valve, first look up through the dome at the living ceiling. Christian's lines are spoken to himself or his log recorder. Use these numbers where a number is spoken: 2000 L tank, 2 L per hour, 20 L of water a day, 700 meals, 17 hours to decompress.
- `Act2Branches.md`: scenes and dialogue for each state in `EAct2State` (names in Task 4), including the Day 40 distress broadcast text, the three preparation tasks, both Ending 1 outcomes and Ending 2.

- [ ] **Step 1:** Write the three documents and the JSON.
- [ ] **Step 2:** Verify: `python3 -c "import json;d=json.load(open('unreal/Content/Data/Narrative/opening_slice.json'));b=d['beats'];assert 8<=len(b)<=12;assert sum(x['timeoutSeconds'] for x in b)==300;assert len({x['id'] for x in b})==len(b);assert all(x['completeOnEvent'] for x in b);print('ok')"`. Expected: `ok`.
- [ ] **Step 3:** Do not commit; the orchestrator commits `Docs/Narrative` and the JSON after Task 1's commit lands, with message "Add narrative bible, opening slice and Act 2 script".

---

### Task 6: Blender-to-UE5 art production sheet (Sub-Agent 05)

**Files:**
- Create: `Docs/Art/ProductionSheet.md`, `Docs/Art/FbxExportChecklist.md`

Content requirements, all from `scope.md` section 9 and step 4 of section 10:
- One section per asset: `CHR_Christian_01` (refs 8.1, 8.2), `ENV_Lab04_Int` (8.3), `PROP_ROV_Scout` (8.4), `ENV_RiverWalk_Dock` (8.5), each naming its Higgsfield job ID from `PROJECT_MEMORY.md`.
- Polycount budgets: Christian LOD0 45k–65k tris with LOD1–3 budgets; per-module budgets for Lab 04 kit pieces, the ROV, and dock modules.
- Texel density: 10.24 px/cm hero, 5.12 px/cm environment; texture sets at 4K with ORM packing (R occlusion, G roughness, B metallic), DirectX normals (−Y green), albedo.
- Scale and axes: metric, 1 unit = 1 m, +X forward, +Z up; Christian exactly 175.26 cm.
- Skeleton: the UE5 Manny/Quinn bone hierarchy listed by name, including twist and IK bones; the 52 ARKit shape key names; note any bone name the author is unsure of as "verify against `SK_Mannequin` in engine" rather than guessing.
- Hair: 360-wave cap with anisotropic tangent map plus hair-card fringe.
- Nanite rules: opaque hull meshes Nanite, no manual LODs; acrylic, glass and water on non-Nanite translucent Substrate materials.
- Collision: `UCX_<MeshName>` convex hulls authored in Blender.
- Naming and folder conventions for `.blend`, `.fbx` and `unreal/Content/` destinations.
- `FbxExportChecklist.md`: tick-list with Smoothing: Face, Apply Transform on, scale, axis, leaf bones off, and a per-asset import-settings table for UE.

- [ ] **Step 1:** Write both documents.
- [ ] **Step 2:** Verify: `grep -c -E "10\.24|5\.12|175\.26|UCX_|45k|65k" Docs/Art/ProductionSheet.md` prints 6 or more.
- [ ] **Step 3:** Do not commit; the orchestrator commits `Docs/Art` with message "Add art production sheet and FBX checklist".

---

### Task 7: Integration check and memory write-back (orchestrator)

- [ ] **Step 1:** Run Build then Test from a clean state (`rm -rf unreal/Intermediate unreal/Binaries` first). Expected: exit 0 and every `FortyDaysUnder.*` test `Result={Success}`.
- [ ] **Step 2:** `grep -rn "AddToViewport\|UUserWidget" unreal/Source` — expected: no matches.
- [ ] **Step 3:** Update `PROJECT_MEMORY.md`: C++ module registry (each class, `[ACTIVE]`), test count, completed list, open blockers (Sony hooks stubbed; no level; no Blender assets; haptic `.wav` assets not authored), and the exact next task ("Build graybox Lab 04 level and place `AUnderwaterCharacter`; then `AROVCompanionPawn`").
- [ ] **Step 4:** Commit: `git add -A && git commit -m "Update project memory after first pass"`.
- [ ] **Step 5:** Report to the owner: what was built, test results as observed, what was not verified (feel, controller on real hardware).
