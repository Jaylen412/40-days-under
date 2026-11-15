"""Import SM_Lab04_ValveWheel (Higgsfield-derived prop) and swap it onto the valve actor in L_OpeningSlice.

Run inside the editor:
  UnrealEditor <project> -EnablePlugins=PythonScriptPlugin -RenderOffscreen -unattended
      -ExecCmds="py <this file>"
The script quits the editor when done.
"""
import os
import unreal

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
REPO = os.path.abspath(os.path.join(PROJECT, ".."))
FBX = os.path.join(REPO, "blender", "export", "SM_Lab04_ValveWheel.fbx")
TEX = os.path.join(REPO, "blender", "export", "textures", "T_Lab04_ValveWheel_D.png")
DEST = "/Game/Environments/Lab04/Props/ValveWheel"
LEVEL = "/Game/Maps/L_OpeningSlice"
OLD_MESH = "/Game/Environments/Lab04/Graybox/Meshes/SM_GB_ValveWheel"
log = lambda m: unreal.log("VALVEIMPORT " + str(m))

def task(src, name, options=None):
    t = unreal.AssetImportTask()
    t.filename = src; t.destination_path = DEST; t.destination_name = name
    t.automated = True; t.replace_existing = True; t.save = True
    if options: t.options = options
    return t

try:
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    fbx = unreal.FbxImportUI()
    fbx.import_mesh = True; fbx.import_as_skeletal = False; fbx.import_materials = False; fbx.import_textures = False
    fbx.static_mesh_import_data.combine_meshes = True
    fbx.static_mesh_import_data.auto_generate_collision = False   # interaction traces hit the interactable's own box
    fbx.static_mesh_import_data.generate_lightmap_u_vs = False
    tools.import_asset_tasks([task(FBX, "SM_Lab04_ValveWheel", fbx), task(TEX, "T_Lab04_ValveWheel_D")])

    mesh = unreal.load_asset(DEST + "/SM_Lab04_ValveWheel")
    tex = unreal.load_asset(DEST + "/T_Lab04_ValveWheel_D")
    mel = unreal.MaterialEditingLibrary
    mat_path = DEST + "/M_Lab04_ValveWheel"
    if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
        unreal.EditorAssetLibrary.delete_asset(mat_path)
    mat = tools.create_asset("M_Lab04_ValveWheel", DEST, unreal.Material, unreal.MaterialFactoryNew())
    sample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 0)
    sample.texture = tex
    mel.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    rough = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -400, 250)
    rough.r = 0.65
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    mesh.set_material(0, mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    b = mesh.get_bounds()
    log("mesh bounds origin=%s extent=%s tris=%d" % (b.origin, b.box_extent, mesh.get_num_triangles(0)))

    unreal.EditorLoadingAndSavingUtils.load_map(LEVEL)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    swapped = 0
    for a in actors:
        if isinstance(a, unreal.StaticMeshActor):
            comp = a.static_mesh_component
            if comp.static_mesh and comp.static_mesh.get_path_name().startswith(OLD_MESH):
                a.modify(); comp.modify()
                comp.set_editor_property("static_mesh", mesh)
                comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
                log("swapped %s at %s rot %s" % (a.get_actor_label(), a.get_actor_location(), a.get_actor_rotation()))
                swapped += 1
    log("swapped %d actor(s)" % swapped)
    if swapped:
        ok = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
        log("level saved: %s" % ok)
except Exception as e:
    unreal.log_error("VALVEIMPORT failed: %r" % e)
finally:
    unreal.SystemLibrary.quit_editor()
