mesh_name        = "test"
smooth_shaded    = 0
make_convex_hull = 0

directory = "W:\\untitled_roguelike_game\\data\\mesh_data\\"
extension = ".mesh"


"""
    About this mesh exporter:
    - It only writes blender's Principled BSDF, and only the basic data in it.
    - It exports one object only, so make sure to join sub-meshes together.
    - It only exports one index of Vertex Colors.
    - The exported object must have _at least_ one material slot.
    - It generates the convex hull if `make_convex_hull` is set to 1.
    
    INCOMPLETE:
    1) Doesn't export skeletal hierarchy nor animation.
    
"""

""" IMPORTS
"""
from   collections import OrderedDict
import os
import struct
import array
import bpy
import bmesh
import pdb

""" GLOBAL VARIABLES
"""
last_vert_index       = None
putbnc_vertices       = {}      # {vertex_index: vertex_data}
texture_map_fullpaths = []

# Data to write:
triangle_mesh_header  = []
vertices 			  = []
uvs 				  = []
tbns                  = []
colors 			      = []
indices 		      = []
triangle_list_info    = []
material_info         = []

# Reading material_info (for header.num_materials):
# - current material attributes.
# - texture count for current material.
# - length of texture file_name
# - the texture file_name

""" FUNCTIONS
"""
def set_material_info(input):
    # Check if attribute type is iterable.
    if input.type == "RGBA" or input.type == "VECTOR":
        material_info.extend(input.default_value)
    else:
        material_info.append(input.default_value)

def set_input_node(input):
    # Set the attribute according to its type.
    # Supported node links: 1.
    # Supported node types: TEX_IMAGE and NORMAL_MAP.
    if   len(input.links) == 1 and input.links[0].from_node.type == 'TEX_IMAGE':
        set_material_info(input)
        texture_map_fullpaths.append(input.links[0].from_node.image.filepath)
    elif len(input.links) == 1 and input.links[0].from_node.type == 'NORMAL_MAP':
        set_material_info(input)
        texture_map_fullpaths.append(input.links[0].from_node.inputs['Color'].links[0].from_node.image.filepath)
    else:
        set_material_info(input)

def set_materials(materials):
    for mat in  materials:    
        # Get the bsdf node.
        bsdf = None
        if mat.node_tree != None:                   # Should have node tree.
            for n in mat.node_tree.nodes.keys():    # Searching all nodes.
                node = mat.node_tree.nodes[n]
                if node.type == 'BSDF_PRINCIPLED':  # Node type should be BSDF.
                    bsdf = node                     # Setting the node.
                    break                           # Exit node tree.
        
        # BSDF not found, skipping.
        if bsdf == None:
            continue
        
        # Get attributes (keep it simple for now).
        base_color              = bsdf.inputs.get("Base Color")
        #metallic                = bsdf.inputs.get("Metallic") 
        specular                = bsdf.inputs.get("Specular")
        #specular_tint           = bsdf.inputs.get("Specular Tint")
        roughness               = bsdf.inputs.get("Roughness")
        #index_of_refraction     = bsdf.inputs.get("IOR")
        #transmission            = bsdf.inputs.get("Transmission")
        #transmission_roughness  = bsdf.inputs.get("Transmission Roughness")
        #emission                = bsdf.inputs.get("Emission")
        #emission_strength       = bsdf.inputs.get("Emission Strength")
        normal_map              = bsdf.inputs.get("Normal")

        # Set attributes.
        set_input_node(base_color)
        #set_input_node(metallic)
        set_input_node(specular)
        #set_input_node(specular_tint)
        set_input_node(roughness)
        #set_input_node(index_of_refraction)
        #set_input_node(transmission)
        #set_input_node(transmission_roughness)
        #set_input_node(emission)
        #set_input_node(emission_strength)
        set_input_node(normal_map)

        # Append texture count for the current material.
        global texture_map_fullpaths
        num_textures = len(texture_map_fullpaths)
        material_info.append(num_textures) 

        # Extract basenames and append them to material_info.
        for s in range(num_textures):
            full_path         = texture_map_fullpaths[s]
            file_name         = bpy.path.basename(full_path)
            file_name_len     = len(file_name.encode('utf-8'))
            file_name_unicode = [ord(c) for c in file_name]
            material_info.append(file_name_len)
            material_info.append(file_name_unicode) # Append as a list of unicode ints.
            
        texture_map_fullpaths = []

def add_unique_vertex_and_set_index(PUTBNC, face, loop_index):
    global last_vert_index
    vert_index = face.vertices[loop_index]
    if PUTBNC not in putbnc_vertices.values():
        if vert_index not in putbnc_vertices.keys():
            # Use the current vertex index if it has not been used before.
            putbnc_vertices[vert_index] = PUTBNC
            indices.append(vert_index)
        else:
            # Otherwise use a new vertex index.
            putbnc_vertices[last_vert_index] = PUTBNC
            indices.append(last_vert_index)
            last_vert_index += 1
    else:
        # If PUTBNC is a duplicate, append its index to be reused.
        vert_index = list(putbnc_vertices.keys())[list(putbnc_vertices.values()).index(PUTBNC)]
        indices.append(vert_index)
        
