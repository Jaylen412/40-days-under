import AppKit
import SceneKit

/// Procedural greybox art; all geometry is local and needs no downloaded resources.
enum WorldBuilder {
    private static let steel = NSColor(calibratedRed: 0.13, green: 0.18, blue: 0.18, alpha: 1)
    private static let dark = NSColor(calibratedRed: 0.035, green: 0.065, blue: 0.07, alpha: 1)
    private static let teal = NSColor(calibratedRed: 0.05, green: 0.85, blue: 0.74, alpha: 1)
    private static let amber = NSColor(calibratedRed: 1, green: 0.53, blue: 0.15, alpha: 1)
    private static let rust = NSColor(calibratedRed: 0.3, green: 0.14, blue: 0.07, alpha: 1)

    static func build(zone: String) -> SCNScene {
        let scene = SCNScene()
        scene.background.contents = dark
        scene.fogColor = NSColor(calibratedRed: 0.025, green: 0.12, blue: 0.105, alpha: 1)
        scene.fogStartDistance = zone == "lab" ? 24 : 8
        scene.fogEndDistance = zone == "lab" ? 55 : 65
        scene.lightingEnvironment.intensity = 0.55
        let ambient = SCNNode(); ambient.light = SCNLight(); ambient.light!.type = .ambient
        ambient.light!.color = NSColor(calibratedRed: 0.38, green: 0.55, blue: 0.5, alpha: 1)
        ambient.light!.intensity = zone == "lab" ? 260 : 380
        scene.rootNode.addChildNode(ambient)
        switch zone {
        case "river": river(scene)
        case "marina": marina(scene)
        case "boat": boat(scene)
        default: lab(scene)
        }
        return scene
    }

