
FUNCTION void make_terrain(Arena_List *arena_list, 
                           Terrain *terrain, 
                           Vector3 position, Vector3 scale, f32 uv_scale,
                           s32 num_cells_side = MIN_S32)
{
    
    terrain->position = position;
    terrain->scale    = scale;
    
    Memory_Arena *level_arena = &arena_list->level_arena;
    Memory_Arena *temp_arena = &arena_list->temp_arena;
    
    //
    // Find texture maps.
    //
    if(!terrain->height_map && !terrain->diffuse_map) return;
    
    if(!terrain->diffuse_map)
        terrain->diffuse_map = find_texture(default_white_texture);
    
    //
    // If num_cells_side is not passed, we set it to height_map->width if available.
    //
    Texture_Map *m = terrain->height_map;
    if(num_cells_side <= 0)
    {
        num_cells_side = m? (m->width - 1) : 10;
    }
    
    s32 num_vertices_side   = (num_cells_side + 1);
    s32 num_vertices_total  = num_vertices_side * num_vertices_side;
    terrain->num_vertices   = num_vertices_total;
    terrain->vertices       = PUSH_ARRAY(level_arena, num_vertices_total, Vector3);
    terrain->uvs            = PUSH_ARRAY(level_arena, num_vertices_total, Vector2);
    Vertex_XU *v_buffer     = PUSH_ARRAY(temp_arena, num_vertices_total, Vertex_XU);
    Vertex_XU *v            = v_buffer;
    terrain->num_cells_side = num_cells_side;
    for(s32 row = 0; row < num_vertices_side; row++)
    {
        for(s32 col = 0; col < num_vertices_side; col++)
        {
            //
            // Initial grid coordinates [0.0f, 1.0f] (origin at top-left).
            //
            f32 x = (f32)col / (f32)num_cells_side;
            f32 y = 0.0f;
            f32 z = (f32)row / (f32)num_cells_side;
            
            s32 vindex = row*num_vertices_side + col;
            
            //
            // Fill uvs.
            //
            terrain->uvs[vindex] = make_vector2(x, z)*uv_scale;
            
            //
            // Read heights from height_map with bilinear interpolation.
            //
            f32 height     = 0.0f;
            if(m)
            {
                s32 width = m->width - 1;
                
                f32 colf  = x * (f32)width;
                f32 rowf  = z * (f32)width;
                
                s32 coli  = (s32)(colf);
                s32 rowi  = (s32)(rowf);
                
                f32 tx    = (colf - (f32)coli);
                f32 ty    = (rowf - (f32)rowi);
                
                f32 x1    = get_pixel(m, rowi+0, coli+0).r;
                f32 x2    = get_pixel(m, rowi+0, coli+1).r;
                
                f32 y1    = get_pixel(m, rowi+1, coli+0).r;
                f32 y2    = get_pixel(m, rowi+1, coli+1).r;
                
                height    = lerp(lerp(x1, tx, x2), ty, lerp(y1, tx, y2));
                height   /= 255.0f;
            }
            
            //
            // Fill vertices and make origin at the center.
            //
            terrain->vertices[vindex] = make_vector3(x, height, z) - make_vector3(0.5f);
            
            //
            // Prepare buffers for GPU.
            //
            v->position = terrain->vertices[vindex];
            v->uv       = terrain->uvs[vindex];
            v++;
        }
    }
    
    //
    // Fill indices (2 triangles per cell/face - 3 indices per triangle).
    //
    s32 num_cells_total  = (num_cells_side) * (num_cells_side);
    terrain->num_indices = num_cells_total * 2 * 3;
    terrain->indices     = PUSH_ARRAY(level_arena, terrain->num_indices, u32);
    u32 *indices         = terrain->indices;
    for(s32 row = 0; row < num_cells_side; row++)
    {
        for(s32 col = 0; col < num_cells_side; col++)
        {
            u32 r0 = (row + 0) * (num_cells_side + 1);
            u32 r1 = (row + 1) * (num_cells_side + 1);
            
            // Triangle 1 (CCW).
            *indices++ = r0 + col + 0; 
            *indices++ = r0 + col + 1; 
            *indices++ = r1 + col + 1;
            
            // Triangle 2 (CCW).
            *indices++ = r1 + col + 1;
            *indices++ = r1 + col + 0;
            *indices++ = r0 + col + 0; 
        }
    }
    
    //
    // Generate buffers for terrain.
    //
    extensions->glGenBuffers(1, &terrain->vbo_id);
    extensions->glBindBuffer(GL_ARRAY_BUFFER, terrain->vbo_id);
    extensions->glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_XU) * num_vertices_total, v_buffer, GL_STATIC_DRAW);
    extensions->glGenBuffers(1, &terrain->ibo_id);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain->ibo_id);
    extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * terrain->num_indices, terrain->indices, GL_STATIC_DRAW);
    
    //
    // Set the terrain transform.
    //
    Matrix4 m_non = matrix4_identity();
    m_non._14 = position.x;
    m_non._24 = position.y;
    m_non._34 = position.z;
    m_non._11 = scale.x;
    m_non._22 = scale.y;
    m_non._33 = scale.z;
    Matrix4 m_inv = matrix4_identity();
    m_inv._14 = -position.x;
    m_inv._24 = -position.y;
    m_inv._34 = -position.z;
    m_inv._11 = 1.0f/scale.x;
    m_inv._22 = 1.0f/scale.y;
    m_inv._33 = 1.0f/scale.z;
    terrain->object_to_world_matrix.non_inverse = m_non;
    terrain->object_to_world_matrix.inverse     = m_inv;
    
    clear_arena(temp_arena);
}

