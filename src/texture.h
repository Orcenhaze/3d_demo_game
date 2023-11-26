#ifndef TEXTURE_H
#define TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image/stb_image.h"

String default_white_texture = STRING_LITERAL("default_white_texture");

struct Texture_Map
{
    GLuint id;
    
    GLsizei width, height;
    s32     bytes_per_pixel; 
    GLint   internal_format;
    GLenum  data_format;
    
    u8  *data;
    String full_path;
};

struct Cube_Map
{
    GLuint id;
    
    String directory_full_path;
    
    GLuint vbo_id;
    GLuint ibo_id;
    s32    num_indices;
};

struct Texture_Entry
{
    String key_texture_name;
    Texture_Map *texture;
};
GLOBAL Texture_Entry global_texture_hash_table[64]; // Power of 2.

FUNCTION Vector4 get_pixel(Texture_Map *map, s32 row, s32 col)
{
    ASSERT(map && map->data);
    
    Vector4 result = {};
    
    if(row < 0 || row >= map->height ||
       col < 0 || col >= map->width)
        return result;
    
    // @Note: We are forcing 4-bytes per pixel when loading an image.
    s32 bytes_per_pixel = 4;
    
    u8 *pixel = map->data + (row*map->width + col) * bytes_per_pixel;
    result.r  = pixel[0];
    result.g  = pixel[1];
    result.b  = pixel[2];
    result.a  = pixel[3];
    
    return result;
}

