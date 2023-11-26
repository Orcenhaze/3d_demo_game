#include "mesh.h"

FUNCTION void load_mesh_data(Arena_List *arena_list, Triangle_Mesh *mesh, u8* file_data)
{
    // @Robustness: This seems very error prone. Can we make it more robust? 
    
    u8 *data = file_data;
    Memory_Arena *perma_arena = &arena_list->permanent_arena;
    umm attribute_size  = 0;
    
    Triangle_Mesh_Header *h = (Triangle_Mesh_Header *) data;
    data                   += sizeof(Triangle_Mesh_Header);
    
    mesh->num_vertices = h->num_vertices;
    attribute_size     = mesh->num_vertices * sizeof(Vector3);
    mesh->vertices     = (Vector3 *) PUSH_SIZE(perma_arena, attribute_size);
    MEMORY_COPY(mesh->vertices, data, attribute_size);
    data              += attribute_size;
    
    mesh->num_uvs      = h->num_uvs;
    attribute_size     = mesh->num_uvs * sizeof(Vector2);
    mesh->uvs          = (Vector2 *) PUSH_SIZE(perma_arena, attribute_size);
    MEMORY_COPY(mesh->uvs, data, attribute_size);
    data              += attribute_size;
    
    mesh->num_tbns     = h->num_tbns;
    attribute_size     = mesh->num_tbns * sizeof(TBN);
    mesh->tbns         = (TBN *) PUSH_SIZE(perma_arena, attribute_size);
    MEMORY_COPY(mesh->tbns, data, attribute_size);
    data              += attribute_size; 
    
    mesh->num_colors   = h->num_colors;
    attribute_size     = mesh->num_colors * sizeof(Vector4);
    mesh->colors       = (Vector4 *) PUSH_SIZE(perma_arena, attribute_size);
    MEMORY_COPY(mesh->colors, data, attribute_size);
    data              += attribute_size;
    
    mesh->num_indices  = h->num_indices;
    attribute_size     = mesh->num_indices * sizeof(u32);
    mesh->indices      = (u32 *) PUSH_SIZE(perma_arena, attribute_size);
    MEMORY_COPY(mesh->indices, data, attribute_size);
    data              += attribute_size;
    
    // Read triangle_list_info.
    mesh->num_triangle_lists = h->num_triangle_lists;
    attribute_size           = mesh->num_triangle_lists * sizeof(Triangle_List_Info);
    mesh->triangle_list_info = (Triangle_List_Info *) PUSH_SIZE(perma_arena, attribute_size);
    for(s32 i = 0; i < mesh->num_triangle_lists; i++)
    {
        // @Robustness: data += sizeof(material_index)...?
        mesh->triangle_list_info[i].material_index = *(s32 *) data;
        data += sizeof(s32);
        
        mesh->triangle_list_info[i].num_indices    = *(s32 *) data;
        data += sizeof(s32);
        
        mesh->triangle_list_info[i].first_index    = *(s32 *) data;
        data += sizeof(s32);
        
        mesh->triangle_list_info[i].diffuse_map    = 0;
        mesh->triangle_list_info[i].normal_map     = 0;
    }
    
    // Read material_info.
    mesh->num_materials = h->num_materials;
    attribute_size      = mesh->num_materials * sizeof(Material_Info);
    mesh->material_info = (Material_Info *) PUSH_SIZE(perma_arena, attribute_size);
    for(s32 i = 0; i < mesh->num_materials; i++)
    {
        Material_Info *m = &mesh->material_info[i];
        
        m->base_color = *(Vector4 *) data;
        data += sizeof(Vector4);
        
        m->specular = *(f32 *) data;
        data += sizeof(f32);
        
        m->roughness = *(f32 *) data;
        data += sizeof(f32);
        
        m->normal = *(Vector3 *) data;
        data += sizeof(Vector3);
        
        m->original_base_color = m->base_color;
        
        // Read number of texture_map_names.
        s32 num_texture_map_names         = *(s32 *) data;
        data += sizeof(s32);
        
        // Read characters into each texture_map_name.
        for(s32 tindex = 0; tindex < num_texture_map_names; tindex++)
        {
            // Read texture_map_name string length.
            s32 texture_name_count = *(s32 *) data;
            data += sizeof(s32);
            
            String *texture_name = &m->texture_map_names[tindex];
            *texture_name = push_string_copy(perma_arena, make_string(data, texture_name_count));
            
            data += texture_name_count;
        }
    }
}

FUNCTION void load_mesh_textures(Triangle_Mesh *mesh)
{
    for(s32 list_index = 0; list_index < mesh->num_triangle_lists; list_index++)
    {
        Triangle_List_Info *list = mesh->triangle_list_info + list_index;
        Material_Info *m         = &mesh->material_info[list->material_index];
        
        // @Robustness: Sometimes getting a random '.' at the end of the map_name.
        String map_name  = m->texture_map_names[0];
        Texture_Map *map = 0;
        if(string_empty(map_name)) 
            map = find_texture(default_white_texture);
        else
            map = find_texture(map_name);
        
        if(!map)
        {
            PRINT("Texture was not found: \"%S\" for mesh \"%S\"!\n", map_name, mesh->name);
            return;
        }
        list->diffuse_map = map;
        
        map_name = m->texture_map_names[1];
        map      = 0;
        if(string_empty(map_name)) continue;
        else map = find_texture(map_name);
        list->normal_map = map;
    }
}

