"""Hand-modelled Lab 04 props that have live parts (DEC-013): the manifold, the telemetry rack and the dive watch.

Run: blender -b --factory-startup --python blender/props/build_lab04_handmade.py

The meshes are built around the live actors that stay in the level:
  manifold   four needle actors pivot at x = -0.9/-0.3/0.3/0.9 m, z 1.65 m and sit at y 4.132-4.148 m;
             gauge faces sit at y 4.15 m and nothing else is in front of a face inside radius 0.17 m.
  telemetry  the fault lamp actors (12 cm spheres) sit at local (-0.15, 0.30, 1.80) m, front = local -X;
             the rack gives them a caged housing. The bus tray is modelled here (it never moved).
  watch      not placed: the player has no first-person arms yet. Pivot at the underside of the case,
             face up (+Z), 12 o'clock on +Y. The screen is its own slot (M_Lab04_WatchScreen) for a live material.
Materials are slot names only; import_lab04_props.py builds them in UE.

Writes blender/export/SM_Lab04_<Name>.fbx (git-ignored) and blender/props/lab04_handmade_placement.json.
"""
import bpy, bmesh, json, math, os
import mathutils
from mathutils import Matrix, Vector

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
EXPORT = os.path.join(ROOT, "blender", "export")
FBX_ARGS = dict(use_selection=True, object_types={'MESH'}, global_scale=1.0, apply_scale_options='FBX_SCALE_UNITS',
                apply_unit_scale=True, use_space_transform=True, bake_space_transform=True, mesh_smooth_type='FACE',
                use_mesh_modifiers=True, use_triangles=True, add_leaf_bones=False, bake_anim=False, path_mode='AUTO')

bpy.ops.wm.read_factory_settings(use_empty=True)
MATS = {}
def mat(name):
    if name not in MATS:
        MATS[name] = bpy.data.materials.new(name)
    return MATS[name]

PARTS = []
def finish(obj, material, smooth, bevel=0.0):
    obj.data.materials.clear(); obj.data.materials.append(mat(material))
    for p in obj.data.polygons:
        p.use_smooth = smooth
    if bevel:
        m = obj.modifiers.new("Bevel", 'BEVEL'); m.width = bevel; m.segments = 2; m.limit_method = 'ANGLE'
        m.harden_normals = False
    PARTS.append(obj)
    return obj

def box(material, centre, size, rot_z=0.0, bevel=0.004):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=centre, rotation=(0, 0, math.radians(rot_z)))
    o = bpy.context.active_object; o.scale = size
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    return finish(o, material, False, bevel)

def cyl(material, centre, radius, depth, axis='Z', verts=32, cap=True):
    rot = {'X': (0, math.radians(90), 0), 'Y': (math.radians(90), 0, 0), 'Z': (0, 0, 0)}[axis]
    bpy.ops.mesh.primitive_cylinder_add(vertices=verts, radius=radius, depth=depth, location=centre, rotation=rot,
                                        end_fill_type='NGON' if cap else 'NOTHING')
    return finish(bpy.context.active_object, material, True)

def torus(material, centre, major, minor, axis='Y', seg=40, minor_seg=10):
    rot = {'X': (0, math.radians(90), 0), 'Y': (math.radians(90), 0, 0), 'Z': (0, 0, 0)}[axis]
    bpy.ops.mesh.primitive_torus_add(major_radius=major, minor_radius=minor, major_segments=seg, minor_segments=minor_seg,
                                     location=centre, rotation=rot)
    return finish(bpy.context.active_object, material, True)

def pipe_between(material, a, b, radius, verts=20):
    a, b = Vector(a), Vector(b)
    d = b - a
    bpy.ops.mesh.primitive_cylinder_add(vertices=verts, radius=radius, depth=d.length, location=(a + b) / 2)
    o = bpy.context.active_object
    o.rotation_mode = 'QUATERNION'; o.rotation_quaternion = d.to_track_quat('Z', 'Y')
    return finish(o, material, True)