FUNCTION void load_cube_map(Cube_Map *map)
{
    map->vbo_id      = global_bilateral_cube_vbo;
    map->ibo_id      = global_cube_triangle_ibo;
    map->num_indices = ARRAY_COUNT(global_cube_triangle_indices);
    
    // @Hardcode: Cube maps have to be JPGs.
#if 0
    char *cube_map_faces[] = 
    {
        "right.jpg",  // +X
        "left.jpg",   // -X
        "top.jpg",    // +Y
        "bottom.jpg", // -Y
        "back.jpg",   // +Z
        "front.jpg",  // -Z
    };
#else
    char *cube_map_faces[] = 
    {
        "px.jpg", // +X
        "nx.jpg", // -X
        "py.jpg", // +Y
        "ny.jpg", // -Y
        "pz.jpg", // +Z
        "nz.jpg", // -Z
    };
#endif
    
    extensions->glGenTextures(1, &map->id);
	extensions->glBindTexture(GL_TEXTURE_CUBE_MAP, map->id);
	
    for(u32 i = 0; i < 6; i++)
    {
        char full_path[MAX_STRING];
        string_format(full_path, sizeof(full_path), "%S%s", map->directory_full_path, cube_map_faces[i]);
        
        s32 width, height, bpp;
        u8 *data = stbi_load(full_path, &width, &height, &bpp, 4);
        if(data)
        {
            extensions->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                                     0, 
                                     GL_RGBA8, 
                                     width, height, 0, 
                                     GL_RGBA, 
                                     GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            PRINT("STBI Error: image failed to load! Path: %s\n", full_path);
            stbi_image_free(data);
        }
    }
    
    //extensions->glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    extensions->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	extensions->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	extensions->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	extensions->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	extensions->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    extensions->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

FUNCTION void load_default_white_texture(Texture_Map *map)
{
    u8 white_texture[4]      = {0xFF, 0xFF, 0xFF, 0xFF};
    map->width = map->height = 1;
    map->bytes_per_pixel     = 4;
    map->data                = white_texture;
    
    extensions->glGenTextures(1, &map->id);
    extensions->glBindTexture(GL_TEXTURE_2D, map->id);
    
    map->internal_format = GL_RGBA8;
    map->data_format     = GL_RGBA;
    
    extensions->glTexImage2D(GL_TEXTURE_2D, 0, map->internal_format, map->width, map->height, 0, map->data_format, GL_UNSIGNED_BYTE, map->data);
    
    extensions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    extensions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    extensions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    extensions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    
    extensions->glBindTexture(GL_TEXTURE_2D, 0);
    map->data = 0;
}

FUNCTION void load_texture(Texture_Map *map)
{
    // @Note: Both UV coordinates and first byte in image are top-left.
	map->data = stbi_load((char *)map->full_path.data, 
                          &map->width, &map->height, 
                          &map->bytes_per_pixel, 4);
    
    if(map->data)
    {
        extensions->glGenTextures(1, &map->id);
        extensions->glBindTexture(GL_TEXTURE_2D, map->id);
        
        map->internal_format = GL_RGBA8;
        map->data_format     = GL_RGBA;
        
        extensions->glTexImage2D(GL_TEXTURE_2D, 0, map->internal_format, map->width, map->height, 0, map->data_format, GL_UNSIGNED_BYTE, map->data);
        extensions->glGenerateMipmap(GL_TEXTURE_2D);
        
		extensions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		extensions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		extensions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
		extensions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
        
        extensions->glBindTexture(GL_TEXTURE_2D, 0);
		//stbi_image_free(map->data);
	}
	else
	{
        PRINT("STBI Error: image failed to load! Path: %S\n", map->full_path);
		stbi_image_free(map->data);
	}
}

FUNCTION void bind_texture(GLuint id, GLenum texture_unit = 0, GLenum target = GL_TEXTURE_2D)
{
    extensions->glActiveTexture(GL_TEXTURE0 + texture_unit);
    extensions->glBindTexture(target, id);
};

FUNCTION Texture_Map * find_texture(String texture_name)
{
    Texture_Map *result = 0;
    
    // @Note: We're using internal chaining/probing.
    u64 hash = hash_string(texture_name);
    for(u32 offset = 0;
        offset < ARRAY_COUNT(global_texture_hash_table);
        offset++)
    {
        u32 hash_index = (hash + offset) & (ARRAY_COUNT(global_texture_hash_table)-1);
        Texture_Entry *entry = global_texture_hash_table + hash_index;
        
        if(string_match(entry->key_texture_name, texture_name))
        {
            result = entry->texture;
            break;
        }
    }
    
    if(!result)
    {
        PRINT("Error: couldn't find texture %S\n", texture_name);
    }
    
    return result;
}

FUNCTION void load_textures_into_global_hash_table(Arena_List *arena_list, String asset_full_path_wild)
{
    Memory_Arena *arena = &arena_list->permanent_arena;
    
    // Store the default white texture in the hash table.
    {
        u64 hash                     = hash_string(default_white_texture);
        u32 hash_index               = hash & (ARRAY_COUNT(global_texture_hash_table)-1);
        Texture_Entry *hash_table    = global_texture_hash_table + hash_index;
        
        hash_table->texture = PUSH_STRUCT(arena, Texture_Map);
        hash_table->key_texture_name = default_white_texture;
        
        load_default_white_texture(hash_table->texture);
    }
    
    File_Group_Info texture_file_group = {};
    texture_file_group = platform_api->load_file_group(asset_full_path_wild);
    for(u32 i = 0; i < texture_file_group.file_count; i++)
    {
        String base_name = make_string_from_cstring(texture_file_group.base_names[i]);
        
        // @Note: We're using internal chaining/probing.
        u64 hash = hash_string(base_name);
        for(u32 offset = 0;
            offset < ARRAY_COUNT(global_texture_hash_table);
            offset++)
        {
            u32 hash_index = (hash + offset) & (ARRAY_COUNT(global_texture_hash_table)-1);
            Texture_Entry *entry = global_texture_hash_table + hash_index;
            
            if(!entry->texture)
            {
                entry->texture            = PUSH_STRUCT(arena, Texture_Map);
                entry->key_texture_name   = push_string_copy(arena, base_name);
                entry->texture->full_path = push_string_cat(arena, get_parent_directory(asset_full_path_wild), base_name);
                
                load_texture(entry->texture);
                
                break;
            }
        }
    }
}

FUNCTION void set_texture_map(GLuint id, GLenum texture_unit = 0, GLint location = -1, GLenum target = GL_TEXTURE_2D)
{
    // @Note: To unbind a texture, pass (id = 0) to this function.
    
    bind_texture(id, texture_unit, target);
    
    if((id) && (current_shader) && (location != -1))
        extensions->glUniform1i(location, texture_unit);
}

#endif //TEXTURE_H
