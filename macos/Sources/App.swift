import AppKit
import SceneKit

final class GameView: SCNView {
    var input: InputController!
    var look: ((CGFloat,CGFloat)->Void)?
    var clicked: ((NSPoint)->Void)?
    override var acceptsFirstResponder: Bool { true }
    override func keyDown(with event:NSEvent) { if !event.isARepeat { input.keyDown(event.keyCode) } }
    override func keyUp(with event:NSEvent) { input.keyUp(event.keyCode) }
    override func mouseDragged(with event:NSEvent) { look?(event.deltaX,event.deltaY) }
    override func rightMouseDragged(with event:NSEvent) { look?(event.deltaX,event.deltaY) }
    override func mouseDown(with event:NSEvent) { clicked?(convert(event.locationInWindow, from:nil)) }
}

@main final class App: NSObject, NSApplicationDelegate {
    static func main() { let app = NSApplication.shared; let delegate = App(); app.delegate = delegate; app.setActivationPolicy(.regular); app.run() }
    var window:NSWindow!
    var view:GameView!
    let model = GameModel()
    let input = InputController()
    let camera = SCNNode()
    var yaw:Float = 0, pitch:Float = 0
    var zone = ""
    var paused = true
    var journal = false
    var last = Date()
    var timer:Timer?
    let title = NSTextField(labelWithString: "40 DAYS UNDER")
    let status = NSTextField(labelWithString: "")
    let prompt = NSTextField(labelWithString: "")
    let objective = NSTextField(wrappingLabelWithString: "")
    let notice = NSTextField(wrappingLabelWithString: "")
    let panel = NSVisualEffectView()
    let panelText = NSTextField(wrappingLabelWithString: "")
    var nearby:String?
    var sound:NSSound?
    func applicationDidFinishLaunching(_ notification:Notification) {
        window = NSWindow(contentRect:NSRect(x:0,y:0,width:1280,height:800), styleMask:[.titled,.closable,.resizable,.miniaturizable], backing:.buffered, defer:false)
        window.title = "40 Days Under • Lab 04"; window.minSize = NSSize(width:960,height:640); window.center()
        view = GameView(frame:window.contentView!.bounds); view.autoresizingMask = [.width,.height]
        view.input = input; view.preferredFramesPerSecond = 60; view.antialiasingMode = .multisampling4X
        view.isPlaying = true; window.contentView = view
        camera.camera = SCNCamera(); camera.camera!.fieldOfView = 75; camera.camera!.zNear = 0.08; camera.camera!.zFar = 160
        let light = SCNNode(); light.light = SCNLight(); light.light!.type = .omni; light.light!.intensity = 110; light.light!.color = NSColor(calibratedRed:0.5,green:0.8,blue:0.73,alpha:1); camera.addChildNode(light)
        setupHUD()
        input.onAction = { [weak self] in self?.action($0) }
        input.onControllerChanged = { [weak self] _ in self?.refresh() }
        input.start()
        view.look = { [weak self] x,y in guard let self = self, !self.paused else { return }; self.yaw -= Float(x)*0.005; self.pitch = min(1.2,max(-1.2,self.pitch-Float(y)*0.005)) }
        view.clicked = { [weak self] point in guard let self = self, !self.paused else { return }; self.interact(at:point) }
        model.onChange = { [weak self] in self?.refresh() }
        loadZone("lab"); refresh()
        timer = Timer.scheduledTimer(withTimeInterval:1.0/60.0,repeats:true) { [weak self] _ in self?.step() }
        NotificationCenter.default.addObserver(forName:NSWindow.didResignKeyNotification,object:window,queue:.main) { [weak self] _ in self?.paused = true; self?.input.reset(); self?.model.save(); self?.refresh() }
        window.makeKeyAndOrderFront(nil); window.makeFirstResponder(view); NSApp.activate(ignoringOtherApps:true)
        setupMenu()
    }
    func setupMenu() {
        let root = NSMenu(); let item = NSMenuItem(); root.addItem(item); let menu = NSMenu(); item.submenu = menu
        menu.addItem(withTitle:"Quit 40 Days Under",action:#selector(NSApplication.terminate(_:)),keyEquivalent:"q")
        NSApp.mainMenu = root
    }
    func label(_ field:NSTextField, size:CGFloat, color:NSColor = .white) {
        field.font = NSFont.monospacedSystemFont(ofSize:size,weight:.medium); field.textColor = color
        field.translatesAutoresizingMaskIntoConstraints = false; view.addSubview(field)
    }
    func setupHUD() {
        label(title,size:18,color:NSColor(calibratedRed:0.7,green:0.92,blue:0.84,alpha:1)); label(status,size:12)
        label(objective,size:15); label(prompt,size:13); label(notice,size:14,color:.init(calibratedRed:1,green:0.78,blue:0.47,alpha:1))
        panel.material = .hudWindow; panel.blendingMode = .withinWindow; panel.state = .active; panel.wantsLayer = true; panel.layer?.cornerRadius = 18; panel.translatesAutoresizingMaskIntoConstraints = false; view.addSubview(panel)
        panelText.font = .monospacedSystemFont(ofSize:15,weight:.regular); panelText.textColor = .white; panelText.translatesAutoresizingMaskIntoConstraints = false; panel.addSubview(panelText)
        NSLayoutConstraint.activate([
            title.leadingAnchor.constraint(equalTo:view.leadingAnchor,constant:28),title.topAnchor.constraint(equalTo:view.topAnchor,constant:24),
            status.trailingAnchor.constraint(equalTo:view.trailingAnchor,constant:-28),status.topAnchor.constraint(equalTo:title.topAnchor),
            objective.leadingAnchor.constraint(equalTo:title.leadingAnchor),objective.bottomAnchor.constraint(equalTo:view.bottomAnchor,constant:-76),objective.widthAnchor.constraint(lessThanOrEqualToConstant:850),
            notice.leadingAnchor.constraint(equalTo:title.leadingAnchor),notice.bottomAnchor.constraint(equalTo:objective.topAnchor,constant:-16),notice.widthAnchor.constraint(lessThanOrEqualToConstant:850),
            prompt.centerXAnchor.constraint(equalTo:view.centerXAnchor),prompt.bottomAnchor.constraint(equalTo:view.bottomAnchor,constant:-28),
            panel.centerXAnchor.constraint(equalTo:view.centerXAnchor),panel.centerYAnchor.constraint(equalTo:view.centerYAnchor),panel.widthAnchor.constraint(equalToConstant:650),
            panelText.leadingAnchor.constraint(equalTo:panel.leadingAnchor,constant:30),panelText.trailingAnchor.constraint(equalTo:panel.trailingAnchor,constant:-30),panelText.topAnchor.constraint(equalTo:panel.topAnchor,constant:28),panelText.bottomAnchor.constraint(equalTo:panel.bottomAnchor,constant:-28)
        ])
    }
    func loadZone(_ name:String) {
        zone = name; view.scene = WorldBuilder.build(zone:name); camera.removeFromParentNode(); view.scene!.rootNode.addChildNode(camera); camera.position = SCNVector3(0,1.65,6); yaw = 0; pitch = 0; view.pointOfView = camera
    }
    func refresh() {
        guard view != nil else { return }
        let s = model.state
        let next = s.location.lowercased().contains("marina") || s.phase == "B_DOCK" ? "marina" : (s.phase == "B_OUTBOUND" || s.phase == "B_RETURN" ? "river" : (s.phase == "ENDING_A" ? "boat" : "lab"))
        if zone != next { loadZone(next) }
        status.stringValue = "DAY \(s.day) / 40  •  \(input.controllerName)"
        objective.stringValue = model.objective
        notice.stringValue = model.message
        panel.isHidden = !paused && !journal && s.phase != "DEAD" && !s.phase.hasPrefix("ENDING") && s.phase != "CRISIS"
        let meters = String(format:"DIESEL %6.1f L   WATER %5.1f L\nBATTERY %5.1f   HEALTH %3.0f%%\nNITROGEN %3.0f%%   RATIONS %4.0f",s.fuel,s.water,s.battery,s.health,s.nitrogen*100,s.meals)
        if s.phase == "DEAD" || s.phase.hasPrefix("ENDING") {
            panelText.stringValue = "\(s.phase == "DEAD" ? "THE RIVER KEEPS ITS OWN" : "YOU SURVIVED")\n\n\(model.message)\n\n\(meters)\n\nCross / E — begin a new run\nYour completed run is archived."
        } else if s.phase == "CRISIS" {
            panelText.stringValue = "DAY 40 • A VOICE UPRIVER\n\nMercy has a family aboard and diesel to spare.\nAnswering reveals your hiding place.\n\nL1 / 1 — BROADCAST YOUR LOCATION\nR1 / 2 — KEEP SILENT. GO ALONE.\n\n\(meters)"
        } else {
            panelText.stringValue = "\(paused ? "40 DAYS UNDER • PAUSED" : "LAB 04 • DIVE WATCH")\n\nChristian Thomas / Marine robotics engineer\nDetroit River • 15 metres below\n\n\(meters)\n\nLeft stick / WASD — move\nRight stick / mouse drag — look\nCross / E — use nearby station\nTouchpad / Tab — watch & journal\nOptions / Escape — pause / resume\n\n\(paused ? "Cross / E to enter Lab 04" : model.objective)\n\n\(model.state.tutorialStep < 5 ? "Follow the lit consoles to restore the station." : "Bunk: advance one day. Radio: check supply boat.")"
        }
    }
    func action(_ action:String) {
        if action == "pause" { paused.toggle(); input.reset(); model.save(); refresh(); return }
        if model.state.phase == "DEAD" || model.state.phase.hasPrefix("ENDING") { if action == "interact" { model.newRun(); paused = false; loadZone("lab"); refresh() }; return }
        if paused { if action == "interact" { paused = false; refresh() }; return }
        if action == "journal" { journal.toggle(); refresh(); return }
        if model.state.phase == "CRISIS" { if action == "choiceA" { model.choose(branch:"A") }; if action == "choiceB" { model.choose(branch:"B") }; refresh(); return }
        if action == "interact" || action == "tool" { if let id = nearby { model.interact(station:id); refresh() } }
        if action == "fire" { fire() }
    }
    func interact(at point:NSPoint) {
        for hit in view.hitTest(point,options:nil) { var n:SCNNode? = hit.node; while let node = n { if let name = node.name, name.hasPrefix("interact:") { let p = node.worldPosition; let c = camera.position; let distance = sqrt(pow(p.x-c.x,2)+pow(p.z-c.z,2)); if distance < 4 { model.interact(station:String(name.dropFirst(9))); refresh() }; return }; n = node.parent } }
    }
    func fire() {
        let origin = camera.presentation.worldPosition
        let forward = camera.presentation.worldFront
        let projectile = SCNNode(geometry:SCNCylinder(radius:0.015,height:0.6)); projectile.geometry?.firstMaterial?.diffuse.contents = NSColor.lightGray
        projectile.position = origin; projectile.eulerAngles = camera.eulerAngles; projectile.eulerAngles.x += .pi/2
        view.scene?.rootNode.addChildNode(projectile)
        projectile.runAction(.sequence([.move(by:SCNVector3(forward.x*18,forward.y*18,forward.z*18),duration:0.4),.removeFromParentNode()]))
    }
    func step() {
        let now = Date(); let dt = min(0.1,now.timeIntervalSince(last)); last = now
        guard !paused, model.state.phase != "DEAD", !model.state.phase.hasPrefix("ENDING") else { return }
        model.tick(delta:dt)
        guard !journal, model.state.phase != "CRISIS" else { return }
        yaw -= input.lookX*Float(dt)*2; pitch = min(1.2,max(-1.2,pitch+input.lookY*Float(dt)*1.6))
        camera.eulerAngles = SCNVector3(pitch,yaw,0)
        let speed:Float = zone == "river" ? 3 : 2.7
        let dx = input.moveX, dz = input.moveY
        camera.position.x += (cos(yaw)*dx-sin(yaw)*dz)*Float(dt)*speed
        camera.position.z += (-sin(yaw)*dx-cos(yaw)*dz)*Float(dt)*speed
        let bound:Float = zone == "lab" ? 5.1 : 18
        camera.position.x = max(-bound,min(bound,camera.position.x)); camera.position.z = max(zone == "lab" ? -12.8 : -35,min(7,camera.position.z))
        var best:Float = 3.5; nearby = nil
        view.scene?.rootNode.enumerateChildNodes { node,_ in guard let name = node.name,name.hasPrefix("interact:") else { return }; let p = node.worldPosition; let d = sqrt(pow(p.x-self.camera.position.x,2)+pow(p.z-self.camera.position.z,2)); if d < best { best = d; self.nearby = String(name.dropFirst(9)) } }
        prompt.stringValue = nearby.map { "[ ✕ / E ]  " + $0.uppercased() } ?? "Left stick / WASD to move • Right stick / drag to look"
    }
    func applicationWillTerminate(_ notification:Notification) { model.save() }
    func applicationShouldTerminateAfterLastWindowClosed(_ sender:NSApplication)->Bool { true }
}
