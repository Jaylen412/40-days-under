import Foundation
@main struct ModelTests {
 static func main() {
  let base = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
  func game(_ name: String) -> GameModel { GameModel(saveURL: base.appendingPathComponent(name).appendingPathComponent("run.json")) }
  func crisis(_ g: GameModel) { for a in ["inspect","repair","firmware","drone","log"] { g.interact(station:a) }; for _ in 0..<39 { g.interact(station:"rest") }; assert(g.state.phase == "CRISIS") }
  let a=game("a"); crisis(a); a.choose(branch:"A"); a.interact(station:"rest"); assert(a.state.phase == "A_DEFENSE"); for _ in 0..<8 { a.interact(station:"defend") }; assert(a.state.phase == "ENDING_A")
  let b=game("b"); crisis(b); b.choose(branch:"B"); b.interact(station:"prepare"); b.interact(station:"deco"); b.interact(station:"rest"); assert(b.state.phase == "B_READY"); b.interact(station:"depart"); for _ in 0..<3 { b.interact(station:"rest") }; assert(b.state.phase == "B_DOCK"); b.interact(station:"rest"); b.interact(station:"secureFuel"); for _ in 0..<3 { b.interact(station:"rest") }; b.interact(station:"return"); assert(b.state.phase == "ENDING_B"); assert(b.state.fuel > 65)
  let death=game("death"); death.state.phase="CAMPAIGN"; death.state.fuel=0; death.interact(station:"rest"); assert(death.state.phase == "DEAD"); let id=death.state.runID; death.load(); assert(death.state.phase == "DEAD"); death.newRun(); assert(death.state.runID != id)
  let paused=game("pause"); paused.tick(delta:.nan); assert(paused.state.elapsedHours == 0); paused.interact(station:"log"); assert(paused.state.tutorialStep == 0)
  try? FileManager.default.removeItem(at:base)
  print("Mac model: both endings, tutorial gates, death persistence, new run and invalid delta passed")
 }
}
