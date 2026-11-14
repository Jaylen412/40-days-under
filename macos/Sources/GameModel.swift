import Foundation

struct GameState: Codable {
    var version = 1
    var runID = UUID().uuidString
    var elapsedHours: Double = 0
    var fuel: Double = 970
    var water: Double = 60
    var meals: Double = 700
    var battery: Double = 12
    var health: Double = 100
    var stamina: Double = 100
    var nitrogen: Double = 0
    var isolation: Double = 0
    var phase = "TUTORIAL"
    var tutorialStep = 0
    var branch = ""
    var cargoFuel: Double = 0
    var salvage = 12
    var location = "LAB"
    var phaseHours: Double = 0
    var prepared = false
    var stealthCompromised = false
    var reason = ""
    var day: Int { 1 + Int(elapsedHours / 24) }
    var terminal: Bool { phase == "DEAD" || phase.hasPrefix("ENDING") }
}

final class GameModel {
    var state = GameState()
    var message = "Christian Thomas · Lab 04. Compare the digital pump reading with the mechanical gauge."
    var onChange: (() -> Void)?
    var campaignHoursPerSecond = 0.05
    private let saveURL: URL
    private var autosaveSeconds: Double = 0
    private var lowFuelWarned = false
    private var stoppedWarned = false
    private(set) var persistenceError: String?

