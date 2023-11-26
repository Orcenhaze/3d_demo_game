#ifndef PLATFORM_H
#define PLATFORM_H

#include "types.h"
#include "memory.h"
#include "string.h"
#include "opengl_extensions.h"

//
// @Note: Platform services to game.
//
#define MAX_FILES_IN_GROUP 64
struct File_Group_Info
{
    u32  file_count;
    char base_names[MAX_FILES_IN_GROUP][MAX_STRING];
};

typedef File_Group_Info platform_load_file_group_T(String file_name);
typedef void platform_free_file_memory_T(void *memory);
typedef String platform_read_entire_file_T(String full_path);
typedef b32 platform_write_entire_file_T(String full_path, String memory);
typedef void platform_print_to_console_T(char *source, umm source_count);
typedef void platform_audio_init_and_start_T(String audio_full_path);

struct Platform_API
{
    platform_free_file_memory_T     *free_file_memory;
    platform_load_file_group_T      *load_file_group;
    platform_read_entire_file_T     *read_entire_file;
    platform_write_entire_file_T    *write_entire_file;
    platform_print_to_console_T     *print_to_console;
    platform_audio_init_and_start_T *audio_init_and_start;
};
Platform_API *platform_api;

enum
{
    Key_NONE = 0,
    Key_Q = 1,
    Key_W = 2,
    Key_E = 3,
    Key_R = 4,
    Key_T = 5,
    Key_Y = 6,
    Key_U = 7,
    Key_I = 8,
    Key_O = 9,
    Key_P = 10,
    Key_A = 11,
    Key_S = 12,
    Key_D = 13,
    Key_F = 14,
    Key_G = 15,
    Key_H = 16,
    Key_J = 17,
    Key_K = 18,
    Key_L = 19,
    Key_Z = 20,
    Key_X = 21,
    Key_C = 22,
    Key_V = 23,
    Key_B = 24,
    Key_N = 25,
    Key_M = 26,
    Key_ALT     = 27,
    Key_SHIFT   = 28,
    Key_SPACE   = 29,
    Key_ESCAPE  = 30,
    Key_CONTROL = 31,
    
    Key_COUNT   = 32
};

enum
{
    MouseButton_LEFT,
    MouseButton_RIGHT,
    MouseButton_MIDDLE,
    
    MouseButton_COUNT
};

struct Button_State
{
    s32 half_transition_count; // Per-frame.
    b32 ended_down;            // Across frames.
    
    // Incremented by delta_time at the end of every frame iff ended_down was true.
    f32 down_counter_seconds;
};

struct Game_Input
{
    Button_State mouse_buttons[MouseButton_COUNT];
    Vector3      mouse_ndc;
    
    Button_State keyboard_buttons[Key_COUNT];
    
    f32 delta_time;
    
#if INTERNAL_BUILD
    b32 shift_down, ctrl_down, alt_down;
    b32 fk_pressed[13]; // F1-F12 (0 is not used).
    b32 del_pressed;
#endif
};

inline b32 is_down(Button_State bs)
{
    b32 result = bs.ended_down;
    
    return result;
}

inline b32 is_down(Button_State *bs, f32 duration_until_trigger_seconds)
{
    b32 result = false;
    
    if(bs->down_counter_seconds > duration_until_trigger_seconds)
    {
        result = true;
        bs->down_counter_seconds -= duration_until_trigger_seconds;
    }
    
    return result;
}

inline b32 was_pressed(Button_State bs)
{
    b32 result = (bs.half_transition_count > 1) || 
    ((bs.half_transition_count == 1) && (bs.ended_down)); 
    
    return result;
}

struct Settings
{
    Vector2u window_dimensions;
    Vector2u render_dimensions;
    Rect2i   drawing_dimensions;
};

struct Game_Memory
{
    u64   permanent_storage_size;
    void *permanent_storage;
    
    Settings settings;
    
    Platform_API      platform_api;
    Opengl_Extensions gl_extensions;
    
    char *data_directory_full_path;
};

//
// @Note: Game services to platform.
//

typedef void game_init_T(Game_Memory *memory);
typedef void game_update_T(Game_Memory *memory, Game_Input *input);
typedef void game_render_T(Game_Memory *memory, Game_Input *input);

#endif //PLATFORM_H
