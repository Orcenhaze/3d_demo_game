#ifndef TERRAIN_H
#define TERRAIN_H

struct Terrain
{
    // @Note: The origin of the terrain is the center, even though we start at the top-left
    // corner when creating the initial grid.
    
    s32     num_cells_side;
    Vector3 position;
    Vector3 scale;
    
    s32 num_vertices;
    Vector3 *vertices;
    
    s32 num_indices;
    u32 *indices;
    
    Vector2 *uvs;
    
    Texture_Map *height_map;
    Texture_Map *diffuse_map;
    
    Matrix4_Inverse object_to_world_matrix;
    GLuint vbo_id;
    GLuint ibo_id;
};

#endif //TERRAIN_H