FUNCTION void generate_buffers_for_mesh(Arena_List *arena_list, Triangle_Mesh *mesh)
{
    s32 num_vertices             = mesh->num_vertices;
    Vertex_XTBNUC *vertex_buffer = PUSH_ARRAY(&arena_list->temp_arena, num_vertices, Vertex_XTBNUC);
    Vertex_XTBNUC *vertex        = vertex_buffer;
    
    for(s32 vindex = 0; vindex < num_vertices; vindex++, vertex++)
    {
        if(mesh->vertices)
            vertex->position = mesh->vertices[vindex];
        else
            vertex->position = make_vector3(0.0f, 0.0f, 0.0f);
        
        if(mesh->tbns)
            vertex->tangent = mesh->tbns[vindex].tangent;
        else
            vertex->tangent = make_vector3(1.0f, 0.0f, 0.0f);
        
        if(mesh->tbns)
            vertex->bitangent = mesh->tbns[vindex].bitangent;
        else
            vertex->bitangent = make_vector3(0.0f, 1.0f, 0.0f);
        
        if(mesh->tbns)
            vertex->normal = mesh->tbns[vindex].normal;
        else
            vertex->normal = make_vector3(0.0f, 0.0f, 1.0f);
        
        if(mesh->uvs)
            vertex->uv = mesh->uvs[vindex];
        else
            vertex->uv = make_vector2(0.0f, 0.0f);
        
        if(mesh->colors)
            vertex->color = mesh->colors[vindex];
        else
            vertex->color = make_vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    extensions->glGenBuffers(1, &mesh->vbo_id);
    extensions->glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_id);
    extensions->glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_XTBNUC) * num_vertices, vertex_buffer, GL_STATIC_DRAW);
    
    extensions->glGenBuffers(1, &mesh->ibo_id);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo_id);
    extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * mesh->num_indices, mesh->indices, GL_STATIC_DRAW);
    
    clear_arena(&arena_list->temp_arena);
}

FUNCTION void generate_bounding_box_for_mesh(Triangle_Mesh *mesh)
{
    Bounding_Box *bb = &mesh->bounding_box; 
    
    Vector3 min = {MAX_F32, MAX_F32, MAX_F32};
    Vector3 max = {MIN_F32, MIN_F32, MIN_F32};
    
    for(s32 i = 0; i < mesh->num_vertices; i++)
    {
        Vector3 v = mesh->vertices[i];
        
        if(v.x < min.x) min.x = v.x;
        if(v.x > max.x) max.x = v.x;
        
        if(v.y < min.y) min.y = v.y;
        if(v.y > max.y) max.y = v.y;
        
        if(v.z < min.z) min.z = v.z;
        if(v.z > max.z) max.z = v.z;
    }
    
    Vector3 s = rect3_get_scale(make_rect3(min, max));
    Vector3 c = rect3_get_center(make_rect3(min, max));
    
    bb->bounds.min = min;
    bb->bounds.max = max;
    bb->unit_cube_to_object_matrix = 
    {
        {        
            { s.x, 0.0f, 0.0f, c.x},
            {0.0f,  s.y, 0.0f, c.y},
            {0.0f, 0.0f,  s.z, c.z},
            {0.0f, 0.0f, 0.0f, 1.0f}
        }
    };
    
    bb->vbo_id      = global_unit_cube_vbo;
    bb->ibo_id      = global_cube_line_ibo;
    bb->num_indices = ARRAY_COUNT(global_cube_line_indices);
}

FUNCTION void load_bounding_volume_data(Arena_List *arena_list, Bounding_Volume *bv, u8 *file_data)
{
    u8 *data                  = file_data;
    Memory_Arena *perma_arena = &arena_list->permanent_arena;
    umm attribute_size        = 0;
    
    Bounding_Volume_Header *h = (Bounding_Volume_Header *) data;
    data += sizeof(Bounding_Volume_Header);
    
    bv->num_vertices = h->num_vertices;
    attribute_size   = bv->num_vertices * sizeof(Vector3);
    bv->vertices     = (Vector3 *) PUSH_SIZE(perma_arena, attribute_size);
    MEMORY_COPY(bv->vertices, data, attribute_size);
    data += attribute_size;
    
    bv->num_indices  = h->num_indices;
    attribute_size   = bv->num_indices * sizeof(u32);
    bv->indices      = (u32 *) PUSH_SIZE(perma_arena, attribute_size);
    MEMORY_COPY(bv->indices, data, attribute_size);
    data += attribute_size;
    
    if(h->is_sphere == 0)
    {
        bv->num_face_normal_axes = h->num_face_normal_axes;
        attribute_size           = bv->num_face_normal_axes * sizeof(Vector3);
        bv->face_normal_axes     = (Vector3 *) PUSH_SIZE(perma_arena, attribute_size);
        MEMORY_COPY(bv->face_normal_axes, data, attribute_size);
        data += attribute_size;
        
        bv->num_edge_axes = h->num_edge_axes;
        attribute_size    = bv->num_edge_axes * sizeof(Vector3);
        bv->edge_axes     = (Vector3 *) PUSH_SIZE(perma_arena, attribute_size);
        MEMORY_COPY(bv->edge_axes, data, attribute_size);
        data += attribute_size;
    }
    else
    {
        bv->num_face_normal_axes = 0;
        bv->face_normal_axes     = 0;
        bv->num_edge_axes        = 0;
        bv->edge_axes            = 0;
    }
    
    // Initialize remaining bounding_volume members.
    bv->is_sphere = h->is_sphere;
}

