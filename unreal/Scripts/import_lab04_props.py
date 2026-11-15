"""Import the Higgsfield-derived Lab 04 props, reimport the graybox static set without the blocks they
replace, and place the props in L_OpeningSlice and L_OpeningSlice_Preview.

Inputs come from blender/props/build_lab04_props.py (FBX, textures, lab04_props_placement.json).
Run inside the editor:
  UnrealEditor <project> -EnablePlugins=PythonScriptPlugin -RenderOffscreen -unattended
      -ExecCmds="py <this file>"
Re-running is safe: assets are replaced and existing PROP_ actors are updated, not duplicated.
"""
import json
import os
import unreal

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
REPO = os.path.abspath(os.path.join(PROJECT, ".."))
EXPORT = os.path.join(REPO, "blender", "export")
PLACEMENT = os.path.join(REPO, "blender", "props", "lab04_props_placement.json")
PROPS_ROOT = "/Game/Environments/Lab04/Props"
GRAYBOX = "/Game/Environments/Lab04/Graybox"
LEVELS = ["/Game/Maps/L_OpeningSlice", "/Game/Maps/L_OpeningSlice_Preview"]
NO_COLLISION = {"SM_Lab04_LogRecorder"}  # the interactable's own box must win the view trace
log = lambda m: unreal.log("PROPIMPORT " + str(m))
tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary


def import_task(src, dest, name, options=None):
    t = unreal.AssetImportTask()
    t.filename = src; t.destination_path = dest; t.destination_name = name
    t.automated = True; t.replace_existing = True; t.save = True
    if options:
        t.options = options
    return t


def fbx_options(collision):
    o = unreal.FbxImportUI()
    o.import_mesh = True; o.import_as_skeletal = False; o.import_materials = False; o.import_textures = False
    o.static_mesh_import_data.combine_meshes = True
    o.static_mesh_import_data.auto_generate_collision = collision
    o.static_mesh_import_data.generate_lightmap_u_vs = False
    return o


def textured_material(dest, name, tex, roughness):
    path = "%s/%s" % (dest, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    mat = tools.create_asset(name, dest, unreal.Material, unreal.MaterialFactoryNew())
    sample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 0)
    sample.texture = tex
    mel.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    rough = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -400, 250)
    rough.r = roughness
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    return mat


try:
    with open(PLACEMENT) as f:
        placement = json.load(f)

    # 1. Props.
    meshes = {}
    for mesh_name in placement:
        prop = mesh_name.replace("SM_Lab04_", "")
        dest = "%s/%s" % (PROPS_ROOT, prop)
        tex_name = "T_Lab04_%s_D" % prop
        tools.import_asset_tasks([
            import_task(os.path.join(EXPORT, mesh_name + ".fbx"), dest, mesh_name, fbx_options(mesh_name not in NO_COLLISION)),
            import_task(os.path.join(EXPORT, "textures", tex_name + ".png"), dest, tex_name)])
        mesh = unreal.load_asset("%s/%s" % (dest, mesh_name))
        mat = textured_material(dest, "M_Lab04_" + prop, unreal.load_asset("%s/%s" % (dest, tex_name)), 0.7)
        mesh.set_material(0, mat)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        b = mesh.get_bounds()
        log("%s extent=(%.1f, %.1f, %.1f) tris=%d" % (mesh_name, b.box_extent.x, b.box_extent.y, b.box_extent.z, mesh.get_num_triangles(0)))
        meshes[mesh_name] = mesh

    # 2. Graybox static set without the replaced blocks. Slots GB_X keep material M_GB_X; collision stays complex-as-simple.
    static_path = GRAYBOX + "/Meshes/SM_Lab04_Graybox_Static"
    tools.import_asset_tasks([import_task(os.path.join(EXPORT, "SM_Lab04_Graybox_Static.fbx"), GRAYBOX + "/Meshes",
                                          "SM_Lab04_Graybox_Static", fbx_options(False))])
    static = unreal.load_asset(static_path)
    slots = static.get_editor_property("static_materials")
    for i, slot in enumerate(slots):
        slot_name = str(slot.material_slot_name)
        mat = unreal.load_asset("%s/Materials/M_%s" % (GRAYBOX, slot_name))
        if mat:
            static.set_material(i, mat)
        else:
            unreal.log_warning("PROPIMPORT no material for slot " + slot_name)
    body = static.get_editor_property("body_setup")
    body.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    unreal.EditorAssetLibrary.save_loaded_asset(static)
    log("static slots=%s tris=%d" % ([str(s.material_slot_name) for s in static.get_editor_property("static_materials")],
                                     static.get_num_triangles(0)))

    # 3. Place props in both levels.
    actors_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    level_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    for level in LEVELS:
        unreal.EditorLoadingAndSavingUtils.load_map(level)
        existing = {a.get_actor_label(): a for a in actors_sub.get_all_level_actors()}
        for mesh_name, p in placement.items():
            label = mesh_name.replace("SM_Lab04_", "PROP_")
            loc = unreal.Vector(*p["location_cm"])
            rot = unreal.Rotator(0.0, 0.0, p["yaw_deg"])  # roll, pitch, yaw
            actor = existing.get(label)
            if actor is None:
                actor = actors_sub.spawn_actor_from_class(unreal.StaticMeshActor, loc, rot)
                actor.set_actor_label(label)
            actor.modify()
            actor.set_actor_location_and_rotation(loc, rot, False, False)
            comp = actor.static_mesh_component
            comp.modify()
            comp.set_editor_property("static_mesh", meshes[mesh_name])
            if mesh_name in NO_COLLISION:
                comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
            log("%s %s at %s yaw %.1f" % (level, label, p["location_cm"], p["yaw_deg"]))
        log("%s saved: %s" % (level, level_sub.save_current_level()))
except Exception as e:
    unreal.log_error("PROPIMPORT failed: %r" % e)
finally:
    unreal.SystemLibrary.quit_editor()