def handwheel(centre, radius, spokes=4, paint="M_Lab04_ValvePaint"):
    """Small handwheel facing -Y at centre."""
    torus(paint, centre, radius, radius * 0.13, 'Y', 32, 8)
    for k in range(spokes):
        a = math.radians(45 + k * 360 / spokes)
        tip = Vector(centre) + Vector((math.cos(a) * radius, 0, math.sin(a) * radius))
        pipe_between(paint, centre, tip, radius * 0.07, 8)
    cyl("M_Lab04_BareSteel", centre, radius * 0.22, radius * 0.35, 'Y', 16)

def build(name, export=True):
    """Join PARTS into one object called name, apply modifiers, export, clear PARTS."""
    bpy.ops.object.select_all(action='DESELECT')
    for o in PARTS:
        o.select_set(True)
    bpy.context.view_layer.objects.active = PARTS[0]
    for o in PARTS:
        bpy.context.view_layer.objects.active = o
        for m in list(o.modifiers):
            bpy.ops.object.modifier_apply(modifier=m.name)
    bpy.context.view_layer.objects.active = PARTS[0]
    bpy.ops.object.join()
    o = bpy.context.active_object
    o.name = o.data.name = name
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)   # pivot = local frame origin
    bm = bmesh.new(); bm.from_mesh(o.data)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(o.data); bm.free()
    bpy.ops.object.select_all(action='DESELECT'); o.select_set(True); bpy.context.view_layer.objects.active = o
    bpy.ops.export_scene.fbx(filepath=os.path.join(EXPORT, name + ".fbx"), **FBX_ARGS)
    vs = [v.co for v in o.data.vertices]
    lo = [round(min(v[i] for v in vs), 3) for i in range(3)]; hi = [round(max(v[i] for v in vs), 3) for i in range(3)]
    info = {"tris": len(o.data.polygons), "bounds_m": [lo, hi], "slots": [s.material.name for s in o.material_slots]}
    print("BUILT", name, info)
    o.hide_set(True)
    PARTS.clear()
    return info


# ---------------------------------------------------------------- manifold
# Local frame = world minus (0, 4.3, 0): wall plane y = +0.10, front faces -Y, floor z = 0.
GAUGES_X = (-0.9, -0.3, 0.3, 0.9)
GZ, FACE_Y = 1.65, 4.15 - 4.3          # gauge centre height, gauge face plane (local y = -0.15)
box("M_Lab04_PaintedSteel", (0, 0.08, 1.45), (2.6, 0.04, 1.36))                      # backplate
for x in (-1.25, 1.25):
    for z in (0.83, 2.07):
        cyl("M_Lab04_BareSteel", (x, 0.055, z), 0.014, 0.012, 'Y', 12)                 # bolts
for z in (0.92, 1.98):
    box("M_Lab04_BareSteel", (0, 0.04, z), (2.5, 0.04, 0.05))                          # mounting rails
# Header pipe and floor feeds.
pipe_between("M_Lab04_BareSteel", (-1.40, -0.02, 1.32), (1.40, -0.02, 1.32), 0.045)
for x in (-1.40, 1.40):
    pipe_between("M_Lab04_BareSteel", (x, -0.02, 0.0), (x, -0.02, 1.32), 0.045)
    cyl("M_Lab04_BareSteel", (x, -0.02, 1.32), 0.058, 0.10, 'Z', 24)                    # elbow collar
    cyl("M_Lab04_BareSteel", (x, -0.02, 0.03), 0.075, 0.06, 'Z', 24)                    # floor flange
for x in (-0.6, 0.0, 0.6):
    cyl("M_Lab04_BareSteel", (x, -0.02, 1.32), 0.056, 0.035, 'X', 24)                   # pipe couplings