    private static func material(_ color: NSColor, glow: Bool = false, metal: CGFloat = 0.3) -> SCNMaterial {
        let m = SCNMaterial(); m.diffuse.contents = color; m.roughness.contents = 0.72
        m.metalness.contents = metal; m.lightingModel = .physicallyBased
        if glow { m.emission.contents = color }
        return m
    }
    @discardableResult private static func box(_ root: SCNNode, _ size: SCNVector3, _ at: SCNVector3, _ color: NSColor, glow: Bool = false, chamfer: CGFloat = 0.03) -> SCNNode {
        let g = SCNBox(width: CGFloat(size.x), height: CGFloat(size.y), length: CGFloat(size.z), chamferRadius: chamfer)
        g.materials = [material(color, glow: glow)]
        let n = SCNNode(geometry: g); n.position = at; root.addChildNode(n); return n
    }
    @discardableResult private static func cylinder(_ root: SCNNode, radius: CGFloat, height: CGFloat, at: SCNVector3, color: NSColor) -> SCNNode {
        let g = SCNCylinder(radius: radius, height: height); g.radialSegmentCount = 16
        g.materials = [material(color)]; let n = SCNNode(geometry: g); n.position = at; root.addChildNode(n); return n
    }
    private static func text(_ root: SCNNode, _ value: String, at: SCNVector3, scale: Float = 0.035, color: NSColor = .white) {
        let g = SCNText(string: value, extrusionDepth: 0.15); g.font = NSFont.monospacedSystemFont(ofSize: 10, weight: .semibold)
        g.flatness = 0.3; g.materials = [material(color, glow: true, metal: 0)]
        let n = SCNNode(geometry: g); n.scale = SCNVector3(scale, scale, scale); n.position = at
        root.addChildNode(n)
    }
    private static func light(_ root: SCNNode, at: SCNVector3, color: NSColor, power: CGFloat = 450) {
        let n = SCNNode(); n.position = at; let l = SCNLight(); l.type = .omni
        l.color = color; l.intensity = power; l.attenuationStartDistance = 1; l.attenuationEndDistance = 12
        n.light = l; root.addChildNode(n)
    }
    private static func station(_ root: SCNNode, action: String, title: String, detail: String, x: Float, z: Float, yaw: Float = 0, color: NSColor = teal) {
        let n = SCNNode(); n.name = "interact:\(action)"; n.position = SCNVector3(x, 0, z); n.eulerAngles.y = CGFloat(yaw)
        root.addChildNode(n)
        box(n, SCNVector3(1.6, 1.25, 0.65), SCNVector3(0, 0.625, 0), steel)
        box(n, SCNVector3(1.5, 0.68, 0.13), SCNVector3(0, 1.54, -0.14), dark)
        box(n, SCNVector3(1.28, 0.42, 0.02), SCNVector3(0, 1.55, -0.06), color.withAlphaComponent(0.65), glow: true)
        text(n, title, at: SCNVector3(-0.72, 2.06, 0.03), scale: 0.022, color: color)
        text(n, detail, at: SCNVector3(-0.71, 0.75, 0.34), scale: 0.014, color: .lightGray)
        text(n, "[X] / [E]", at: SCNVector3(-0.37, 1.28, 0.34), scale: 0.019, color: .white)
        for i in 0..<4 {
            box(n, SCNVector3(0.13, 0.06, 0.1), SCNVector3(-0.5 + Float(i) * 0.32, 1.27, 0.17), i == 3 ? amber : color, glow: true)
        }
    }
    private static func lab(_ scene: SCNScene) {
        let r = scene.rootNode
        box(r, SCNVector3(12, 0.25, 22), SCNVector3(0, -0.125, -3), steel)
        box(r, SCNVector3(12, 0.2, 22), SCNVector3(0, 4.3, -3), dark)
        for x: Float in [-6, 6] { box(r, SCNVector3(0.2, 4.4, 22), SCNVector3(x, 2.1, -3), steel) }
        for z: Float in [-14, 8] { box(r, SCNVector3(12, 4.4, 0.2), SCNVector3(0, 2.1, z), dark) }
        for z in stride(from: Float(-13), through: 7, by: 2) {
            for x: Float in [-5.8, 5.8] { box(r, SCNVector3(0.22, 4.2, 0.22), SCNVector3(x, 2.1, z), rust) }
            box(r, SCNVector3(11.7, 0.17, 0.2), SCNVector3(0, 4.05, z), rust)
            box(r, SCNVector3(0.055, 0.01, 1.3), SCNVector3(-1.8, 0.015, z), amber, glow: true)
            box(r, SCNVector3(0.055, 0.01, 1.3), SCNVector3(1.8, 0.015, z), amber, glow: true)
        }
        for x: Float in [-4.9, -4.55, 4.9] {
            let p = cylinder(r, radius: 0.09, height: 21, at: SCNVector3(x, 3.65, -3), color: x > 0 ? amber : rust)
            p.eulerAngles.x = .pi / 2
        }
        for z: Float in [-10, -3, 4] {
            light(r, at: SCNVector3(0, 3.4, z), color: amber, power: 570)
            box(r, SCNVector3(2.2, 0.08, 0.24), SCNVector3(0, 3.95, z), amber, glow: true)
        }
        // A murky observation window with layered silhouettes outside the glass.
        box(r, SCNVector3(4.3, 2.1, 0.18), SCNVector3(0, 2.2, -13.72), rust)
        box(r, SCNVector3(3.9, 1.7, 0.08), SCNVector3(0, 2.2, -13.58), NSColor(calibratedRed: 0.02, green: 0.25, blue: 0.19, alpha: 1), glow: true)
        for i in 0..<7 { body(r, at: SCNVector3(Float(i) * 0.55 - 1.6, 2.8, -13.45), floating: true, scale: 0.43) }
        text(r, "LAB 04 / DETROIT RIVER", at: SCNVector3(-3.5, 3.55, -13.3), scale: 0.043, color: teal)
        station(r, action: "inspect", title: "01 / AIR MANIFOLD", detail: "COMPARE PRESSURE", x: -4.7, z: 4, yaw: .pi / 2)
        station(r, action: "repair", title: "02 / SENSOR ARRAY", detail: "RESEAT INPUT 03", x: -4.7, z: 0, yaw: .pi / 2)
        station(r, action: "firmware", title: "03 / ENGINEERING", detail: "SIGNED PROFILE", x: -4.7, z: -4, yaw: .pi / 2)
        station(r, action: "drone", title: "04 / ROV CONTROL", detail: "CLEAR THE INTAKE", x: -4.7, z: -8, yaw: .pi / 2)
        station(r, action: "log", title: "05 / FIELD JOURNAL", detail: "RECORD / OBJECTIVES", x: 4.7, z: 4, yaw: -.pi / 2)
        station(r, action: "treat", title: "WATER / RATIONS", detail: "RECOVER / MAINTAIN", x: 4.7, z: 0, yaw: -.pi / 2)
        station(r, action: "radio", title: "HYDROPHONE / RADIO", detail: "LISTEN / CHOOSE", x: 4.7, z: -4, yaw: -.pi / 2, color: amber)
        station(r, action: "rest", title: "BUNK / PASS TIME", detail: "REST 24 HOURS", x: 4.7, z: -8, yaw: -.pi / 2)
        station(r, action: "scavenge", title: "WRECK / SALVAGE", detail: "ROV EXPEDITION", x: -3.5, z: -11.5)
        station(r, action: "prepare", title: "UMBILICAL BENCH", detail: "ASSEMBLE RATED KIT", x: 3.5, z: -11.5)
        station(r, action: "deco", title: "PRESSURE LOCK", detail: "17 HOUR GAME CYCLE", x: -2.5, z: 7, yaw: .pi, color: amber)
        station(r, action: "depart", title: "TRANSFER CAPSULE", detail: "MARINA RUN", x: 0, z: 7, yaw: .pi, color: amber)
        station(r, action: "defend", title: "BULKHEAD CONTROL", detail: "HOLD / REINFORCE", x: 2.5, z: 7, yaw: .pi)
        station(r, action: "escape", title: "TRAWLER EVAC", detail: "SCUTTLE / ESCAPE", x: 4.7, z: 7, yaw: .pi)
    }
    private static func body(_ root: SCNNode, at: SCNVector3, floating: Bool, scale: Float = 1) {
        let n = SCNNode(); n.position = at; n.scale = SCNVector3(scale, scale, scale)
        box(n, SCNVector3(0.45, 0.72, 0.23), SCNVector3(0, 0.9, 0), dark)
        let head = SCNSphere(radius: 0.16); head.segmentCount = 10; head.materials = [material(steel)]
        let h = SCNNode(geometry: head); h.position.y = 1.43; n.addChildNode(h)
        for x: Float in [-0.14, 0.14] { box(n, SCNVector3(0.15, 0.65, 0.18), SCNVector3(x, 0.28, 0), dark) }
        for x: Float in [-0.32, 0.32] { let a = box(n, SCNVector3(0.14, 0.64, 0.16), SCNVector3(x, 0.85, 0), dark); a.eulerAngles.z = CGFloat(x * 0.8) }
        if floating { n.eulerAngles.x = .pi / 2; n.runAction(.repeatForever(.sequence([.moveBy(x: 0.15, y: 0, z: 0, duration: 4), .moveBy(x: -0.15, y: 0, z: 0, duration: 4)]))) }
        root.addChildNode(n)
    }
    private static func river(_ scene: SCNScene) {
        let r = scene.rootNode
        box(r, SCNVector3(100, 0.4, 120), SCNVector3(0, -1, -30), steel)
        box(r, SCNVector3(100, 0.1, 120), SCNVector3(0, 9, -30), NSColor(calibratedRed: 0.03, green: 0.16, blue: 0.11, alpha: 1), glow: true)
        for i in 0..<35 { body(r, at: SCNVector3(Float((i * 13) % 43) - 21, 8.6, Float(-((i * 7) % 65))), floating: true) }
        let wreck = box(r, SCNVector3(8, 6, 31), SCNVector3(-14, 1.5, -21), rust); wreck.eulerAngles.z = -0.2
        for i in 0..<10 { box(r, SCNVector3(0.15, 0.08, 2.2), SCNVector3(Float(i % 3) * 0.3, 0.1, -Float(i) * 4), amber, glow: true) }
        light(r, at: SCNVector3(0, 3, 3), color: teal, power: 1000)
        station(r, action: "depart", title: "SHALLOW ROUTE / MARINA", detail: "FOLLOW THE UMBILICAL", x: 0, z: -8, color: amber)
        station(r, action: "return", title: "CAPSULE / LAB 04", detail: "RETURN TO STATION", x: 3, z: 2)
        text(r, "UNDER THE LIVING CEILING", at: SCNVector3(-4, 4, -10), scale: 0.05, color: teal)
    }
    private static func marina(_ scene: SCNScene) {
        let r = scene.rootNode
        scene.background.contents = NSColor(calibratedRed: 0.035, green: 0.065, blue: 0.12, alpha: 1)
        scene.fogColor = NSColor(calibratedRed: 0.08, green: 0.12, blue: 0.16, alpha: 1)
        box(r, SCNVector3(150, 0.08, 150), SCNVector3(0, -0.7, -30), NSColor(calibratedRed: 0.025, green: 0.095, blue: 0.12, alpha: 1))
        box(r, SCNVector3(9, 0.3, 30), SCNVector3(0, -0.15, -6), rust)
        for z in stride(from: Float(-20), through: 8, by: 0.6) { box(r, SCNVector3(9, 0.03, 0.035), SCNVector3(0, 0.015, z), dark) }
        for x: Float in [-4.2, 4.2] { for z: Float in [-17,-9,-1,7] { cylinder(r, radius: 0.2, height: 3, at: SCNVector3(x, 0.4, z), color: steel) } }
        station(r, action: "secureFuel", title: "DIESEL / MANUAL TRANSFER", detail: "FILL THE FUEL SLED", x: 0, z: -9, color: amber)
        station(r, action: "return", title: "TETHER / RETURN", detail: "BACK TO LAB 04", x: 2, z: 4)
        for x: Float in [-13, 13] {
            let hull = box(r, SCNVector3(4, 1.8, 12), SCNVector3(x, 0, -6), .lightGray); hull.eulerAngles.z = CGFloat(x * 0.018)
            box(hull, SCNVector3(2.7, 1.8, 4), SCNVector3(0, 1.5, -1), steel)
        }
        box(r, SCNVector3(70, 0.5, 12), SCNVector3(0, 0, -30), steel)
        for i in 0..<12 { body(r, at: SCNVector3(Float(i) * 3.4 - 18, 0.3, -26 - Float(i % 3)), floating: false) }
        for i in 0..<5 { cylinder(r, radius: i == 2 ? 2 : 1.6, height: CGFloat(19 + i % 2 * 6), at: SCNVector3(Float(i) * 4 - 8, 10, -65), color: dark) }
        for x: Float in [-3.8, 3.8] {
            cylinder(r, radius: 0.06, height: 4, at: SCNVector3(x, 2, -5), color: steel)
            box(r, SCNVector3(0.4, 0.2, 0.4), SCNVector3(x, 4, -5), amber, glow: true)
            light(r, at: SCNVector3(x, 3.8, -5), color: amber, power: 650)
        }
        text(r, "MILLIKEN / FUEL DOCK", at: SCNVector3(-3.5, 3.2, -12), scale: 0.04, color: amber)
    }
    private static func boat(_ scene: SCNScene) {
        marina(scene)
        text(scene.rootNode, "MERCY / UPRIVER", at: SCNVector3(-3, 2.8, -4), scale: 0.065, color: teal)
    }
}