def main():
    """ VARIABLES
    """
    global last_vert_index
    obj                = None
    mesh_data          = None
    previous_polygon   = None
    triangle_lists     = []     # len(triangle_lists) is num_triangle_lists
    materials          = []     # len(materials)      is num_materials
    used_materials     = []
    current_list_index = -1
    
    """ SETUP
    """
    # Set object mode.
    bpy.ops.object.mode_set()
    
    # Store the active/selected object.
    obj = bpy.context.view_layer.objects.active
    if obj.type != "MESH":
        raise Exception("Object must be of type MESH, not %s." % obj.type)
    if not obj.material_slots.values():
        raise Exception("The object must have at least one Material Slot.")
    
    mesh_data      = obj.data
    orig_mesh_data = mesh_data.copy()
    
    # Create bmesh.
    b_mesh = bmesh.new()
    b_mesh.from_mesh(mesh_data)
   
    # Split quads into triangles.
    bmesh.ops.triangulate(b_mesh, faces=b_mesh.faces[:])
    
    # Update mesh after triangulation.
    b_mesh.to_mesh(mesh_data)
    b_mesh.free()
    
    # Build the Convex Hull as a mesh.
    if make_convex_hull:
        ch_obj       = obj.copy()
        ch_obj.name  = "%sConvexHull" % obj.name
        ch_mesh_data = ch_obj.data = bpy.data.meshes.new("%sConvexHull" % orig_mesh_data.name)
        
        ch_b_mesh = bmesh.new()
        ch_b_mesh.from_mesh(orig_mesh_data)
        
        ch = bmesh.ops.convex_hull(ch_b_mesh, input=ch_b_mesh.verts, use_existing_faces=True)
        bmesh.ops.delete(ch_b_mesh, geom=ch["geom_unused"] + ch["geom_interior"], context='VERTS')
        #bmesh.ops.triangulate(ch_b_mesh, faces=ch_b_mesh.faces[:])
        
        ch_b_mesh.to_mesh(ch_mesh_data)
        ch_b_mesh.free()
        bpy.context.scene.collection.objects.link(ch_obj)
    
    # Make sure we have one vertex color index.
    if not mesh_data.vertex_colors.active:
        mesh_data.vertex_colors.new()
    
    # Calculate tangents for mesh.
    mesh_data.calc_tangents()
    
    # Initialize last_vert_index
    last_vert_index = len(mesh_data.vertices)
    
    for face in mesh_data.polygons:
        PUTBNC0 = []
        PUTBNC1 = []
        PUTBNC2 = []
        
        # The TBN is the same for each vertex in a face.
        face_tangent   = mesh_data.loops[face.index*3].tangent
        face_normal    = mesh_data.loops[face.index*3].normal
        face_bitangent = mesh_data.loops[face.index*3].bitangent_sign * face_normal.cross(face_tangent)
        
        # If you want to use flat shading, use the face_normal, and append it to all vertices in this face.
        TB012  = list(face_tangent)
        TB012 += list(face_bitangent)
        
        TBN0 = TB012 + list(face_normal)
        TBN1 = TBN0
        TBN2 = TBN0
        if smooth_shaded:
            TBN0 = TB012 + list(mesh_data.vertices[face.vertices[0]].normal)
            TBN1 = TB012 + list(mesh_data.vertices[face.vertices[1]].normal)
            TBN2 = TB012 + list(mesh_data.vertices[face.vertices[2]].normal)
            
        # Construct vertex [0] in current face and add it to the list (putbnc_vertices) if it is unique.
        P0    = list(mesh_data.vertices[face.vertices[0]].co)
        U0    = list(mesh_data.uv_layers.active.data[face.index*3 + 0].uv) if mesh_data.uv_layers.active is not None else [0.0, 0.0]
        U0[1] = 1.0 - U0[1]     # Flip vertically, to make origin top-left.
        C0    = list(mesh_data.vertex_colors[0].data[face.index*3 + 0].color)
        PUTBNC0.append(P0)
        PUTBNC0.append(U0)
        PUTBNC0.append(TBN0)
        PUTBNC0.append(C0)
        add_unique_vertex_and_set_index(PUTBNC0, face, 0)
        
        # Construct vertex [1].
        P1    = list(mesh_data.vertices[face.vertices[1]].co)
        U1    = list(mesh_data.uv_layers.active.data[face.index*3 + 1].uv) if mesh_data.uv_layers.active is not None else [0.0, 0.0]
        U1[1] = 1.0 - U1[1]     # Flip vertically, to make origin top-left.
        C1    = list(mesh_data.vertex_colors[0].data[face.index*3 + 1].color)
        PUTBNC1.append(P1)
        PUTBNC1.append(U1)
        PUTBNC1.append(TBN1)
        PUTBNC1.append(C1)
        add_unique_vertex_and_set_index(PUTBNC1, face, 1)

        # Construct vertex [2].
        P2 = list(mesh_data.vertices[face.vertices[2]].co)
        U2 = list(mesh_data.uv_layers.active.data[face.index*3 + 2].uv) if mesh_data.uv_layers.active is not None else [0.0, 0.0]
        U2[1] = 1.0 - U2[1]     # Flip vertically, to make origin top-left.
        C2 = list(mesh_data.vertex_colors[0].data[face.index*3 + 2].color)
        PUTBNC2.append(P2)
        PUTBNC2.append(U2)
        PUTBNC2.append(TBN2)
        PUTBNC2.append(C2)
        add_unique_vertex_and_set_index(PUTBNC2, face, 2)
    
        # Get used_materials and construct triangle_lists.
        used_materials.append(obj.material_slots[face.material_index].material)
        if face.index == 0:
            triangle_lists.append([face.material_index, face.loop_total, face.loop_start])
            previous_polygon = face
            current_list_index += 1
            continue
        if face.material_index == previous_polygon.material_index:
            triangle_lists[current_list_index][1] += 3
            previous_polygon = face
            continue
        if face.material_index != previous_polygon.material_index:
            triangle_lists.append([face.material_index, face.loop_total, face.loop_start])
            previous_polygon = face
            current_list_index += 1
            continue
    
    
    # Set triangle_list_info to export.
    for t in triangle_lists:
        triangle_list_info.extend(t)
        
    # Intersection between all materials and used_materials.
    used_materials = list(OrderedDict.fromkeys(used_materials))
    materials      = [mat for mat in mesh_data.materials if mat in used_materials]
    
    # Set material_info to export.
    set_materials(materials)
    
    # Set vertex attributes to export. They must be sorted, so we can index the correct ones. 
    for key in sorted(putbnc_vertices.keys()):
        vertices.extend(putbnc_vertices[key][0])
        uvs.extend(putbnc_vertices[key][1])
        tbns.extend(putbnc_vertices[key][2])
        colors.extend(putbnc_vertices[key][3])
    
    """ TRIANGLE_MESH_HEADER
    """
    triangle_mesh_header.append(len(vertices)//3)		# num_vertices
    triangle_mesh_header.append(len(uvs)//2)			# num_uvs
    triangle_mesh_header.append(len(tbns)//9)	        # num_tbns
    triangle_mesh_header.append(len(colors)//4)		    # num_colors
    triangle_mesh_header.append(len(indices))           # num_indices
    triangle_mesh_header.append(len(triangle_lists))    # num_triangle_lists
    triangle_mesh_header.append(len(materials))		    # num_materials

    """ WRITE TO BINARY FILE
    """
    filepath = directory + mesh_name + extension
    
    if os.path.exists(filepath):
      os.remove(filepath)
    f = open(filepath, "wb")

    triangle_mesh_header_arr = array.array('i', triangle_mesh_header)
    vertices_arr     		 = array.array('f', vertices)
    uvs_arr     			 = array.array('f', uvs)
    tbns_arr      	         = array.array('f', tbns)
    colors_arr       		 = array.array('f', colors)
    indices_arr     		 = array.array('I', indices)
    triangle_list_info_arr   = array.array('i', triangle_list_info)

    triangle_mesh_header_arr .tofile(f)
    vertices_arr			 .tofile(f)
    uvs_arr					 .tofile(f)
    tbns_arr		         .tofile(f)
    colors_arr				 .tofile(f)
    indices_arr              .tofile(f)
    triangle_list_info_arr	 .tofile(f)

    # material_info has different types inside it.
    for e in material_info:
        if isinstance(e, float):
            tmp = struct.pack("<f", e)
            f.write(tmp)
        if isinstance(e, int):
            tmp = struct.pack("<i", e)
            f.write(tmp)
        if isinstance(e, list):
            tmp = array.array('b', e)
            tmp.tofile(f)

    f.close()

    # Clear console (for debugging)
    #cls = lambda: system('cls')
    #cls()

    """ DEBUG
    """
    #print(material_info)
    #print(mesh_data)
    #print("len frames: {}".format(len(tbns)))
    #print("len frames//9: {}".format(len(tbns)//9))
    #print("len verts: {}".format(len(vertices)//3))
    #print(tbns)
    #print("len indices: {}".format(len(indices)))
    #print("indices: {}".format(indices))
    #print("len putbnc_vertices: {}".format(len(putbnc_vertices)))
    #print("putbnc_vertices: {}".format(putbnc_vertices))
    #print("Sorted putbnc_vertices.keys(): {}".format(sorted(putbnc_vertices.keys())))
    #print(uvs)

    print("\"{0}\" exported successfully!".format(mesh_name+extension))

if __name__ == "__main__":
    main()