FUNCTION f32 get_terrain_height_world(Terrain *terrain, Vector3 p_world, Collision_Data *col_data_out)
{
    f32 result = 0.0f;
    
    Vector3 *vertices     = terrain->vertices;
    s32 num_cells_side    = terrain->num_cells_side;
    s32 num_vertices_side = (num_cells_side + 1);
    
    // Transform to terrain space and make the origin the top-left corner of the terrain.
    Vector3 p_terrain = terrain->object_to_world_matrix.inverse * p_world;
    p_terrain        += make_vector3(0.5f);
    
    // p_terrain is in range [0, 1] here.
    s32 cell_x        = (s32)(p_terrain.x * (f32)num_cells_side);
    s32 cell_z        = (s32)(p_terrain.z * (f32)num_cells_side);
    
    if(cell_x < 0 || cell_x >= num_cells_side ||
       cell_z < 0 || cell_z >= num_cells_side)
        return result;
    
    // Normalized grid-cell coordinates, where the origin is the top-left corner of the cell.
    f32 cell_size  = 1.0f / (f32)num_cells_side;
    f32 x          = _fmodf(p_terrain.x, cell_size) / cell_size;
    f32 z          = _fmodf(p_terrain.z, cell_size) / cell_size;
    
    s32 i00        = (cell_z + 0)*num_vertices_side + (cell_x + 0);
    s32 i10        = (cell_z + 0)*num_vertices_side + (cell_x + 1);
    s32 i01        = (cell_z + 1)*num_vertices_side + (cell_x + 0);
    s32 i11        = (cell_z + 1)*num_vertices_side + (cell_x + 1);
    
    Vector3 v00    = vertices[i00];
    Vector3 v10    = vertices[i10];
    Vector3 v01    = vertices[i01];
    Vector3 v11    = vertices[i11];
    
    // (height + 0.5f) to be consistent with the terrain space. 
    Vector3 p00    = make_vector3(0.0f,  v00.y + 0.5f,  0.0f);
    Vector3 p10    = make_vector3(1.0f,  v10.y + 0.5f,  0.0f);
    Vector3 p01    = make_vector3(0.0f,  v01.y + 0.5f,  1.0f);
    Vector3 p11    = make_vector3(1.0f,  v11.y + 0.5f,  1.0f);
    
    // Bilinear interpolation. 
    p_terrain.y = lerp(lerp(p00, x, p10), z, lerp(p01, x, p11)).y;
    
    // Get the normal of the triangle we're on (CCW).
    Vector3 n = {};
    if(x <= z) // Left triangle of the grid-cell.
    {
        Vector3 v10_00 = v10 - v00; Vector3 v01_00 = v01 - v00;
        n = normalize(cross(v01_00, v10_00));
    }
    else       // Right triangle of the grid-cell.
    {
        Vector3 v10_11 = v10 - v11; Vector3 v01_11 = v01 - v11;
        n = normalize(cross(v10_11, v01_11));
    }
    
    // Transform back to world space and make the origin at the center again.
    p_terrain                -= make_vector3(0.5f);
    Matrix4 inverse_transpose = transpose(terrain->object_to_world_matrix.inverse);
    col_data_out->normal      = transform_direction(inverse_transpose, n);
    result                    = (terrain->object_to_world_matrix.non_inverse * p_terrain).y;
    
    return result;
}

FUNCTION b32 terrain_collision_world(Entity *entity, Terrain *terrain, Collision_Data *col_data_out)
{
    b32 result = false;
    
    f32 height  = get_terrain_height_world(terrain, entity->position, col_data_out);
    f32 bb_miny = entity->bb_bounds_world.min.y;
    if(bb_miny < (height + 0.01f))
    {
        col_data_out->penetration = (height - bb_miny);
        result = true;
    }
    
    col_data_out->result = result;
    return result;
}
