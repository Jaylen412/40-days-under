"""First-person forearms for Christian (viewmodel, owner-only). Blender 5.3:
    blender -b --factory-startup --python blender/props/build_fp_arms.py

Frame: pivot at the elbow, forearm along +X (elbow to fingertips 0.44 m), back of the hand up (+Z), left = +Y.
The left arm carries the dive watch: UE attaches SM_Lab04_DiveWatch at WATCH_UE below (cm / degrees),
which is the top of the strap on the forearm, crown toward the hand, 12 o'clock away from the thumb. Keep AUnderwaterCharacter in sync.
Writes blender/export/SM_FP_Forearm_L.fbx and SM_FP_Forearm_R.fbx (git-ignored).
"""
import bpy, bmesh, math, os
from mathutils import Matrix, Vector

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
EXPORT = os.path.join(ROOT, "blender", "export")
FBX_ARGS = dict(use_selection=True, object_types={'MESH'}, global_scale=1.0, apply_scale_options='FBX_SCALE_UNITS',
                apply_unit_scale=True, use_space_transform=True, bake_space_transform=True, mesh_smooth_type='FACE',
                use_mesh_modifiers=True, use_triangles=True, add_leaf_bones=False, bake_anim=False, path_mode='AUTO')
WRIST_X, WRIST_R = 0.25, 0.027                 # wrist joint (hand starts here), wrist radius
WATCH_X = WRIST_X - 0.025                       # watch sits on the forearm, 2.5 cm short of the joint
WATCH_UE = {"location_cm": [round(WATCH_X * 100, 2), 0.0, round((WRIST_R + 0.0015) * 100, 2)], "yaw_deg": 0.0}

bpy.ops.wm.read_factory_settings(use_empty=True)
MATS = {n: bpy.data.materials.new(n) for n in ("M_FP_Sleeve", "M_FP_Cuff", "M_FP_Skin", "M_FP_Strap")}
PARTS = []

def add(obj, mat, smooth=True):
    obj.data.materials.append(MATS[mat])
    for p in obj.data.polygons:
        p.use_smooth = smooth
    PARTS.append(obj)
    return obj

def tapered(mat, x0, x1, r0, r1, verts=24):
    """Open-ended tapered tube along +X."""
    bpy.ops.mesh.primitive_cone_add(vertices=verts, radius1=r0, radius2=r1, depth=x1 - x0,
                                    location=((x0 + x1) / 2, 0, 0), rotation=(0, math.radians(90), 0))
    return add(bpy.context.active_object, mat)

def rounded_box(mat, centre, size, bevel):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=centre)
    o = bpy.context.active_object; o.scale = size
    bpy.ops.object.transform_apply(scale=True)
    m = o.modifiers.new("B", 'BEVEL'); m.width = bevel; m.segments = 3
    return add(o, mat)

def finger(base, length, radius, pitch_deg=8.0, yaw_deg=0.0, segs=2):
    """Finger as capsule segments from base, curling down by pitch per joint."""
    p = Vector(base); d = Vector((1, 0, 0))
    d.rotate(Matrix.Rotation(math.radians(yaw_deg), 3, 'Z'))
    for k in range(segs):
        d.rotate(Matrix.Rotation(math.radians(pitch_deg), 3, Vector((-d.y, d.x, 0)).normalized()))
        q = p + d * (length / segs)
        bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=radius, depth=(q - p).length, location=(p + q) / 2)
        o = bpy.context.active_object
        o.rotation_mode = 'QUATERNION'; o.rotation_quaternion = (q - p).to_track_quat('Z', 'Y')
        add(o, "M_FP_Skin")
        bpy.ops.mesh.primitive_uv_sphere_add(segments=12, ring_count=8, radius=radius, location=q)
        add(bpy.context.active_object, "M_FP_Skin")
        p = q
        radius *= 0.92

def build_arm(side):
    """side = +1 left, -1 right (mirror in Y). Thumb is on the body side: -Y for the left hand."""
    tapered("M_FP_Sleeve", -0.02, WRIST_X - 0.06, 0.048, 0.038)                         # jacket sleeve
    bpy.ops.mesh.primitive_torus_add(major_radius=0.037, minor_radius=0.006, location=(WRIST_X - 0.065, 0, 0),
                                     rotation=(0, math.radians(90), 0), major_segments=24, minor_segments=8)
    add(bpy.context.active_object, "M_FP_Cuff")                                          # cuff band
    tapered("M_FP_Cuff", WRIST_X - 0.08, WRIST_X - 0.055, 0.040, 0.037)
    tapered("M_FP_Skin", WRIST_X - 0.08, WRIST_X + 0.03, WRIST_R, WRIST_R * 1.02)        # wrist
    if side > 0:                                                                         # watch strap round the wrist
        bpy.ops.mesh.primitive_cylinder_add(vertices=32, radius=WRIST_R + 0.0025, depth=0.020,
                                            location=(WATCH_X, 0, 0), rotation=(0, math.radians(90), 0))
        add(bpy.context.active_object, "M_FP_Strap")
    rounded_box("M_FP_Skin", (WRIST_X + 0.065, 0, -0.002), (0.085, 0.082, 0.028), 0.010)  # palm, back of hand up
    for k, (dy, l) in enumerate(((0.030, 0.070), (0.010, 0.080), (-0.010, 0.077), (-0.029, 0.062))):
        finger((WRIST_X + 0.105, dy * side, 0.0), l, 0.0085, pitch_deg=10.0)
    finger((WRIST_X + 0.045, -0.040 * side, -0.006), 0.055, 0.010, pitch_deg=6.0, yaw_deg=-38.0 * side)  # thumb
    bpy.ops.object.select_all(action='DESELECT')
    for o in PARTS:
        o.select_set(True)
        bpy.context.view_layer.objects.active = o
        for m in list(o.modifiers):
            bpy.ops.object.modifier_apply(modifier=m.name)
    bpy.context.view_layer.objects.active = PARTS[0]
    bpy.ops.object.join()
    arm = bpy.context.active_object
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    name = "SM_FP_Forearm_" + ("L" if side > 0 else "R")
    arm.name = arm.data.name = name
    bm = bmesh.new(); bm.from_mesh(arm.data)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(arm.data); bm.free()
    bpy.ops.object.select_all(action='DESELECT'); arm.select_set(True)
    bpy.ops.export_scene.fbx(filepath=os.path.join(EXPORT, name + ".fbx"), **FBX_ARGS)
    vs = [v.co for v in arm.data.vertices]
    print("ARM", name, len(arm.data.polygons), "tris",
          [round(min(v[i] for v in vs), 3) for i in range(3)], [round(max(v[i] for v in vs), 3) for i in range(3)])
    arm.hide_set(True)
    PARTS.clear()

build_arm(+1)
build_arm(-1)
print("WATCH_UE", WATCH_UE)
