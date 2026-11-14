import AppKit
import GameController

/// Application action input. All consumers and state updates run on the main queue.
/// Positive moveY is forward; positive lookY is up. Axes include a rescaled dead zone.
final class InputController {
    var onAction: ((String) -> Void)?
    var onControllerChanged: ((String) -> Void)?
    private(set) var controllerName = "Keyboard / mouse"
    private var keys = Set<UInt16>()
    private var controller: GCController?
    private var observers: [NSObjectProtocol] = []
    private var started = false
    private var padMoveX: Float = 0
    private var padMoveY: Float = 0
    private var padLookX: Float = 0
    private var padLookY: Float = 0

    var moveX: Float { clamp(padMoveX + axis(positive: 2, negative: 0)) }
    var moveY: Float { clamp(padMoveY + axis(positive: 13, negative: 1)) }
    var lookX: Float { clamp(padLookX + axis(positive: 124, negative: 123)) }
    var lookY: Float { clamp(padLookY + axis(positive: 126, negative: 125)) }

    private func clamp(_ value: Float) -> Float { min(1, max(-1, value)) }
    private func axis(positive: UInt16, negative: UInt16) -> Float {
        (keys.contains(positive) ? 1 : 0) - (keys.contains(negative) ? 1 : 0)
    }
    private func filtered(_ value: Float) -> Float {
        guard value.isFinite, abs(value) > 0.18 else { return 0 }
        return (value < 0 ? -1 : 1) * min(1, (abs(value) - 0.18) / 0.82)
    }

    func start() {
        guard !started else { return }
        started = true
        let center = NotificationCenter.default
        observers.append(center.addObserver(forName: .GCControllerDidConnect, object: nil, queue: .main) { [weak self] _ in
            guard let self = self, self.controller == nil else { return }
            self.selectController()
        })
        observers.append(center.addObserver(forName: .GCControllerDidDisconnect, object: nil, queue: .main) { [weak self] note in
            guard let self = self, let disconnected = note.object as? GCController,
                  disconnected === self.controller else { return }
            self.detach()
            self.reset()
            self.onAction?("controllerDisconnected")
            self.selectController()
        })
        selectController()
    }

    private func selectController() {
        guard let found = GCController.controllers().first(where: { $0.extendedGamepad != nil }),
              let pad = found.extendedGamepad else {
            controllerName = "Keyboard / mouse"
            onControllerChanged?(controllerName)
            return
        }
        controller = found
        found.handlerQueue = .main
        controllerName = found.vendorName ?? "Game controller"
        pad.leftThumbstick.valueChangedHandler = { [weak self] _, x, y in
            guard let self = self else { return }
            self.padMoveX = self.filtered(x); self.padMoveY = self.filtered(y)
        }
        pad.rightThumbstick.valueChangedHandler = { [weak self] _, x, y in
            guard let self = self else { return }
            self.padLookX = self.filtered(x); self.padLookY = self.filtered(y)
        }
        bind(pad.buttonA, "interact")  // Cross
        bind(pad.buttonB, "cancel")    // Circle
        bind(pad.buttonX, "tool")      // Square
        bind(pad.buttonY, "rest")      // Triangle
        bind(pad.leftShoulder, "choiceA")
        bind(pad.rightShoulder, "choiceB")
        bind(pad.rightTrigger, "fire")
        bind(pad.buttonMenu, "pause")
        if let options = pad.buttonOptions { bind(options, "journal") }
        if let dualSense = pad as? GCDualSenseGamepad {
            bind(dualSense.touchpadButton, "journal")
        } else if let dualShock = pad as? GCDualShockGamepad {
            bind(dualShock.touchpadButton, "journal")
        }
        onControllerChanged?(controllerName)
    }

    private func bind(_ button: GCControllerButtonInput, _ action: String) {
        button.pressedChangedHandler = { [weak self] _, _, pressed in
            if pressed { self?.onAction?(action) }
        }
    }

    func keyDown(_ code: UInt16) {
        // NSEvent auto-repeat must never repeatedly commit choices or interactions.
        guard keys.insert(code).inserted else { return }
        let actions: [UInt16: String] = [14: "interact", 53: "pause", 48: "journal",
                                       49: "tool", 18: "choiceA", 19: "choiceB",
                                       15: "rest", 3: "fire", 8: "cancel"]
        if let action = actions[code] { onAction?(action) }
    }
    func keyUp(_ code: UInt16) { keys.remove(code) }
    func reset() {
        keys.removeAll()
        padMoveX = 0; padMoveY = 0; padLookX = 0; padLookY = 0
    }
    private func detach() {
        guard let pad = controller?.extendedGamepad else { controller = nil; return }
        pad.leftThumbstick.valueChangedHandler = nil
        pad.rightThumbstick.valueChangedHandler = nil
        [pad.buttonA, pad.buttonB, pad.buttonX, pad.buttonY, pad.leftShoulder,
         pad.rightShoulder, pad.rightTrigger, pad.buttonMenu].forEach { $0.pressedChangedHandler = nil }
        pad.buttonOptions?.pressedChangedHandler = nil
        (pad as? GCDualSenseGamepad)?.touchpadButton.pressedChangedHandler = nil
        (pad as? GCDualShockGamepad)?.touchpadButton.pressedChangedHandler = nil
        controller = nil
    }
    deinit {
        detach()
        observers.forEach { NotificationCenter.default.removeObserver($0) }
    }
}
