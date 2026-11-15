"""Render the intake valve in L_OpeningSlice with a SceneCapture2D (works with -RenderOffscreen).

Writes Saved/Screenshots/valve_check_<view>.png, then quits. Nothing is saved to the level.
"""
import unreal
DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()) + "Screenshots/"
world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/L_OpeningSlice")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
target = unreal.Vector(-400, 0, 120)
views = [("near", unreal.Vector(-280, 60, 160)), ("wide", unreal.Vector(-40, 180, 175))]
rt = unreal.RenderingLibrary.create_render_target2d(world, 1280, 720, unreal.TextureRenderTargetFormat.RTF_RGBA8_SRGB)
cap = sub.spawn_actor_from_class(unreal.SceneCapture2D, views[0][1])
comp = cap.capture_component2d
comp.set_editor_property("texture_target", rt)
comp.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_TONE_CURVE_HDR)
comp.set_editor_property("fov_angle", 70.0)
state = {"frame": 0, "i": 0}
log = lambda m: unreal.log("VALVESHOT " + m)
def tick(dt):
    state["frame"] += 1
    f, i = state["frame"], state["i"]
    if i < len(views):
        name, loc = views[i]
        if f % 150 == 1:
            cap.set_actor_location_and_rotation(loc, unreal.MathLibrary.find_look_at_rotation(loc, target), False, False)
        elif f % 150 in (60, 90, 120):
            comp.capture_scene()   # repeat so Lumen and exposure settle
        elif f % 150 == 140:
            unreal.RenderingLibrary.export_render_target(world, rt, DIR, "valve_check_%s.png" % name)
            log("wrote " + name)
            state["i"] += 1
    elif f > 150 * len(views) + 30:
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()
handle = unreal.register_slate_post_tick_callback(tick)
