# DualSense Mapping

How survival state drives the PS5 DualSense. Values come from `DualSenseFeedbackProfile` (pure functions, unit tested) and are applied by `UDualSenseFeedbackComponent`. With no supported device every call is a silent no-op. Speaker cues are PS5-only and go through `SonyPad::PlaySpeakerCue`, which needs the Sony SDK and is an empty stub elsewhere.

Trigger positions and strengths are 0..8 on the right trigger. Lightbar colours are linear RGB.

## Gameplay state to feedback

| Gameplay state | Adaptive trigger | Haptics | Speaker cue | Lightbar |
|---|---|---|---|---|
| Idle, nominal vitals, fuel and nitrogen safe | None: positions 0, strengths 0 | None | None | Teal (0.0, 0.5, 0.5) |
| Loading the speargun (progress 0 to 1) | Resistance across positions 2..8, strength 2 at the start rising to round(lerp(2, 8, progress)) at the end | None | None | Per vitals |
| Speargun armed to fire | Hard wall at position 6, strength 8 | None | None | Per vitals |
| Speargun released (progress >= 1) | Released: strengths 0 | None | None | Per vitals |
| Cranking a valve | Positions 0..8, strength 4, vibration 12 Hz, amplitude 5 | None | None | Per vitals |
| Fatigue (Stamina below 100) | Adds round(3 x (1 - Stamina/100)) to start and end strength of every action including None, capped at 8 | None | None | Per vitals |
| Decompression sickness (DcsSeverity > 0) | Overrides any action vibration: 8 Hz, amplitude round(8 x DcsSeverity) | None | `DecoAlarm` when it first appears | Per vitals |
| Pump running steadily | None | 1.6 Hz thump, amplitude 0.35, jitter 0 | `RegulatorBreath` | Per vitals |
| Pump sputtering (fuel under 15%) | None | 1.6 Hz, amplitude 0.5, jitter 1 - Fuel/0.15 | `RegulatorBreath`, strained | Per vitals |
| Pump dead | None | Amplitude 0 | None | Per vitals |
| Fuel under 15% | None | See pump rows | None | Blend toward amber (1.0, 0.6, 0.0) |
| Tissue ceiling within 0.3 bar of ambient | None | None | `DecoAlarm` | Blend toward amber |
| Air stage Impaired | None | None | `RadioStatic` | Per vitals |
| Air stage Hypoxic or worse | None | None | `Hallucination` | Crimson pulse, overrides blend |
| Horde proximity >= 0.7 | None | None | None | Crimson pulse, overrides blend |
| Distress broadcast | None | None | `RadioStatic` | Per vitals |

## Lightbar rules

- Warning level `W = max(clamp(1 - FuelFraction/0.15, 0, 1), clamp((CeilingBar - AmbientBar + 0.3)/0.3, 0, 1))`.
- Colour = lerp(teal, amber, `W`). The nitrogen term rises once the tissue ceiling is within 0.3 bar of ambient, so a diver living saturated at 15 m stays teal.
- If `AirStage >= Hypoxic` or `HordeProximity01 >= 0.7`: crimson (0.86, 0.08, 0.24) times `0.6 + 0.4 x sin(2 pi x 1.5 Hz x t)`, alpha 1. This replaces the teal/amber blend entirely.

## Speaker cues

| Cue name | Meaning |
|---|---|
| `RadioStatic` | Distress broadcast and impaired air |
| `DecoAlarm` | Decompression danger |
| `RegulatorBreath` | Breathing through the pump |
| `Hallucination` | Hypoxic whispers |

The speaker cue column is the design intent. Nothing triggers cues yet; the hook exists so gameplay code can call it once the Sony SDK body is written.

## Device units

The profile works on a 0..8 design scale (positions, strengths, amplitude; frequency in Hz). `FDualSenseFeedbackProfile::ToDeviceUnits` is the only place that converts to what the platform expects. Under `PLATFORM_PS5` values pass through unchanged. On other platforms the engine's Apple DualSense path divides position, strength, amplitude and frequency by 255, so 8 maps to 255 and frequency is normalised against the 12 Hz maximum. Both conversions are unverified on hardware.

A trigger holds one mode, so the component sends one property at a time: vibration when vibrating, resistance when any strength is non-zero, otherwise a trigger reset. On end play the triggers are reset and the lightbar is switched off.