for x in GAUGES_X:
    # Gauge: steel can behind the face plane, brass bezel outside the needle sweep.
    cyl("M_Lab04_PaintedSteel", (x, FACE_Y + 0.075, GZ), 0.205, 0.15, 'Y', 48)
    cyl("M_Lab04_GaugeFace", (x, FACE_Y + 0.002, GZ), 0.19, 0.004, 'Y', 48)            # front at local -0.15
    torus("M_Lab04_Brass", (x, FACE_Y - 0.004, GZ), 0.198, 0.013, 'Y', 48, 10)          # inner edge r 0.185
    for k in range(11):                                                                 # tick marks, -135..135 deg
        a = math.radians(-135 + k * 27)
        r = 0.158
        t = box("M_Lab04_GaugeRed" if k >= 9 else "M_Lab04_GaugeInk",
                (x + math.sin(a) * r, FACE_Y - 0.0005, GZ + math.cos(a) * r), (0.006, 0.001, 0.028 if k % 5 == 0 else 0.018), bevel=0)
        t.rotation_euler = (0, a, 0); bpy.ops.object.select_all(action='DESELECT'); t.select_set(True)
        bpy.context.view_layer.objects.active = t; bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
    box("M_Lab04_Brass", (x, FACE_Y + 0.0, GZ + 0.27), (0.16, 0.012, 0.05))              # name plate above
    # Riser from the header up into the gauge, and the isolation valve below.
    pipe_between("M_Lab04_BareSteel", (x, -0.02, 1.32), (x, -0.02, GZ - 0.20), 0.018, 12)
    cyl("M_Lab04_BareSteel", (x, -0.06, 1.10), 0.04, 0.10, 'Y', 20)                     # valve body
    pipe_between("M_Lab04_BareSteel", (x, -0.02, 1.32), (x, -0.02, 1.13), 0.02, 12)
    handwheel((x, -0.17, 1.10), 0.10)                                                   # front ~ world y 4.12
manifold = build("SM_Lab04_Manifold")

# ---------------------------------------------------------------- telemetry rack
# Local frame: origin on the floor under the old panel centre, front = -X, width along Y.
W, H = 1.20, 2.0
box("M_Lab04_PaintedSteel", (0.0, 0, H / 2), (0.24, W, H), bevel=0.006)               # carcass, front face x = -0.12
for y in (-W / 2 + 0.02, W / 2 - 0.02):
    box("M_Lab04_BareSteel", (-0.13, y, H / 2), (0.03, 0.04, H))                        # front posts
for z in (0.04, H - 0.02):
    box("M_Lab04_BareSteel", (-0.13, 0, z), (0.03, W, 0.04))                            # top / bottom rails
# Rack units (front panels) with vents, LED strips and switches; leave a bay for the bus tray at z 1.125-1.375.
units = [(0.15, 0.30), (0.50, 0.30), (0.85, 0.20), (1.50, 0.20), (2.0 - 0.16, 0.22)]
for zc, h in units:
    box("M_Lab04_PanelGrey", (-0.135, 0, zc), (0.01, W - 0.10, h - 0.02), bevel=0.003)
    for k in range(9):                                                                  # vent slots
        box("M_Lab04_Vent", (-0.141, -0.40 + k * 0.05, zc), (0.004, 0.025, h * 0.55), bevel=0)
    for k in range(4):                                                                  # status LEDs
        box("M_Lab04_LedGreen" if k else "M_Lab04_LedAmber", (-0.142, 0.12 + k * 0.035, zc + h * 0.25), (0.006, 0.014, 0.008), bevel=0)
    box("M_Lab04_BareSteel", (-0.145, 0.42, zc), (0.012, 0.03, 0.05), bevel=0.002)      # toggle
box("M_Lab04_Vent", (-0.122, -0.15, 1.25), (0.004, 0.64, 0.27), bevel=0)               # tray bay (dark surround)
# Bus tray (static; the hydrophone channel card the player reseats).
box("M_Lab04_PanelGrey", (-0.16, -0.15, 1.25), (0.12, 0.60, 0.25), bevel=0.004)
box("M_Lab04_Vent", (-0.221, -0.15, 1.25), (0.004, 0.50, 0.17), bevel=0)
for y in (-0.38, 0.08):
    pipe_between("M_Lab04_BareSteel", (-0.22, y, 1.22), (-0.26, y, 1.22), 0.008, 10)
    pipe_between("M_Lab04_BareSteel", (-0.22, y, 1.28), (-0.26, y, 1.28), 0.008, 10)
    pipe_between("M_Lab04_BareSteel", (-0.26, y, 1.22), (-0.26, y, 1.28), 0.008, 10)
