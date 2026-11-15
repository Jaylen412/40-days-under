"""Build the Higgsfield-derived Lab 04 props and a graybox static set without the blocks they replace.

Run: blender -b blender/starting-animation.blend --python blender/props/build_lab04_props.py

SAM 3D returns each object upright, axis-aligned, front facing -Y, long side on X, about 1 m across.
Each prop gets its pivot at the bottom centre and is scaled to its real size. The script writes:
  blender/export/SM_Lab04_<Prop>.fbx + textures/T_Lab04_<Prop>_D.png   (git-ignored, rebuilt by this script)
  blender/export/SM_Lab04_Graybox_Static.fbx                            (graybox minus replaced blocks)
  blender/props/lab04_props_placement.json                              (UE transforms, read by the import script)
The .blend itself is not modified or saved.
"""
import bpy, bmesh, json, math, os
import mathutils

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
EXPORT = os.path.join(ROOT, "blender", "export")
ASSETS = os.path.join(ROOT, "assets")

# name: (glb, (size along X, Y, Z) in metres: all three = per-axis fit, one = uniform scale from that axis,
#        graybox object it replaces, yaw offset from that object's yaw in degrees)
PROPS = {
    "Cot": ("hf_20261115_103738_663c65ea-89af-4a18-b137-7bf88a46f90c.glb", (1.95, 0.85, 0.55), "GB_Lab04_Cot", 0.0),
    "Bench": ("hf_20261115_103848_0a318843-8920-485d-a584-ae934a37b88d.glb", (2.0, 0.80, 0.90), "GB_Lab04_Bench_Lab", -90.0),
    "LogRecorder": ("hf_20261115_103741_5c08aef4-2ef6-47fa-b2d0-5b59f8f13e08.glb", (None, 0.14, None), "GB_LogRecorder", 0.0),
    "AirScrubber": ("hf_20261115_103743_356b245d-ec8f-49e7-a192-60a956b702ca.glb", (None, None, 2.2), "GB_Lab04_AirScrubber_Unit", -90.0),
}
# Graybox objects removed from the static set (the scrubber's pipe goes with its unit).
REMOVED = {"GB_Lab04_Cot", "GB_Lab04_Bench_Lab", "GB_LogRecorder", "GB_Lab04_AirScrubber_Unit", "GB_Lab04_Scrubber_Pipe"}
# Exported separately as movable actors (unchanged), never part of the static set.
MOVABLE = {"GB_Lab04_Intake_Valve_Wheel", "GB_Lab04_Intake_Valve_Spoke", "GB_Telemetry_FaultLamp_HydroCh3"}
MOVABLE_PREFIX = ("GB_Gauge_Needle_",)

FBX_ARGS = dict(use_selection=True, object_types={'MESH'}, global_scale=1.0, apply_scale_options='FBX_SCALE_UNITS',
                apply_unit_scale=True, use_space_transform=True, bake_space_transform=True, mesh_smooth_type='FACE',
                use_mesh_modifiers=True, use_triangles=True, add_leaf_bones=False, bake_anim=False, path_mode='AUTO')


def bounds(mesh):
    vs = [v.co for v in mesh.vertices]
    return (mathutils.Vector([min(v[i] for v in vs) for i in range(3)]),
            mathutils.Vector([max(v[i] for v in vs) for i in range(3)]))


def select_only(objs):
    bpy.ops.object.select_all(action='DESELECT')
    for o in objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]


def to_ue(loc_m, yaw_deg):
    """Blender metres / yaw to UE centimetres / yaw (Y flips, DEC-009)."""
    return [round(loc_m[0] * 100, 2), round(-loc_m[1] * 100, 2), round(loc_m[2] * 100, 2)], round(-yaw_deg, 2)


scene = bpy.context.scene
placement = {}
os.makedirs(os.path.join(EXPORT, "textures"), exist_ok=True)

