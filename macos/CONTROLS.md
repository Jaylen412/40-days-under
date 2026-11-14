# Mac controls

The game targets the M3 Pro MacBook and retains PS5-style controls through Apple's GameController framework. Controller support uses the standard extended gamepad profile and additionally recognizes DualSense/DualShock touchpad clicks. Actual controller connection and input must be checked on the user's device; successful compilation is not hardware certification.

| Action | DualSense | Keyboard / mouse |
|---|---|---|
| Move / swim | Left stick | W A S D |
| Look | Right stick | Mouse; arrow-key fallback |
| Interact with targeted station | Cross | E |
| Cancel / close | Circle | C |
| Contextual tool | Square | Space |
| Rest action | Triangle | R |
| Choose A | L1 | 1 |
| Choose B | R1 | 2 |
| Fire / contextual tool action | R2 | F |
| Pause / resume | Options | Escape |
| Journal / control guide | Touchpad click; secondary menu button fallback | Tab |

Buttons generate one action per press. Held movement and look use analog axes with an 18% dead zone. The host game determines which actions are valid in the current phase; pressing a branch button early must not commit a choice. On disconnect the host receives a pause request and stale movement is cleared. Reconnect does not resume gameplay automatically. The system/PS button is not needed for gameplay.

Use an already paired controller or connect it with a data-capable USB cable. Bluetooth pairing remains a macOS/user operation; the app does not change Bluetooth settings. Test USB and Bluetooth separately. Keyboard/mouse remain available while a controller is connected. Advanced adaptive triggers, controller speaker, lightbar and haptic output are not implemented by this input layer; essential game cues must appear through the game display and main audio.

Developer integration: call `start()` once on the main thread. Forward `NSEvent.keyCode` through `keyDown` / `keyUp`; call `reset()` on focus loss, pause and scene transitions. Read axes each frame on the main thread. Update the camera from mouse deltas in the host view. The `controllerDisconnected` action should pause gameplay immediately.

API references: [Apple extended gamepad profile](https://developer.apple.com/documentation/gamecontroller/gcextendedgamepad), [Apple DualSense profile](https://developer.apple.com/documentation/gamecontroller/gcdualsensegamepad).
