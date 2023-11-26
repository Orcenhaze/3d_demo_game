meshbv_name      = "test"
is_sphere        = 0

directory = "W:\\untitled_roguelike_game\\data\\mesh_data\\"
extension = ".meshbv"



"""
    About this mesh bounding volume exporter:
    - It is meant to be used specifically for SAT collision tests.
    - The exported object must have _zero_ material slots.
    
    INCOMPLETE:
    1) Doesn't export more than one bounding volume (yet).
    
"""

""" IMPORTS
"""
import numpy
import os
import struct
import array
import bpy
import bmesh
import pdb

""" GLOBAL VARIABLES
"""
# Data to write:
bounding_volume_header = []
vertices 			   = []
indices 		       = []
face_normal_axes	   = []
edge_axes	           = []

""" FUNCTIONS
"""
def main():
    """ VARIABLES
    """
    mesh_data = None
    normal_axes_list = []
    edge_axes_list   = []
    
    """ SETUP
    """
    # Make sure the bounding mesh isn't triangulated when we start, else we'll  
    # have useless edge_axes that aren't on the border of that bounding mesh.
    if bpy.context.active_object.mode != "EDIT":
        bpy.ops.object.editmode_toggle()
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.tris_convert_to_quads()
    
    # Set object mode.
    bpy.ops.object.mode_set()
    
    # Store the active/selected object.
    obj = bpy.context.view_layer.objects.active
    if obj.type != "MESH":
        raise Exception("Object must be of type MESH, not %s." % obj.type)
    if obj.material_slots.values():
        raise Exception("The object must have NO Material Slots.")
    
    mesh_data = obj.data
        
    # Prepare face_normal_axes and edge_axes pre-triangulation.
    if is_sphere == 0:
        T = [[True], [True], [True]]
        
        # Due to floating-point precision, we'll check if the axis we
        # compute is close to an existing axis we have. If NOT, we add it.
        # numpy.isclose() returns boolean for each element in an array.
        for face in mesh_data.polygons:
            n = list(face.normal)
            
            axis = list(numpy.absolute(n))
            
            exists = T in list(list(numpy.isclose(axis, sublist)) for sublist in normal_axes_list)
            
            if not exists:
                normal_axes_list.append(axis)
        
        for edge in mesh_data.edges:
            v0 = list(mesh_data.vertices[edge.vertices[0]].co)
            v1 = list(mesh_data.vertices[edge.vertices[1]].co)
            
            axis = numpy.subtract(v1, v0)
            axis = axis / numpy.linalg.norm(axis)
            axis = list(numpy.absolute(axis))
            
            exists = T in list(list(numpy.isclose(axis, sublist)) for sublist in edge_axes_list)
            exists = (exists) or ( T in list(list(numpy.isclose(axis, sublist)) for sublist in normal_axes_list))
            
            if not exists:
                edge_axes_list.append(axis)
                
    # Triangulate the bounding mesh.
    b_mesh = bmesh.new()
    b_mesh.from_mesh(mesh_data)
   
    bmesh.ops.triangulate(b_mesh, faces=b_mesh.faces[:])
    
    b_mesh.to_mesh(mesh_data)
    b_mesh.free()
    
    # Set vertices to export.
    for vert in mesh_data.vertices:
        vertices.extend(vert.co)
        
    # Set indices to export.
    for face in mesh_data.polygons:
        indices.append(face.vertices[0])
        indices.append(face.vertices[1])
        indices.append(face.vertices[2])
    
    # Set face_normal_axes to export.
    for nx in normal_axes_list:
        face_normal_axes.extend(nx)
        
    # Set edge_axes to export.
    for ex in edge_axes_list:
        edge_axes.extend(ex)
        
    """ BOUNDING_VOLUME_HEADER
    """
    bounding_volume_header.append(len(vertices)//3)		    # num_vertices
    bounding_volume_header.append(len(indices))             # num_indices
    bounding_volume_header.append(len(face_normal_axes)//3) # num_face_normal_axes
    bounding_volume_header.append(len(edge_axes)//3)	    # num_edge_axes
    bounding_volume_header.append(is_sphere)		        # is_sphere
    
    """ WRITE TO BINARY FILE
    """
    filepath = directory + meshbv_name + extension
    
    if os.path.exists(filepath):
      os.remove(filepath)
    f = open(filepath, "wb")
    
    bounding_volume_header_arr = array.array('i', bounding_volume_header)
    vertices_arr     		   = array.array('f', vertices)
    indices_arr     		   = array.array('I', indices)
    face_normal_axes_arr       = array.array('f', face_normal_axes)
    edge_axes_arr       	   = array.array('f', edge_axes)
    
    bounding_volume_header_arr .tofile(f)
    vertices_arr               .tofile(f)
    indices_arr                .tofile(f)
    face_normal_axes_arr       .tofile(f)
    edge_axes_arr              .tofile(f)
    
    f.close()
    
    
    """ DEBUG
    """
    print("=====================")
    print("        BEGIN        ")
    print("=====================")
    print(normal_axes_list)
    print("=====================")
    print(edge_axes_list)
    print("=====================")
    for x in bounding_volume_header:
        print(x)
    print("=====================")
    print("         END         ")
    print("=====================")
    
    print("\"{0}\" exported successfully!".format(meshbv_name+extension))

if __name__ == "__main__":
    main()