for name, (glb, size, gb_name, yaw_offset) in PROPS.items():
    gb = bpy.data.objects[gb_name]
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=os.path.join(ASSETS, glb))
    new = [o for o in bpy.data.objects if o not in before]
    obj = next(o for o in new if o.type == 'MESH')
    for o in new:
        if o is not obj:
            bpy.data.objects.remove(o)
    obj.parent = None
    mesh_name = "SM_Lab04_" + name
    obj.name = obj.data.name = mesh_name
    obj.data.transform(obj.matrix_world)
    obj.matrix_world = mathutils.Matrix.Identity(4)

    lo, hi = bounds(obj.data)
    dims = hi - lo
    # Pivot: bottom centre.
    obj.data.transform(mathutils.Matrix.Translation(-mathutils.Vector(((lo.x + hi.x) / 2, (lo.y + hi.y) / 2, lo.z))))
    # Scale: explicit per-axis targets where given; otherwise uniform from the given axis.
    given = [(i, s) for i, s in enumerate(size) if s]
    if len(given) == 3:
        sx, sy, sz = (size[i] / dims[i] for i in range(3))
    else:
        i, s = given[0]
        sx = sy = sz = s / dims[i]
    obj.data.transform(mathutils.Matrix.Diagonal((sx, sy, sz, 1.0)))

    bm = bmesh.new(); bm.from_mesh(obj.data)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-6)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(obj.data); bm.free()
    for p in obj.data.polygons:
        p.use_smooth = True

    mat = obj.material_slots[0].material
    mat.name = "M_Lab04_" + name
    img = next(n.image for n in mat.node_tree.nodes if n.type == 'TEX_IMAGE' and n.image)
    img.name = "T_Lab04_%s_D" % name
    img.filepath_raw = os.path.join(EXPORT, "textures", img.name + ".png"); img.file_format = 'PNG'; img.save()

    select_only([obj])
    bpy.ops.export_scene.fbx(filepath=os.path.join(EXPORT, mesh_name + ".fbx"), **FBX_ARGS)

    # Place at the graybox block: same XY centre and yaw (plus offset), standing on its base.
    gw = gb.matrix_world
    centre = gw.translation.copy()
    base_z = min((gw @ mathutils.Vector(c)).z for c in gb.bound_box)
    yaw = math.degrees(gw.to_euler().z) + yaw_offset
    ue_loc, ue_yaw = to_ue((centre.x, centre.y, base_z), yaw)
    lo, hi = bounds(obj.data)
    placement[mesh_name] = {"location_cm": ue_loc, "yaw_deg": ue_yaw, "replaces": gb_name,
                            "size_m": [round(v, 3) for v in (hi - lo)], "tris": len(obj.data.polygons)}
    print("PROP", mesh_name, placement[mesh_name])
    bpy.data.objects.remove(obj)

# Graybox static set without the replaced blocks.
gb_col = bpy.data.collections["SET_Lab04_Graybox"]
def walk(c):
    yield from c.objects
    for ch in c.children:
        yield from walk(ch)
keep = [o for o in walk(gb_col) if o.type == 'MESH' and o.name not in REMOVED and o.name not in MOVABLE
        and not o.name.startswith(MOVABLE_PREFIX)]
copies = []
for o in keep:
    c = o.copy(); c.data = o.data.copy(); c.parent = None; c.matrix_world = o.matrix_world.copy()
    scene.collection.objects.link(c)
    c.data.transform(c.matrix_world); c.matrix_world = mathutils.Matrix.Identity(4)
    c.modifiers.clear()
    copies.append(c)
select_only(copies)
bpy.ops.object.join()
static = bpy.context.view_layer.objects.active
static.name = static.data.name = "SM_Lab04_Graybox_Static"
lo, hi = bounds(static.data)
print("STATIC", len(keep), "objects", len(static.data.vertices), "verts", [s.material.name for s in static.material_slots],
      "bounds", [round(v, 3) for v in lo], [round(v, 3) for v in hi])
select_only([static])
bpy.ops.export_scene.fbx(filepath=os.path.join(EXPORT, "SM_Lab04_Graybox_Static.fbx"), **FBX_ARGS)

with open(os.path.join(ROOT, "blender", "props", "lab04_props_placement.json"), "w") as f:
    json.dump(placement, f, indent=2)
    f.write("\n")
