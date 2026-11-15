"""Import the Lab 04 props, reimport the graybox static set without the blocks they replace, and place the
props in L_OpeningSlice and L_OpeningSlice_Preview.

Inputs: blender/props/build_lab04_props.py (Higgsfield-derived props: FBX, textures, lab04_props_placement.json)
and blender/props/build_lab04_handmade.py (manifold, telemetry rack, dive watch: FBX, lab04_handmade_placement.json).
Hand-modelled meshes use the shared materials defined in HANDMADE_MATERIALS below.
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
HANDMADE = os.path.join(REPO, "blender", "props", "lab04_handmade_placement.json")
MATERIALS = "/Game/Environments/Lab04/Materials"
WATCH_DEST = "/Game/Characters/Christian/Watch"
# name: (base colour, roughness, metallic, max grime/rust blend 0-1, emissive colour or None)
HANDMADE_MATERIALS = {
    "M_Lab04_PaintedSteel": ((0.20, 0.25, 0.22), 0.62, 0.15, 0.55, None),
    "M_Lab04_BareSteel": ((0.50, 0.50, 0.50), 0.42, 1.0, 0.35, None),
    "M_Lab04_PanelGrey": ((0.28, 0.29, 0.28), 0.55, 0.3, 0.3, None),
    "M_Lab04_Vent": ((0.025, 0.025, 0.025), 0.8, 0.0, 0.0, None),
    "M_Lab04_GaugeFace": ((0.86, 0.82, 0.70), 0.45, 0.0, 0.0, None),
    "M_Lab04_GaugeInk": ((0.02, 0.02, 0.02), 0.6, 0.0, 0.0, None),
    "M_Lab04_GaugeRed": ((0.60, 0.04, 0.03), 0.5, 0.0, 0.0, None),
    "M_Lab04_Brass": ((0.78, 0.58, 0.28), 0.35, 1.0, 0.0, None),
    "M_Lab04_ValvePaint": ((0.78, 0.55, 0.04), 0.5, 0.0, 0.45, None),
    "M_Lab04_LedGreen": ((0.05, 0.4, 0.1), 0.3, 0.0, 0.0, (0.2, 3.0, 0.5)),
    "M_Lab04_LedAmber": ((0.4, 0.2, 0.02), 0.3, 0.0, 0.0, (3.0, 1.2, 0.1)),
    "M_Lab04_Rubber": ((0.02, 0.02, 0.02), 0.9, 0.0, 0.0, None),
    "M_Lab04_WatchBezel": ((0.02, 0.025, 0.05), 0.3, 0.5, 0.0, None),
    "M_Lab04_WatchScreen": ((0.01, 0.03, 0.025), 0.15, 0.0, 0.0, (0.0, 0.05, 0.04)),  # placeholder for the live screen
}
RUST = (0.10, 0.045, 0.02)
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


def handmade_material(name, base, rough, metal, rust, emissive):
    """Constant PBR material; rust breaks up the base colour with world-space noise (no UVs needed)."""
    path = "%s/%s" % (MATERIALS, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    m = tools.create_asset(name, MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    def const3(c, x, y):
        e = mel.create_material_expression(m, unreal.MaterialExpressionConstant3Vector, x, y)
        e.constant = unreal.LinearColor(c[0], c[1], c[2], 1.0)
        return e
    def const(v, x, y):
        e = mel.create_material_expression(m, unreal.MaterialExpressionConstant, x, y)
        e.r = v
        return e
    colour = const3(base, -600, 0)
    if rust > 0:
        noise = mel.create_material_expression(m, unreal.MaterialExpressionNoise, -900, 200)
        # Soft grime: alpha varies smoothly between 0 and the rust amount, never hard spots.
        noise.set_editor_property("scale", 0.03)
        noise.set_editor_property("levels", 4)
        noise.set_editor_property("output_min", 0.0)
        noise.set_editor_property("output_max", rust)
        sat = mel.create_material_expression(m, unreal.MaterialExpressionSaturate, -700, 200)
        mel.connect_material_expressions(noise, "", sat, "")
        lerp = mel.create_material_expression(m, unreal.MaterialExpressionLinearInterpolate, -400, 0)
        mel.connect_material_expressions(colour, "", lerp, "A")
        mel.connect_material_expressions(const3(RUST, -600, 120), "", lerp, "B")
        mel.connect_material_expressions(sat, "", lerp, "Alpha")
        mel.connect_material_property(lerp, "", unreal.MaterialProperty.MP_BASE_COLOR)
        # Rust is rough and not metallic.
        rl = mel.create_material_expression(m, unreal.MaterialExpressionLinearInterpolate, -400, 300)
        mel.connect_material_expressions(const(rough, -600, 300), "", rl, "A")
        mel.connect_material_expressions(const(0.9, -600, 360), "", rl, "B")
        mel.connect_material_expressions(sat, "", rl, "Alpha")
        mel.connect_material_property(rl, "", unreal.MaterialProperty.MP_ROUGHNESS)
        ml = mel.create_material_expression(m, unreal.MaterialExpressionLinearInterpolate, -400, 450)
        mel.connect_material_expressions(const(metal, -600, 450), "", ml, "A")
        mel.connect_material_expressions(const(0.0, -600, 510), "", ml, "B")
        mel.connect_material_expressions(sat, "", ml, "Alpha")
        mel.connect_material_property(ml, "", unreal.MaterialProperty.MP_METALLIC)
    else:
        mel.connect_material_property(colour, "", unreal.MaterialProperty.MP_BASE_COLOR)
        mel.connect_material_property(const(rough, -600, 300), "", unreal.MaterialProperty.MP_ROUGHNESS)
        mel.connect_material_property(const(metal, -600, 450), "", unreal.MaterialProperty.MP_METALLIC)
    if emissive:
        mel.connect_material_property(const3(emissive, -600, 600), "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(m)
    unreal.EditorAssetLibrary.save_loaded_asset(m)
    return m


try:
    with open(PLACEMENT) as f:
        placement = json.load(f)
    with open(HANDMADE) as f:
        handmade = json.load(f)

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

    # 1b. Hand-modelled meshes with shared materials. The watch is imported but not placed (no first-person arms yet).
    hm_mats = {n: handmade_material(n, *v) for n, v in HANDMADE_MATERIALS.items()}
    for mesh_name, p in handmade.items():
        prop = mesh_name.replace("SM_Lab04_", "")
        dest = WATCH_DEST if p["location_cm"] is None else "%s/%s" % (PROPS_ROOT, prop)
        tools.import_asset_tasks([import_task(os.path.join(EXPORT, mesh_name + ".fbx"), dest, mesh_name,
                                              fbx_options(p["location_cm"] is not None))])
        mesh = unreal.load_asset("%s/%s" % (dest, mesh_name))
        for i, slot in enumerate(mesh.get_editor_property("static_materials")):
            m = hm_mats.get(str(slot.material_slot_name))
            if m:
                mesh.set_material(i, m)
            else:
                unreal.log_warning("PROPIMPORT %s: no material for slot %s" % (mesh_name, slot.material_slot_name))
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        b = mesh.get_bounds()
        log("%s extent=(%.1f, %.1f, %.1f) tris=%d" % (mesh_name, b.box_extent.x, b.box_extent.y, b.box_extent.z, mesh.get_num_triangles(0)))
        if p["location_cm"] is not None:
            meshes[mesh_name] = mesh
            placement[mesh_name] = p

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
