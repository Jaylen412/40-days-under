"""Render Lab 04 props in L_OpeningSlice with a SceneCapture2D (works with -RenderOffscreen).

Writes Saved/Screenshots/lab04_<view>.png for each view below, then quits. Nothing is saved to the level.
"""
import unreal
DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()) + "Screenshots/"
world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/L_OpeningSlice")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
# (name, camera location, look-at target), UE centimetres, eye height ~165.
views = [
    ("valve", unreal.Vector(-280, 60, 160), unreal.Vector(-400, 0, 120)),
    ("cot", unreal.Vector(20, 90, 170), unreal.Vector(-195, 278, 30)),
    ("bench", unreal.Vector(40, 110, 165), unreal.Vector(187, 325, 80)),
    ("recorder", unreal.Vector(150, 225, 145), unreal.Vector(192, 293, 92)),
    ("scrubber", unreal.Vector(-20, -60, 165), unreal.Vector(-237, -303, 100)),
    ("manifold", unreal.Vector(0, -200, 165), unreal.Vector(0, -430, 150)),
    ("gauge", unreal.Vector(-30, -360, 160), unreal.Vector(-30, -415, 165)),
    ("telemetry", unreal.Vector(160, -130, 160), unreal.Vector(320, -270, 130)),
    ("lamp", unreal.Vector(250, -230, 175), unreal.Vector(299, -290, 180)),
]
ONLY = {"manifold", "gauge", "telemetry", "lamp"}  # e.g. {"manifold", "telemetry"} to render a subset
views = [v for v in views if ONLY is None or v[0] in ONLY]
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
        name, loc, target = views[i]
        if f % 150 == 1:
            cap.set_actor_location_and_rotation(loc, unreal.MathLibrary.find_look_at_rotation(loc, target), False, False)
        elif f % 150 in (60, 90, 120):
            comp.capture_scene()   # repeat so Lumen and exposure settle
        elif f % 150 == 140:
            unreal.RenderingLibrary.export_render_target(world, rt, DIR, "lab04_%s.png" % name)
            log("wrote " + name)
            state["i"] += 1
    elif f > 150 * len(views) + 30:
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()
handle = unreal.register_slate_post_tick_callback(tick)
