# Mac target with PS5 controls

The target machine is the user's M3 Pro MacBook. Ship a native macOS application for Apple silicon. Retain the PS5/DualSense control scheme as the primary input; this is not a PS5 console build. Existing controller event IDs and tuning remain intact.

## Proposed default action map

| Input | Action |
|---|---|
| Left stick | Move / swim / steer ROV |
| Right stick | Look / aim / ROV camera |
| L2 | Aim speargun / precision interaction |
| R2 | Fire / operate selected tool / turn held valve |
| Cross | Interact / confirm |
| Circle | Cancel / release interaction / crouch toggle on land |
| Square | Reload / service selected equipment |
| Triangle | Inspect wrist watch / switch held tool |
| L1 / R1 | Previous / next tool |
| D-pad | Navigate physical console selections |
| Touchpad click | Open journal / task interface |
| Options | Pause |

These are application action bindings to implement, not an existing engine input asset. Allow full remapping and toggle alternatives to holds. Use PS button glyphs. ROV mode reuses movement/look controls with a visible contextual recall command. Reserve conflicting actions through interaction context rather than simultaneous bindings.

## Feedback capability policy

Keep adaptive-trigger, haptic, speaker and lightbar designs from HARDWARE.md. Their availability is unverified for the selected macOS engine/backend and connection. Do not assume console behavior transfers to Mac. Discover capabilities at runtime and disable unsupported output channels without disabling controls or affecting survival math.

- Trigger resistance fallback: tool animation, main-output mechanical audio and visible load indication.
- Haptic fallback: main audio plus diegetic impact/current cues; optional conventional vibration only where verified.
- Controller speaker fallback: Mac/headphone audio with captions.
- Lightbar fallback: physical in-world alarm lamps and readable watch indicators.

The PS/system button remains outside required gameplay bindings. Provide an alternate remappable journal binding if touchpad click is unavailable. Validate USB and Bluetooth separately if both are supported by the chosen backend. Keyboard/mouse bindings may be secondary, not a replacement for PS5 controls.

## Build and acceptance gates

Implement the engine scene and macOS adapter, package for Apple silicon, and profile the actual M3 Pro configuration before setting final resolution, frame-rate or graphics defaults. No performance claim follows from the current headless tests. Acceptance requires the full opening and both branches to be completable using the controller, including menus, reconnect handling and all optional feedback channels disabled.
