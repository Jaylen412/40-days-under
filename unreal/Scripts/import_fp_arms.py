"""Import the first-person forearms (build_fp_arms.py) and reimport the dive watch (build_lab04_handmade.py).

Run inside the editor:
  UnrealEditor <project> -EnablePlugins=PythonScriptPlugin -RenderOffscreen -unattended
      -ExecCmds="py <this file>"
The watch reuses the Lab 04 materials made by import_lab04_props.py (run that once first).
"""
import os
import unreal

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
EXPORT = os.path.abspath(os.path.join(PROJECT, "..", "blender", "export"))
ARMS = "/Game/Characters/Christian/FirstPerson"
WATCH = "/Game/Characters/Christian/Watch"
LAB_MATERIALS = "/Game/Environments/Lab04/Materials"
# name: (base colour, roughness, metallic[, emissive])
ARM_MATERIALS = {
    "M_FP_Sleeve": ((0.035, 0.045, 0.08), 0.85, 0.0),   # navy engineering jacket
    "M_FP_Cuff": ((0.02, 0.02, 0.025), 0.7, 0.0),
    "M_FP_Skin": ((0.17, 0.085, 0.05), 0.55, 0.0),      # rich brown skin
    "M_FP_Strap": ((0.015, 0.015, 0.015), 0.8, 0.0),    # rubber watch strap
    "M_FP_WatchScreen": ((0.005, 0.02, 0.016), 0.7, 0.0, (0.0, 0.035, 0.028)),  # matte, faint glow; text drawn on top
}
WATCH_SLOT_OVERRIDES = {"M_Lab04_WatchScreen": "M_FP_WatchScreen"}
log = lambda m: unreal.log("FPARMS " + str(m))
tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary


def import_mesh(name, dest):
    o = unreal.FbxImportUI()
    o.import_mesh = True; o.import_as_skeletal = False; o.import_materials = False; o.import_textures = False
    o.static_mesh_import_data.combine_meshes = True
    o.static_mesh_import_data.auto_generate_collision = False
    o.static_mesh_import_data.generate_lightmap_u_vs = False
    t = unreal.AssetImportTask()
    t.filename = os.path.join(EXPORT, name + ".fbx"); t.destination_path = dest; t.destination_name = name
    t.automated = True; t.replace_existing = True; t.save = True; t.options = o
    tools.import_asset_tasks([t])
    return unreal.load_asset("%s/%s" % (dest, name))


def constant_material(name, base, rough, metal, emissive=None):
    path = "%s/%s" % (ARMS, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    m = tools.create_asset(name, ARMS, unreal.Material, unreal.MaterialFactoryNew())
    c = mel.create_material_expression(m, unreal.MaterialExpressionConstant3Vector, -400, 0)
    c.constant = unreal.LinearColor(base[0], base[1], base[2], 1.0)
    mel.connect_material_property(c, "", unreal.MaterialProperty.MP_BASE_COLOR)
    for value, prop, y in ((rough, unreal.MaterialProperty.MP_ROUGHNESS, 200), (metal, unreal.MaterialProperty.MP_METALLIC, 300)):
        e = mel.create_material_expression(m, unreal.MaterialExpressionConstant, -400, y)
        e.r = value
        mel.connect_material_property(e, "", prop)
    if emissive:
        e = mel.create_material_expression(m, unreal.MaterialExpressionConstant3Vector, -400, 400)
        e.constant = unreal.LinearColor(emissive[0], emissive[1], emissive[2], 1.0)
        mel.connect_material_property(e, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(m)
    unreal.EditorAssetLibrary.save_loaded_asset(m)
    return m


def assign(mesh, lookup):
    for i, slot in enumerate(mesh.get_editor_property("static_materials")):
        m = lookup(str(slot.material_slot_name))
        if m:
            mesh.set_material(i, m)
        else:
            unreal.log_warning("FPARMS %s: no material for slot %s" % (mesh.get_name(), slot.material_slot_name))
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    b = mesh.get_bounds()
    log("%s extent=(%.1f, %.1f, %.1f) tris=%d" % (mesh.get_name(), b.box_extent.x, b.box_extent.y, b.box_extent.z, mesh.get_num_triangles(0)))


try:
    mats = {n: constant_material(n, *v) for n, v in ARM_MATERIALS.items()}
    for side in ("L", "R"):
        assign(import_mesh("SM_FP_Forearm_" + side, ARMS), mats.get)
    assign(import_mesh("SM_Lab04_DiveWatch", WATCH),
           lambda n: mats[WATCH_SLOT_OVERRIDES[n]] if n in WATCH_SLOT_OVERRIDES else unreal.load_asset("%s/%s" % (LAB_MATERIALS, n)))
except Exception as e:
    unreal.log_error("FPARMS failed: %r" % e)
finally:
    unreal.SystemLibrary.quit_editor()
