#ifndef MESH_H
#define MESH_H

// @Note: This data structure is based off a video on Mesh VBOs by Jonathan Blow:
// https://www.youtube.com/watch?v=g2F0Yg17ZfU

struct Triangle_Mesh_Header {
    s32 num_vertices;            // Mul by sizeof(Vector3) to get bytes
    s32 num_uvs;                 // Mul by sizeof(Vector2) to get bytes
    s32 num_tbns;                // Mul by sizeof(TBN)     to get bytes
    s32 num_colors;              // Mul by sizeof(Vector4) to get bytes
    s32 num_indices;             // Mul by sizeof(u32)     to get bytes
    
    s32 num_triangle_lists;      // Mul by sizeof(Triangle_List_Info) to get bytes
    s32 num_materials;      
};

struct Triangle_List_Info {
    s32 material_index;
    s32 num_indices;
    s32 first_index;
    
    // @Todo: Multiple textures per triangle.
	Texture_Map *diffuse_map;
	Texture_Map *normal_map;
};

#define MAX_MAPS_PER_MATERIAL 3
struct Material_Info {
    Vector4 base_color;
	f32     specular;
    f32     roughness;
    Vector3 normal;
    
    Vector4 original_base_color;
    
    // @Note:
    // index 0: diffuse map.
    // index 1: normal map.
    // index 2: unused.
    String texture_map_names[MAX_MAPS_PER_MATERIAL];
};

struct TBN
{
    Vector3 tangent;
    Vector3 bitangent;
    Vector3 normal;
};

struct Bounding_Box
{
    // @Note: Object space coordinates.
    // We use this mainly for mouse picking.
    
    Rect3   bounds;
    Matrix4 unit_cube_to_object_matrix;
    
    // For rendering.
    GLuint  vbo_id;
    GLuint  ibo_id;
    s32     num_indices;
};

struct Bounding_Volume_Header
{
    s32 num_vertices;          // Mul by sizeof(Vector3) to get bytes
    s32 num_indices;           // Mul by sizeof(u32)     to get bytes
    s32 num_face_normal_axes;  // Mul by sizeof(Vector3) to get bytes
    s32 num_edge_axes;         // Mul by sizeof(Vector3) to get bytes
    s32 is_sphere;
};

struct Bounding_Volume
{
    // @Note: Object space coordinates.
    // @Todo: Flags bitfield to save space.
    
    //
    // For rendering.
    //
    s32 num_vertices;
    Vector3 *vertices;
    
    s32 num_indices;
    u32 *indices;
    
    GLuint  vbo_id;
    GLuint  ibo_id;
    
    //
    // For SAT collision test.
    //
    // @Note: Will be null if (is_sphere == true)
    s32 num_face_normal_axes;
    Vector3 *face_normal_axes;
    
    // @Note: Will be null if (is_sphere == true)
    s32 num_edge_axes;
    Vector3 *edge_axes;
    
    b32 is_sphere;
};

struct Triangle_Mesh
{
    // @Note: The mesh data is read from `blender_mesh_exporter.py` output.
    // Bounding_Volume data is read from `blender_meshbv_exporter.py`
    
    String name;
    String full_path;
    
    s32 num_vertices;
    Vector3 *vertices;
	
    s32 num_uvs;
    Vector2 *uvs;
    
    s32 num_tbns;
    TBN *tbns;
    
    s32 num_colors;
    Vector4 *colors;
    
    s32 num_indices;
    u32 *indices;
    
    Bounding_Box    bounding_box;    // Computed at mesh load time.
    Bounding_Volume bounding_volume; // Loaded from a .MESHBV file.
    
    // Ideally, num_triangle_lists == num_materials. 
    s32 num_triangle_lists;
	Triangle_List_Info *triangle_list_info;
	
    s32 num_materials;
    Material_Info *material_info;
    
    GLuint vbo_id;
    GLuint ibo_id;
};

struct Mesh_Entry
{
    String key_mesh_name;
    Triangle_Mesh *mesh;
};
GLOBAL Mesh_Entry global_mesh_hash_table[64];

#endif //MESH_H
