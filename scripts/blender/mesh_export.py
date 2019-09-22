bl_info = {
    "name": ".gtmesh Geometry Format",
    "author": "Jascha Wedowski",
    "version": (0, 1, 0),
    "blender": (2, 80, 0),
    "location": "File > Import-Export",
    "description": "Export .gtmesh, Exports GT Mesh Geometry.",
    "category": "Import-Export"}

import bpy
from bpy.props import (
        BoolProperty,
        FloatProperty,
        StringProperty,
        EnumProperty,
        )
from bpy_extras.io_utils import (
        ExportHelper,
        orientation_helper,
        path_reference_mode,
        axis_conversion,
        )

#####
from enum import IntEnum
class VertexFormat(IntEnum):  
    POSITION = 0
    POSITION_NORMAL = 1
    POSITION_NORMAL_TEXCOORD0 = 2
    POSITION_NORMAL_TEXCOORD0_1 = 3
    POSITION_NORMAL_TANGENT_TEXCOORD0 = 4
    POSITION_NORMAL_TANGENT_TEXCOORD0_1 = 5
    COLOR0 = 6
    COLOR0_1 = 7
    BONE_WEIGHTS_INDICES = 8

class IndexFormat(IntEnum):
    NONE = 0
    UINT8 = 1
    UINT16 = 2
    UINT32 = 3

####
class Vec3(object):
    __slots__="x", "y", "z"
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

####
class Vertex(object):
    __slots__="id", "position", "normal", "tangent", "uvs"
    def __init__(self, id, position, normal, tangent, uvs):
        self.id = id
        self.position = position
        self.normal = normal
        self.tangent = tangent
        self.uvs = uvs
    def get_tuple(self):
        position = tuple(self.position)
        normal = tuple(self.normal)
        tangent = tuple(self.tangent)
        uvs = tuple(self.uvs)
        return (position, normal, tangent, uvs)

class Face(object):
    __slots__="vertices"
    def __init__(self, vertices):
        self.vertices = vertices

class Submesh(object):
    __slots__="offset", "count"
    def __init__(self, offset, count):
        self.offset = offset
        self.count = count

class AABB(object):
    __slots__="min", "max"
    def __init__(self, min, max):
        self.min = min
        self.max = max