FUNCTION void generate_buffers_for_bounding_volume(Arena_List *arena_list, Bounding_Volume *bv)
{
    ASSERT(bv->vertices != 0);
    ASSERT(bv->indices  != 0);
    
    extensions->glGenBuffers(1, &bv->vbo_id);
    extensions->glBindBuffer(GL_ARRAY_BUFFER, bv->vbo_id);
    extensions->glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * bv->num_vertices, bv->vertices, GL_STATIC_DRAW);
    
    extensions->glGenBuffers(1, &bv->ibo_id);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bv->ibo_id);
    extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * bv->num_indices, bv->indices, GL_STATIC_DRAW);
}

FUNCTION void load_triangle_mesh(Arena_List *arena_list, Triangle_Mesh *mesh)
{
    String file = platform_api->read_entire_file(mesh->full_path);
    
    if(!file.data)
    {
        PRINT("Load Error: couldn't load mesh data from file: %S\n", mesh->full_path);
        platform_api->free_file_memory(file.data);
        
        return;
    }
    
    String original_file = file;
    
    load_mesh_data(arena_list, mesh, file.data);
    load_mesh_textures(mesh);
    generate_buffers_for_mesh(arena_list, mesh);
    generate_bounding_box_for_mesh(mesh);
    
    platform_api->free_file_memory(original_file.data);
    
    
    //
    // Load bounding_volume data for mesh.
    //
    String bv_full_path = push_string_cat(&arena_list->temp_arena, mesh->full_path, STRING_LITERAL("bv"));
    
    file = platform_api->read_entire_file(bv_full_path);
    
    if(!file.data)
    {
        PRINT("Load Error: couldn't load bounding volume data from file: %S\n", bv_full_path);
        
        clear_arena(&arena_list->temp_arena);
        platform_api->free_file_memory(file.data);
        
        return;
    }
    
    original_file = file;
    
    load_bounding_volume_data(arena_list, &mesh->bounding_volume, file.data);
    generate_buffers_for_bounding_volume(arena_list, &mesh->bounding_volume);
    
    clear_arena(&arena_list->temp_arena);
    platform_api->free_file_memory(original_file.data);
}

FUNCTION Triangle_Mesh * find_mesh(String mesh_name)
{
    Triangle_Mesh *result = 0;
    
    // @Note: We're using internal chaining/probing.
    u64 hash = hash_string(mesh_name);
    for(u32 offset = 0;
        offset < ARRAY_COUNT(global_mesh_hash_table);
        offset++)
    {
        u32 hash_index = (hash + offset) & (ARRAY_COUNT(global_mesh_hash_table)-1);
        Mesh_Entry *entry = global_mesh_hash_table + hash_index;
        
        if(string_match(entry->key_mesh_name, mesh_name))
        {
            result = entry->mesh;
            break;
        }
    }
    
    if(!result)
    {
        PRINT("Error: couldn't find mesh %S\n", mesh_name);
    }
    
    return result;
}

FUNCTION void load_meshes_into_global_hash_table(Arena_List *arena_list, String asset_full_path_wild)
{
    Memory_Arena *arena = &arena_list->permanent_arena;
    
    File_Group_Info mesh_file_group = {};
    mesh_file_group = platform_api->load_file_group(asset_full_path_wild);
    for(u32 i = 0; i < mesh_file_group.file_count; i++)
    {
        String base_name       = make_string_from_cstring(mesh_file_group.base_names[i]);
        
        // @Note: We're using internal chaining/probing.
        u64 hash = hash_string(base_name);
        for(u32 offset = 0;
            offset < ARRAY_COUNT(global_mesh_hash_table);
            offset++)
        {
            u32 hash_index = (hash + offset) & (ARRAY_COUNT(global_mesh_hash_table)-1);
            Mesh_Entry *entry = global_mesh_hash_table + hash_index;
            
            if(!entry->mesh)
            {
                entry->mesh            = PUSH_STRUCT(arena, Triangle_Mesh);
                entry->key_mesh_name   = push_string_copy(arena, base_name);
                entry->mesh->name      = entry->key_mesh_name;
                entry->mesh->full_path = push_string_cat(arena, get_parent_directory(asset_full_path_wild), base_name);
                
                load_triangle_mesh(arena_list, entry->mesh);
                
                break;
            }
        }
    }
}