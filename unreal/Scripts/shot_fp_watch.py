"""Render the first-person arms and live watch from the player's eye (editor, offscreen).

Spawns an AUnderwaterCharacter in L_OpeningSlice facing the manifold, poses both arms raised, turns off
owner-only on this temporary instance so a SceneCapture2D can see them, and writes
Saved/Screenshots/fp_<view>.png. Nothing is saved to the level.
"""
import unreal
DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()) + "Screenshots/"
world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/L_OpeningSlice")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cls = unreal.load_class(None, "/Script/FortyDaysUnder.UnderwaterCharacter")
char = sub.spawn_actor_from_class(cls, unreal.Vector(0, -200, 100), unreal.Rotator(0, 0, -90))  # roll, pitch, yaw: face -Y
cam = char.get_editor_property("FirstPersonCamera")
left, right, watch = (char.get_editor_property(n) for n in ("ArmLeft", "ArmRight", "Watch"))
for c in [left, right, watch] + list(char.get_editor_property("WatchLines")):
    c.set_editor_property("only_owner_see", False)
left.set_relative_transform(char.get_editor_property("ArmLeftRaised"), False, False)
right.set_relative_transform(char.get_editor_property("ArmRightRaised"), False, False)
left.set_visibility(True, True); right.set_visibility(True, True)
char.refresh_watch()

rt = unreal.RenderingLibrary.create_render_target2d(world, 1280, 720, unreal.TextureRenderTargetFormat.RTF_RGBA8_SRGB)
cap = sub.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(0, 0, 0))
comp = cap.capture_component2d
comp.set_editor_property("texture_target", rt)
comp.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_TONE_CURVE_HDR)
views = [("eye", 90.0), ("watch", 30.0)]
state = {"frame": 0, "i": 0}
def tick(dt):
    state["frame"] += 1
    f, i = state["frame"], state["i"]
    if i < len(views):
        name, fov = views[i]
        if f % 150 == 1:
            t = cam.get_world_transform()
            loc, rot = t.translation, t.rotation.rotator()
            if name == "watch":   # aim at the watch from the eye
                rot = unreal.MathLibrary.find_look_at_rotation(loc, watch.get_world_location())
            cap.set_actor_location_and_rotation(loc, rot, False, False)
            comp.set_editor_property("fov_angle", fov)
        elif f % 150 in (60, 90, 120):
            comp.capture_scene()
        elif f % 150 == 140:
            unreal.RenderingLibrary.export_render_target(world, rt, DIR, "fp_%s.png" % name)
            unreal.log("FPSHOT wrote " + name)
            state["i"] += 1
    elif f > 150 * len(views) + 30:
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()
handle = unreal.register_slate_post_tick_callback(tick)