#####
def save(context,
         filepath,
         *,
         use_selection=True,
         global_matrix=None,
         ):
    ##
    import bpy
    import mathutils
    import struct
    import bmesh
    from bpy_extras.io_utils import create_derived_objects, free_derived_objects

    ##
    file = open(filepath, 'wb')
    ##
    depsgraph = context.evaluated_depsgraph_get()
    scene = context.scene
    #exit edit mode
    if bpy.ops.object.mode_set.poll():
        bpy.ops.object.mode_set(mode='OBJECT')
    
    frame = scene.frame_current
    if use_selection:
        objects = context.selected_objects
    else:
        objects = scene.objects
    if global_matrix is None:
        global_matrix = mathutils.Matrix()

    

    mesh_objects = []
    print(f"Processing {len(objects)} objects...")
    for ob in objects:
        print(f"Object type: {ob.type}")
        if ob.type not in {'MESH', 'CURVE', 'SURFACE', 'FONT', 'META'}:
            continue

        # get derived ojects
        free, derived = create_derived_objects(scene, ob)
        if derived is None:
            continue

        print(f"Processing {len(derived)} derived objects...")
        for ob_derived, mat in derived:
            try:
                data = ob_derived.evaluated_get(depsgraph).to_mesh()
            except:
                data = None

            if data:
                print("Got data...")
                matrix = global_matrix @ mat
                data.transform(matrix)
                mesh_objects.append((ob_derived, data, matrix))

        if free:
            free_derived_objects(ob)

    faces = []
    vertices = []
    submeshes = []

    print(f"Processing {len(mesh_objects)} mesh objects...")
    for ob, blender_mesh, matrix in mesh_objects:
        
        submeshOffset = len(faces) * 3
        submeshCount = 0;

        # explicitly triangulate the mesh
        bm = bmesh.new()
        bm.from_mesh(blender_mesh)
        bmesh.ops.triangulate(bm, faces=bm.faces[:])
        bm.to_mesh(blender_mesh)
        bm.free()


        blender_mesh.calc_loop_triangles()
        blender_mesh.calc_tangents()
        for i, face in enumerate(blender_mesh.loop_triangles):
            faceVertices = []
            for vertIdx, loopIdx in zip(face.vertices, face.loops):
                position = (-blender_mesh.vertices[vertIdx].co.x, blender_mesh.vertices[vertIdx].co.y, blender_mesh.vertices[vertIdx].co.z)
                normal = (-blender_mesh.vertices[vertIdx].normal.x, blender_mesh.vertices[vertIdx].normal.y, blender_mesh.vertices[vertIdx].normal.z)
                tangent = (blender_mesh.loops[loopIdx].tangent.x, blender_mesh.loops[loopIdx].tangent.y, blender_mesh.loops[loopIdx].tangent.z)
                uvLayer0 = blender_mesh.uv_layers.active.data
                uvs = (uvLayer0[loopIdx].uv[0], uvLayer0[loopIdx].uv[1])
                faceVertices.append(Vertex(vertIdx, position, normal, tangent, uvs))
                submeshCount = submeshCount + 1
            faces.append(Face(faceVertices))
        submeshes.append(Submesh(submeshOffset, submeshCount))
        print(f"Added submesh with offset = {submeshOffset}, count = {submeshCount}")
    ##
    indices = []
    indexMap = {}
    for f in faces:
        ind = []
        for v in f.vertices:
            vTuple = v.get_tuple()
            if vTuple in indexMap:
                ind.append(indexMap[vTuple])
            else:
                ind.append(len(vertices))
                indexMap[vTuple] = len(vertices)
                vertices.append(v)
        indices.append(ind[0])
        indices.append(ind[2])
        indices.append(ind[1])

    # write header
    numIndices = len(indices)
    formatSpecifier = None
    if numIndices <= 256:
        formatSpecifier = '<B'
        indexFormat = IndexFormat.UINT8
    else:
        if numIndices > 0xffff:
            formatSpecifier = '<I'
            indexFormat = IndexFormat.UINT32
        else:
            formatSpecifier = '<H'
            indexFormat = IndexFormat.UINT16
    

    file.write(struct.pack('<B', int(VertexFormat.POSITION_NORMAL_TANGENT_TEXCOORD0)))
    file.write(struct.pack('<B', int(indexFormat)))   # @NOTE assumption of 2 byte indices
    file.write(struct.pack('<Q', len(vertices)))
    file.write(struct.pack('<Q', numIndices))
    file.write(struct.pack('<I', len(submeshes)))

    # write bounding box

    bbox = AABB(Vec3(vertices[0].position[0], vertices[0].position[1], vertices[0].position[2]), Vec3(vertices[0].position[0], vertices[0].position[1], vertices[0].position[2]))
    for v in vertices:
        if v.position[0] < bbox.min.x:
            bbox.min.x = v.position[0]
        if v.position[1] < bbox.min.y:
            bbox.min.y = v.position[1]
        if v.position[2] < bbox.min.z:
            bbox.min.z = v.position[2]

        if v.position[0] > bbox.max.x:
            bbox.max.x = v.position[0]
        if v.position[1] > bbox.max.y:
            bbox.max.y = v.position[1]
        if v.position[2] > bbox.max.z:
            bbox.max.z = v.position[2]

    file.write(struct.pack('<f', bbox.max.x))
    file.write(struct.pack('<f', bbox.max.y))
    file.write(struct.pack('<f', bbox.max.z))
    file.write(struct.pack('<f', bbox.min.x))
    file.write(struct.pack('<f', bbox.min.y))
    file.write(struct.pack('<f', bbox.min.z))

    print(f"Writing mesh with {len(vertices)} vertices, {len(indices)} indices, {len(submeshes)} submeshes.")  

    # write vertices
    for i, v in enumerate(vertices):
        # position
        file.write(struct.pack('<f', v.position[0]))
        file.write(struct.pack('<f', v.position[1]))
        file.write(struct.pack('<f', v.position[2]))
        # normal
        file.write(struct.pack('<f', v.normal[0]))
        file.write(struct.pack('<f', v.normal[1]))
        file.write(struct.pack('<f', v.normal[2]))
        # tangent
        file.write(struct.pack('<f', v.tangent[0]))
        file.write(struct.pack('<f', v.tangent[1]))
        file.write(struct.pack('<f', v.tangent[2]))
        file.write(struct.pack('<f', 0));
        # uvs
        file.write(struct.pack('<f', v.uvs[0]))
        file.write(struct.pack('<f', v.uvs[1]))
        
    # write indices
    for i in indices:
        file.write(struct.pack(formatSpecifier, i))
        
    # write submeshes
    for submesh in submeshes:
        print(f"Write submesh with offset = {submesh.offset}, count = {submesh.count}")
        file.write(struct.pack('<Q', submesh.offset))
        file.write(struct.pack('<Q', submesh.count))

    file.close()
    return {'FINISHED'}

#####
@orientation_helper(axis_forward='-Z', axis_up='Y')
class ExportGTMesh(bpy.types.Operator, ExportHelper):
    """Save a GT Mesh .gtmesh File"""

    bl_idname = "export_scene.gtmesh"
    bl_label = 'Export .gtmesh'
    bl_options = {'PRESET'}

    filename_ext = ".gtmesh"
    filter_glob: StringProperty(
            default="*.gtmesh",
            options={'HIDDEN'},
            )

    # context group
    use_selection: BoolProperty(
            name="Selection Only",
            description="Export selected objects only",
            default=False,
            )
    global_scale: FloatProperty(
            name="Scale",
            min=0.01, max=1000.0,
            default=1.0,
            )

    check_extension = True

    def execute(self, context):

        print("Executing...")

        from mathutils import Matrix
        keywords = self.as_keywords(ignore=("axis_forward",
                                            "axis_up",
                                            "global_scale",
                                            "check_existing",
                                            "filter_glob",
                                            ))

        global_matrix = (Matrix.Scale(self.global_scale, 4) @
                         axis_conversion(to_forward=self.axis_forward,
                                         to_up=self.axis_up,
                                         ).to_4x4())

        keywords["global_matrix"] = global_matrix
        return save(context, **keywords)

#####

def menu_func_export(self, context):
    self.layout.operator(ExportGTMesh.bl_idname, text="GT Mesh (.gtmesh)")


#classes = ( ExportGTMesh )

#####

def register():
    #for cls in classes:
    #    bpy.utils.register_class(cls)
    bpy.utils.register_class(ExportGTMesh)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    #for cls in classes:
    #    bpy.utils.unregister_class(cls)
    bpy.utils.unregister_class(ExportGTMesh)


if __name__ == "__main__":
    register()