"""Turn the Higgsfield valve wheel GLB into SM_Lab04_ValveWheel (Blender 5.3, run with blender -b --python).

Matches the graybox SM_GB_ValveWheel: 60 cm diameter, pivot at the hub, spin axis on Blender +X
(UE local X, which ASliceInteractable spins around).
"""
import bpy, bmesh, os, sys, math, mathutils

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
GLB = os.path.join(ROOT, "assets", "hf_20261115_101455_8b95e6b0-e291-400c-973c-5506287048a4.glb")
BLEND = os.path.join(ROOT, "blender", "props", "SM_Lab04_ValveWheel.blend")
FBX = os.path.join(ROOT, "blender", "export", "SM_Lab04_ValveWheel.fbx")
TEX = os.path.join(ROOT, "blender", "export", "textures", "T_Lab04_ValveWheel_D.png")
DIAMETER_M = 0.60

bpy.ops.wm.read_factory_settings(use_empty=True)
sc = bpy.context.scene
sc.unit_settings.system = 'METRIC'; sc.unit_settings.scale_length = 1.0
bpy.ops.import_scene.gltf(filepath=GLB)
obj = next(o for o in sc.objects if o.type == 'MESH')
for o in list(sc.objects):
    if o is not obj:
        bpy.data.objects.remove(o)
obj.parent = None
obj.name = obj.data.name = "SM_Lab04_ValveWheel"

# Bake world transform into the mesh, then centre on the bounding-box centre (the hub).
obj.data.transform(obj.matrix_world); obj.matrix_world = mathutils.Matrix.Identity(4)
vs = [v.co for v in obj.data.vertices]
lo = mathutils.Vector([min(v[i] for v in vs) for i in range(3)])
hi = mathutils.Vector([max(v[i] for v in vs) for i in range(3)])
size = hi - lo
axis = min(range(3), key=lambda i: size[i])          # thinnest axis = spin axis
obj.data.transform(mathutils.Matrix.Translation(-(lo + hi) / 2))
rim = max(size[i] for i in range(3) if i != axis)
obj.data.transform(mathutils.Matrix.Scale(DIAMETER_M / rim, 4))
# Put the spin axis on +X.
to_x = {0: mathutils.Matrix.Identity(4), 1: mathutils.Matrix.Rotation(math.radians(-90), 4, 'Z'),
        2: mathutils.Matrix.Rotation(math.radians(90), 4, 'Y')}[axis]
obj.data.transform(to_x)

# Hygiene: merge doubles, recalc normals, triangulate.
bm = bmesh.new(); bm.from_mesh(obj.data)
bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-5)
bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
bmesh.ops.triangulate(bm, faces=bm.faces)
bm.to_mesh(obj.data); bm.free()
for p in obj.data.polygons:
    p.use_smooth = True

# Material and texture: one slot, texture saved beside the FBX (not embedded).
mat = obj.material_slots[0].material
mat.name = "M_Lab04_ValveWheel"
img = next(n.image for n in mat.node_tree.nodes if n.type == 'TEX_IMAGE' and n.image)
img.name = "T_Lab04_ValveWheel_D"
img.filepath_raw = TEX; img.file_format = 'PNG'; img.save()

os.makedirs(os.path.dirname(BLEND), exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=BLEND)
bpy.ops.object.select_all(action='DESELECT'); obj.select_set(True); bpy.context.view_layer.objects.active = obj
bpy.ops.export_scene.fbx(filepath=FBX, use_selection=True, object_types={'MESH'}, global_scale=1.0,
                         apply_scale_options='FBX_SCALE_UNITS', apply_unit_scale=True, use_space_transform=True,
                         bake_space_transform=True, mesh_smooth_type='FACE', use_mesh_modifiers=True,
                         use_triangles=True, add_leaf_bones=False, bake_anim=False, path_mode='AUTO')
vs = [v.co for v in obj.data.vertices]
print("VALVE", len(obj.data.vertices), "verts", len(obj.data.polygons), "tris",
      [round(min(v[i] for v in vs), 3) for i in range(3)], [round(max(v[i] for v in vs), 3) for i in range(3)], "axis was", "XYZ"[axis])