    init(saveURL: URL? = nil) {
        self.saveURL = saveURL ?? FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)[0]
            .appendingPathComponent("40 Days Under", isDirectory: true).appendingPathComponent("run.json")
        load()
    }

    var objective: String {
        switch state.phase {
        case "TUTORIAL": return ["Inspect the pump gauge", "Repair the sensor connector", "Restore the signed firmware profile", "Pilot the ROV and clear the intake", "Record your first station log"][min(4, state.tutorialStep)]
        case "CAMPAIGN": return "Maintain Lab 04. Rest to advance a day; scavenge and monitor fuel. Supply boat expected on Day 21."
        case "CRISIS": return "Day 40: answer the trawler (Choice A), or prepare the marina run (Choice B)."
        case "A_RENDEZVOUS": return "Guide the family in. Wait or rest for the six-hour rendezvous."
        case "A_DEFENSE": return "Defend the station for 48 hours, or escape aboard the trawler."
        case "B_PREP": return "Assemble the rated umbilical kit, then begin the 17-hour game decompression cycle."
        case "B_DECO": return String(format: "Decompression: %.1f / 17 hours. Wait or rest; life support must stay powered.", state.phaseHours)
        case "B_READY": return "Decompression complete. Depart through the sealed transfer capsule."
        case "B_OUTBOUND": return "Follow the tether toward the marina. Reach the dock after the crossing."
        case "B_DOCK": return "Siphon diesel at the fuel dock. Keep the containers quiet."
        case "B_RETURN": return "Return along the tether, then dock and repressurize at Lab 04."
        case "ENDING_A": return "THE SIGNAL · You survived with the family. Start a new run to explore the other choice."
        case "ENDING_B": return "THE SWIM · Air is steady. Just you. Start a new run to explore the other choice."
        case "DEAD": return "Lab 04 is your tomb. A new run cannot undo this death."
        default: return "Inspect the station."
        }
    }

    func tick(delta: Double) {
        guard delta.isFinite, delta > 0, !state.terminal else { return }
        // Never simulate an enormous app-suspend delta or progress offline.
        let seconds = min(delta, 0.25)
        if state.phase != "TUTORIAL" { advance(hours: seconds * campaignHoursPerSecond) }
        autosaveSeconds += seconds
        if autosaveSeconds >= 10 { autosaveSeconds = 0; save() }
        onChange?()
    }

    func interact(station: String) {
        guard !state.terminal else { notify("This run has ended. Choose New Run to begin again."); return }
        let action = station.lowercased()
        if state.phase == "TUTORIAL" {
            let expected = ["inspect", "repair", "firmware", "drone", "log"]
            guard action == expected[state.tutorialStep] else { notify(objective); return }
            let lines = ["Pressure's holding. The screen's lying.", "Revision C. They never changed the labels. Sensor reseated.", "Don't silence it. Fix what it's telling you. Signed profile restored.", "Flow's back. They don't swim. The river brings them.", "Lab 04. Christian Thomas. Air is steady. For now."]
            message = lines[state.tutorialStep]
            state.tutorialStep += 1
            if state.tutorialStep == 5 { state.phase = "CAMPAIGN" }
            commit(); return
        }
        switch action {
        case "inspect", "log": notify(String(format: "Day %d · Diesel %.1f L · Battery %.1f kWh · %@", state.day, state.fuel, state.battery, objective))
        case "repair", "firmware":
            guard state.salvage > 0 else { notify("No spare parts. Scavenge the wreck with the ROV."); return }
            state.salvage -= 1; state.battery = min(12, state.battery + 2); state.health = min(100, state.health + 5)
            advance(hours: 1); notify("Maintenance complete. Electrical reserve restored.")
        case "drone", "scavenge":
            guard state.location == "LAB", state.battery >= 1 else { notify("The ROV needs 1 kWh and a connection to Lab 04."); return }
            state.battery -= 1; state.salvage += 2; advance(hours: 3)
            notify("ROV recovered two usable parts from the wreck. Tether safely recalled.")
        case "treat", "eat", "drink":
            guard state.meals >= 1, state.water >= 1 else { notify("Treatment needs one meal and one liter of water."); return }
            state.meals -= 1; state.water -= 1; state.health = min(100, state.health + 20); state.stamina = 100
            notify("You eat, hydrate, and steady your breathing.")
        case "rest":
            let hours: Double
            switch state.phase {
            case "A_RENDEZVOUS": hours = max(0, 6 - state.phaseHours)
            case "B_DECO": hours = max(0, 17 - state.phaseHours)
            case "A_DEFENSE": hours = 6
            case "B_OUTBOUND", "B_RETURN": hours = 0.5
            case "B_DOCK": hours = 1
            case "B_READY", "B_PREP", "CRISIS": hours = 1
            default: hours = min(24, max(0, 936 - state.elapsedHours))
            }
            advance(hours: hours); state.stamina = min(100, state.stamina + 15)
            if !state.terminal { notify("Time passed. " + objective) }
        case "radio":
            if state.day < 40 { notify(state.day >= 21 ? "The scheduled supply boat never arrived. Only static." : "No reply. The supply boat is due on Day 21.") }
            else { notify("Mercy: We have a child aboard. We have fuel. Please answer. Choose Broadcast (A) or Go Alone (B).") }
        case "prepare":
            guard state.phase == "B_PREP", !state.prepared, state.salvage >= 8 else { notify("Preparation needs Choice B and eight salvage parts; the kit can only be installed once."); return }
            state.salvage -= 8; state.prepared = true
            notify("Rated breathing kit, ROV tether and fuel sled checked. Decoy set for dockside infected.")
        case "deco":
            guard state.phase == "B_PREP", state.prepared, state.fuel >= 24, state.battery >= 2, state.health >= 50 else { notify("Need prepared kit, 24 L diesel, 2 kWh reserve and at least 50 health."); return }
            transition("B_DECO"); notify("Emergency cycle started. Seventeen GAME hours; not a real diving protocol.")
        case "depart":
            guard state.phase == "B_READY", state.fuel >= 7, state.health >= 50 else { notify("Complete decompression and retain 7 L diesel and 50 health before departing."); return }
            transition("B_OUTBOUND", location: "RIVER"); notify("Capsule sealed. Shallow launch reached. One route out. One line home.")
        case "securefuel", "fuel":
            guard state.phase == "B_DOCK", state.phaseHours >= 1 else { notify("Reach the marina and spend one game hour transferring diesel."); return }
            state.cargoFuel = 60; transition("B_RETURN", location: "RIVER")
            notify("Sixty liters secured on the sled. Follow the tether home.")
        case "return":
            guard state.phase == "B_RETURN", state.phaseHours >= 1.5 else { notify("The return crossing takes 1.5 game hours. Keep following the tether."); return }
            state.fuel += state.cargoFuel; state.cargoFuel = 0
            advance(hours: 0.25)
            if !state.terminal { transition("ENDING_B", location: "LAB"); notify("Lab 04. Air is steady. Just me.") }
        case "defend":
            guard state.phase == "A_DEFENSE" else { notify("The family has not arrived yet."); return }
            if state.salvage > 0 { state.salvage -= 1; state.health = min(100, state.health + 10) }
            advance(hours: 6)
            if !state.terminal { notify("Another watch complete. " + objective) }
        case "escape":
            guard state.phase == "A_DEFENSE", state.phaseHours >= 2 else { notify("Wait until the family arrives and stabilizes the trawler for two hours."); return }
            transition("ENDING_A", location: "BOAT"); notify("You scuttle Lab 04 and escape upriver aboard Mercy. You are not alone.")
        default: notify(objective)
        }
        commit()
    }

    func choose(branch: String) {
        guard state.phase == "CRISIS", state.branch.isEmpty else { notify("The Day 40 choice is not available, or has already been committed."); return }
        switch branch.uppercased() {
        case "A": state.branch = "A"; state.stealthCompromised = true; transition("A_RENDEZVOUS"); notify("Mercy, this is Lab 04. Coordinates transmitted. They can never be secret again.")
        case "B": state.branch = "B"; transition("B_PREP"); notify("Radio silence. Prepare the marina extraction kit.")
        default: return
        }
        commit()
    }

    private func advance(hours: Double) {
        var remaining = max(0, hours)
        while remaining > 0.000001 && !state.terminal {
            var dt = min(remaining, 1.0 / 60)
            if state.phase == "CAMPAIGN" { dt = min(dt, max(0, 936 - state.elapsedHours)) }
            if dt < 0.000001 { transition("CRISIS"); notify("Day 40. A family trawler broadcasts from two kilometers upriver."); break }
            let populated = state.phase == "A_DEFENSE"
            let burn = populated ? 1.3 : 1.0
            let pumpHours = min(dt, state.fuel / burn)
            state.fuel = max(0, state.fuel - burn * dt)
            state.battery = min(12, max(0, state.battery + (pumpHours / dt * 0.55 + 0.1 - (populated ? 0.6 : 0.48)) * dt))
            state.water = min(100, max(0, state.water + ((state.battery > 0 ? 20.0 : 0) - (populated ? 12 : 3)) / 24 * dt))
            state.meals = max(0, state.meals - (populated ? 12 : 3) / 24 * dt)
            state.health = max(0, state.health - (dt - pumpHours) * 30 - (state.water == 0 ? dt * 2 : 0) - (state.battery == 0 ? dt * 10 : 0))
            if state.phase == "A_DEFENSE" { state.health = max(0, state.health - dt * 0.25) }
            state.isolation = min(1, max(0, state.isolation + dt * (populated ? -1.0 / 48 : 1.0 / 960)))
            if state.phase != "B_DECO" && state.location == "LAB" { state.nitrogen = 1 - (1 - state.nitrogen) * exp(-dt / 8) }
            state.elapsedHours += dt; state.phaseHours += dt; remaining -= dt
            if state.health <= 0 { state.phase = "DEAD"; state.reason = "Life support failed."; message = "The pump falls silent. Lab 04 becomes your tomb."; save(); break }
            if state.fuel < 145.5 && !lowFuelWarned { lowFuelWarned = true; message = "Diesel below 15%. Check the pump gauge." }
            if state.fuel == 0 && !stoppedWarned { stoppedWarned = true; message = "AIR SUPPLY LOST. Health is falling. Fuel is required now." }
            if state.phase == "CAMPAIGN" && state.elapsedHours >= 936 - 0.000001 { transition("CRISIS"); message = "Day 40. Mercy broadcasts: We have fuel. We need help." }
            if state.phase == "A_RENDEZVOUS" && state.phaseHours >= 6 - 0.000001 { state.fuel += 240; transition("A_DEFENSE"); message = "Mercy arrives with 240 L diesel. Hold for the evacuation sweep, or escape." }
            if state.phase == "A_DEFENSE" && state.phaseHours >= 48 - 0.000001 { transition("ENDING_A", location: "BOAT"); message = "Forty-eight hours survived. The aerial sweep guides Mercy out. The family lives." }
            if state.phase == "B_DECO" {
                if state.fuel == 0 || state.battery == 0 { transition("B_PREP"); message = "Decompression interrupted. Restore life support before restarting." }
                else { state.nitrogen = max(0, 1 - state.phaseHours / 17); if state.phaseHours >= 17 - 0.000001 { transition("B_READY"); message = "Decompression complete. The sealed capsule is ready." } }
            }
            if state.phase == "B_OUTBOUND" && state.phaseHours >= 1.5 - 0.000001 { transition("B_DOCK", location: "MARINA"); message = "Milliken marina. Keep quiet and transfer fuel for one game hour." }
        }
    }

    private func transition(_ phase: String, location: String? = nil) { state.phase = phase; state.phaseHours = 0; if let location { state.location = location } }
    private func notify(_ text: String) { if !state.terminal || state.phase.hasPrefix("ENDING") { message = text }; onChange?() }
    private func commit() { save(); onChange?() }
    func newRun() { guard state.terminal else { notify("Finish this run before starting another. There is only one live save."); return }; state = GameState(); lowFuelWarned = false; stoppedWarned = false; message = "A new shift begins. Inspect the pump gauge."; commit() }

    func save() {
        do {
            let directory = saveURL.deletingLastPathComponent()
            try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
            let data = try JSONEncoder().encode(state)
            if state.terminal {
                let archive = directory.appendingPathComponent("\(state.phase == "DEAD" ? "tomb" : "ending")-\(state.runID).json")
                if !FileManager.default.fileExists(atPath: archive.path) { try data.write(to: archive, options: .atomic) }
            }
            try data.write(to: saveURL, options: .atomic)
            persistenceError = nil
        } catch { persistenceError = error.localizedDescription; message = "SAVE ERROR: " + error.localizedDescription }
    }
    func load() {
        guard FileManager.default.fileExists(atPath: saveURL.path) else { return }
        do {
            let decoded = try JSONDecoder().decode(GameState.self, from: Data(contentsOf: saveURL))
            guard decoded.version == 1, decoded.elapsedHours.isFinite, decoded.elapsedHours >= 0, (0...5).contains(decoded.tutorialStep), decoded.health.isFinite, (0...100).contains(decoded.health) else { throw NSError(domain: "Invalid saved run", code: 1) }
            let tomb = saveURL.deletingLastPathComponent().appendingPathComponent("tomb-\(decoded.runID).json")
            state = FileManager.default.fileExists(atPath: tomb.path) ? try JSONDecoder().decode(GameState.self, from: Data(contentsOf: tomb)) : decoded
            message = state.terminal ? objective : "Welcome back. " + objective
        } catch { persistenceError = error.localizedDescription; state.phase = "DEAD"; state.reason = "Save could not be read"; message = "Save could not be read. Original file preserved. Start New Run to continue." }
    }
}
