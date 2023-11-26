#ifndef WIN32_ENTRY_H
#define WIN32_ENTRY_H

#define MINIAUDIO_IMPLEMENTATION
#include "vendor/miniaudio/miniaudio.h"

// For laptops.
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 1;

struct Win32_Game_Code
{
    HMODULE  game_code_dll;
    FILETIME dll_last_write_time;
    
    game_init_T *game_init;
    game_update_T *game_update;
    game_render_T *game_render;
    
    b32 is_valid;
};

struct Win32_State
{
    u64   total_memory_size;
    void *game_memory_block;
    
    char  exe_full_path[MAX_STRING];
    s32   exe_parent_directory_full_path_length;
    
    char  data_directory_full_path[MAX_STRING];
};

struct Miniaudio_State
{
    ma_decoder       decoder;
    ma_device_config device_config;
    ma_device        device;
};
Miniaudio_State global_miniaudio_state;

#endif //WIN32_ENTRY_H