# Fault lamp housing around the live lamp at (-0.15, 0.30, 1.80): back plate, base ring and four cage bars.
LAMP = Vector((-0.15, 0.30, 1.80))
box("M_Lab04_PanelGrey", (-0.13, 0.30, 1.80), (0.01, 0.20, 0.20), bevel=0.003)
cyl("M_Lab04_BareSteel", (-0.14, 0.30, 1.80), 0.075, 0.02, 'X', 32)
for k in range(4):
    a = math.radians(45 + 90 * k)
    off = Vector((0, math.cos(a) * 0.082, math.sin(a) * 0.082))
    pipe_between("M_Lab04_BareSteel", Vector((-0.14, 0.30, 1.80)) + off, LAMP + Vector((-0.10, 0, 0)) + off * 0.25, 0.005, 8)
torus("M_Lab04_BareSteel", (-0.19, 0.30, 1.80), 0.055, 0.005, 'X', 32, 6)              # cage ring on the bars
box("M_Lab04_Brass", (-0.137, 0.30, 1.66), (0.006, 0.14, 0.035), bevel=0)               # channel label plate
# Cable conduit from the top into the ceiling run.
pipe_between("M_Lab04_Rubber", (0.05, -0.35, H), (0.05, -0.35, H + 0.6), 0.03, 16)
pipe_between("M_Lab04_Rubber", (0.05, -0.25, H), (0.05, -0.25, H + 0.6), 0.022, 16)
rack = build("SM_Lab04_TelemetryRack")

# ---------------------------------------------------------------- dive watch
CASE_R, CASE_H = 0.021, 0.013
cyl("M_Lab04_BareSteel", (0, 0, CASE_H / 2), CASE_R, CASE_H, 'Z', 48)                 # case
torus("M_Lab04_WatchBezel", (0, 0, CASE_H), CASE_R - 0.0015, 0.0022, 'Z', 48, 8)        # bezel
for k in range(12):                                                                     # bezel grip notches
    a = math.radians(k * 30)
    box("M_Lab04_BareSteel", (math.cos(a) * (CASE_R + 0.0006), math.sin(a) * (CASE_R + 0.0006), CASE_H), (0.0015, 0.0015, 0.003), rot_z=k * 30, bevel=0)
cyl("M_Lab04_WatchScreen", (0, 0, CASE_H + 0.0003), CASE_R - 0.0042, 0.0006, 'Z', 48)  # live screen slot
for y, l in ((0.004, 0.0045), (-0.004, 0.0045)):
    cyl("M_Lab04_BareSteel", (CASE_R + 0.002, y * 2.0, CASE_H * 0.55), 0.0018, l, 'X', 12)  # pushers (presses x2)
cyl("M_Lab04_BareSteel", (CASE_R + 0.0025, 0, CASE_H * 0.5), 0.0026, 0.005, 'X', 16)    # crown
for s in (1, -1):                                                                       # lugs and strap stubs
    for x in (-0.009, 0.009):
        box("M_Lab04_BareSteel", (x, s * (CASE_R + 0.002), CASE_H * 0.45), (0.004, 0.008, 0.006), bevel=0.0008)
    box("M_Lab04_Rubber", (0, s * (CASE_R + 0.022), CASE_H * 0.35), (0.020, 0.040, 0.004), bevel=0.001)
watch = build("SM_Lab04_DiveWatch")

placement = {
    "SM_Lab04_Manifold": {"location_cm": [0.0, -430.0, 0.0], "yaw_deg": 0.0, "replaces": "GB_Lab04_Gauge_Panel + dials + gauge valves", **manifold},
    "SM_Lab04_TelemetryRack": {"location_cm": [329.4, -276.4, 0.0], "yaw_deg": -40.0, "replaces": "GB_Lab04_Telemetry_Panel + GB_Telemetry_Bus_Tray", **rack},
    "SM_Lab04_DiveWatch": {"location_cm": None, "yaw_deg": None, "replaces": None, **watch},
}
with open(os.path.join(ROOT, "blender", "props", "lab04_handmade_placement.json"), "w") as f:
    json.dump(placement, f, indent=2); f.write("\n")
