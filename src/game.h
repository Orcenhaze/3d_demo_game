#ifndef GAME_H
#define GAME_H

#include "platform.h"
#include "intrinsics.h"
#include "math.h"
#include "shared.h"

GLOBAL char *global_data_directory_full_path;

struct Arena_List
{
    Memory_Arena permanent_arena; // Lives through the life-time of the program.
    Memory_Arena temp_arena;      // Cleared manually when you finish operating on it.
    Memory_Arena level_arena;     // Specific for *.level data.
};

#include "opengl.cpp"
#include "texture.h"

#include "mesh.h"
#include "terrain.h"
#include "collision.h"
#include "entity.h"

#include "lexer.h"
#include "editor.h"

struct Camera
{
    Vector3 position;
    Vector3 front;
    Vector3 up;
};

struct Game_State
{
    Arena_List arena_list;
    
    // local.variables data.
    String current_level_full_path;
    
    // Level data.
    // @Note: First entity in all entity arrays is an invalid entity.
    u32      level_entity_count;
    Entity  *level_entities;
    Terrain  terrain;
    Cube_Map skybox;
    String   music_file_name;
    
    u32      num_point_light_entities;
    Entity  *point_light_entities[MAX_POINT_LIGHTS];
    
#define MAX_PROJECTILE_COUNT 3
    u32     next_projectile;
    Entity  projectiles[MAX_PROJECTILE_COUNT];
    
    // Camera.
    Vector3 offset_camera;
    b32     invert_camera;
    b32     use_debug_camera;
    Camera  game_camera;
    Camera  debug_camera;
    Camera *active_camera;
    
    Vector3 old_mouse_ndc;
    Vector3 mouse_world;
    
    // Editor.
    Selected_Entity selected_entity; // @Todo: Multiple entity selection.
    
#define MAX_EDITOR_ENTITY_COUNT 64
    u32 next_editor_entity;
    Entity editor_entities[MAX_EDITOR_ENTITY_COUNT];
};
GLOBAL Entity *global_player = 0;

#define RED      make_vector4(1.0f, 0.0f, 0.0f, 1.0f)
#define GREEN    make_vector4(0.0f, 1.0f, 0.0f, 1.0f)
#define BLUE     make_vector4(0.0f, 0.0f, 1.0f, 1.0f)

#endif //GAME_H
