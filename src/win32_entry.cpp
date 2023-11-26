// @REMINDER:
// Always acknowledge the code you steal from other places! I don't particularly enjoy 
// having code that I didn't write myself, but I have nonetheless learned a lot from 
// looking at code by programmers I consider great.

// @Note: The core architecture of the code is based off the Handmade Hero series made by the 
// admirable programmer and educator Casey Muratori.

#include "platform.h"
#include "intrinsics.h"
#include "math.h"
#include "shared.h"

#include <windows.h>
#include <gl/gl.h>

#include "win32_entry.h"

GLOBAL b32             global_running;
GLOBAL WINDOWPLACEMENT global_window_position = {sizeof(global_window_position)};
GLOBAL s64 global_perf_count_frequency;
GLOBAL b32 game_initialized;

FUNCTION void win32_cat_strings(char *dest, umm dest_count,
                                char *a,    umm a_count,
                                char *b,    umm b_count)
{
    ASSERT(a_count+b_count < dest_count);
    
    for(umm i = 0; i < a_count; i++)
    {
        *dest++ = *a++;
    }
    
    for(umm i = 0; i < b_count; i++)
    {
        *dest++ = *b++;
    }
    
    *dest++ = 0;
}

FUNCTION void win32_fix_path_slashes(char *str)
{
    while(*str)
    {
        if(*str == '/')
        {
            *str = '\\';
        }
        
        str++;
    }
}

FUNCTION void win32_get_exe_full_path(Win32_State *win32_state)
{
    DWORD exe_full_path_length = GetModuleFileName(0, win32_state->exe_full_path, 
                                                   sizeof(win32_state->exe_full_path));
    
    char *exe_base_name = 0;
    for(char *scan = win32_state->exe_full_path; *scan; scan++)
    {
        if(*scan == '\\')
        {
            exe_base_name = scan + 1;
        }
    }
    
    win32_state->exe_parent_directory_full_path_length = (s32)(exe_base_name - win32_state->exe_full_path);
}

FUNCTION void win32_build_full_path(Win32_State *win32_state, 
                                    char *file_name,
                                    char *dest, umm dest_count)
{
    win32_cat_strings(dest, dest_count,
                      win32_state->exe_full_path, win32_state->exe_parent_directory_full_path_length,
                      file_name, string_length(file_name));
}

FUNCTION void win32_toggle_fullsceen(HWND window)
{
    // @Note: Fullscreen toggling as suggested by Raymond Chen, see:
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    
    DWORD style = GetWindowLong(window, GWL_STYLE);
    if(style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitor_info = {sizeof(monitor_info)};
        if(GetWindowPlacement(window, &global_window_position) &&
           GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
        {
            SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP,
                         monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &global_window_position);
        SetWindowPos(window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

FUNCTION Vector2u win32_get_window_dimensions(HWND window)
{
    Vector2u result;
    
    RECT rect;
    GetClientRect(window, &rect);
    result.width  = rect.right  - rect.left;
    result.height = rect.bottom - rect.top;
    
    return result;
}

FUNCTION void win32_print_to_console(char *source, umm source_count)
{
    if(!WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), source, (DWORD)source_count, 0, 0))
    {
        OS_PRINT("OS Error: couldn't write to console!\n");
    }
}

FUNCTION File_Group_Info win32_load_file_group(String file_name)
{
    win32_fix_path_slashes((char*)file_name.data);
    
    File_Group_Info result = {};
    s32 file_count = 0;
    
    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFile((char*)file_name.data, &find_data); 
    while(find_handle != INVALID_HANDLE_VALUE)
    {
        char *src = find_data.cFileName;
        MEMORY_COPY(result.base_names[file_count++], src, string_length(src));
        
        if(!FindNextFile(find_handle, &find_data)) break;
    }
    
    result.file_count = file_count;
    
    return result;
}

FUNCTION void win32_free_file_memory(void *memory)
{
    if(memory)
    {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

FUNCTION String win32_read_entire_file(String full_path)
{
    win32_fix_path_slashes((char*)full_path.data);
    
    String result = {};
    
    HANDLE file_handle = CreateFile((char*)full_path.data, GENERIC_READ, FILE_SHARE_READ, 0, 
                                    OPEN_EXISTING, 0, 0);
    if(file_handle == INVALID_HANDLE_VALUE)
    {
        OS_PRINT("OS Error: read_entire_file() INVALID_HANDLE_VALUE!\n");
        return result;
    }
    
    LARGE_INTEGER file_size64;
    if(GetFileSizeEx(file_handle, &file_size64) == 0)
    {
        OS_PRINT("OS Error: read_entire_file() GetFileSizeEx() failed!\n");
    }
    
    u32 file_size32 = (u32)file_size64.QuadPart;
    result.data = (u8 *) VirtualAlloc(0, file_size32, MEM_RESERVE|MEM_COMMIT, 
                                      PAGE_READWRITE);
    
    if(!result.data)
    {
        OS_PRINT("OS Error: read_entire_file() VirtualAlloc() returned 0!\n");
    }
    
    DWORD bytes_read;
    if(ReadFile(file_handle, result.data, file_size32, &bytes_read, 0) && (file_size32 == bytes_read))
    {
        result.count = file_size32;
    }
    else
    {
        OS_PRINT("OS Error: read_entire_file() ReadFile() failed!\n");
        
        win32_free_file_memory(result.data);
        result.data = 0;
    }
    
    CloseHandle(file_handle);
    
    return result;
}

FUNCTION b32 win32_write_entire_file(String full_path, String memory)
{
    win32_fix_path_slashes((char*)full_path.data);
    
    b32 result = false;
    
    HANDLE file_handle = CreateFile((char*)full_path.data, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(file_handle == INVALID_HANDLE_VALUE)
    {
        OS_PRINT("OS Error: write_entire_file() INVALID_HANDLE_VALUE!\n");
    }
    
    DWORD bytes_written;
    if(WriteFile(file_handle, memory.data, (DWORD)memory.size, &bytes_written, 0) && (bytes_written == memory.size))
    {
        result = true;
    }
    else
    {
        OS_PRINT("OS Error: write_entire_file() WriteFile() failed!\n");
    }
    
    CloseHandle(file_handle);
    
    return result;
}

// @Todo: Enable sRGB framebuffer for gamma-correction. 
// We need to get the extensions "WGL_EXT_framebuffer_sRGB" and "WGL_ARB_framebuffer_sRGB".
//
// OpenGL.
//
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_TYPE_RGBA_ARB                         0x202B
#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB          0x20A9
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_SAMPLE_BUFFERS_ARB                    0x2041
#define WGL_SAMPLES_ARB                           0x2042

typedef HGLRC WINAPI wglCreateContextAttribsARB_T(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL WINAPI  wglChoosePixelFormatARB_T(HDC hdc,
                                               const int *piAttribIList,
                                               const FLOAT *pfAttribFList,
                                               UINT nMaxFormats,
                                               int *piFormats,
                                               UINT *nNumFormats);
typedef BOOL WINAPI         wglSwapIntervalEXT_T(int interval);
typedef const char * WINAPI wglGetExtensionsStringEXT_T(void);

GLOBAL wglCreateContextAttribsARB_T *wglCreateContextAttribsARB;
GLOBAL wglChoosePixelFormatARB_T    *wglChoosePixelFormatARB;
GLOBAL wglSwapIntervalEXT_T         *wglSwapIntervalEXT;
GLOBAL wglGetExtensionsStringEXT_T  *wglGetExtensionsStringEXT;

typedef GLuint APIENTRY glCreateShader_T(GLenum type);
typedef void   APIENTRY glShaderSource_T(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void   APIENTRY glAttachShader_T(GLuint program, GLuint shader);
typedef void   APIENTRY glCompileShader_T(GLuint shader);
typedef void   APIENTRY glGetShaderiv_T(GLuint shader, GLenum pname, GLint *params);
typedef GLuint APIENTRY glCreateProgram_T(void);
typedef void   APIENTRY glLinkProgram_T(GLuint program);
typedef void   APIENTRY glUseProgram_T(GLuint program);
typedef void   APIENTRY glGetShaderInfoLog_T(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   APIENTRY glValidateProgram_T(GLuint program);
typedef void   APIENTRY glGetProgramiv_T(GLuint program, GLenum pname, GLint *params);
typedef void   APIENTRY glGetProgramInfoLog_T(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   APIENTRY glDeleteShader_T(GLuint shader);
typedef GLint  APIENTRY glGetAttribLocation_T(GLuint program, const GLchar *name);
typedef void   APIENTRY glGenVertexArrays_T(GLsizei n, GLuint *arrays);
typedef void   APIENTRY glBindVertexArray_T(GLuint array);
typedef void   APIENTRY glGenBuffers_T(GLsizei n, GLuint *buffers);
typedef void   APIENTRY glUniform1f_T(GLint location, GLfloat v0);
typedef void   APIENTRY glUniform1i_T(GLint location, GLint v0);
typedef void   APIENTRY glUniform3fv_T(GLint location, GLsizei count, const GLfloat *value);
typedef void   APIENTRY glUniform4fv_T(GLint location, GLsizei count, const GLfloat *value);
typedef void   APIENTRY glUniformMatrix4fv_T(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef GLint  APIENTRY glGetUniformLocation_T(GLuint program, const GLchar *name);
typedef void   APIENTRY glBindBuffer_T(GLenum target, GLuint buffer);
typedef void   APIENTRY glBufferData_T(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void   APIENTRY glVertexAttribPointer_T(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

typedef void   APIENTRY glDepthFunc_T(GLenum func);
typedef void   APIENTRY glBlendFunc_T(GLenum sfactor, GLenum dfactor);
typedef void   APIENTRY glDisable_T(GLenum cap);
typedef void   APIENTRY glEnable_T(GLenum cap);

typedef void   APIENTRY glEnableVertexAttribArray_T(GLuint index);
typedef void   APIENTRY glDisableVertexAttribArray_T(GLuint index);
typedef void   APIENTRY glDrawArrays_T(GLenum mode, GLint first, GLsizei count);
typedef void   APIENTRY glDrawElements_T(GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void   APIENTRY glDrawElementsInstanced_T(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
typedef GLuint APIENTRY glGetDebugMessageLogARB_T(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);

typedef void   APIENTRY glGenTextures_T(GLsizei n, GLuint *textures);
typedef void   APIENTRY glGenerateMipmap_T(GLenum target);
typedef void   APIENTRY glTexImage2D_T(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void   APIENTRY glTexParameteri_T(GLenum target, GLenum pname, GLint param);
typedef void   APIENTRY glTextureParameteri_T (GLuint texture, GLenum pname, GLint param);
typedef void   APIENTRY glActiveTexture_T(GLenum texture);
typedef void   APIENTRY glBindTexture_T(GLenum target, GLuint texture);
typedef void   APIENTRY glPolygonMode_T(GLenum face, GLenum mode);
typedef void   APIENTRY glClear_T(GLbitfield mask);

FUNCTION void win32_set_pixel_format(HDC window_dc)
{
    int  suggested_pixel_format_index = 0;
    UINT number_of_formats            = 0;
    
    if(wglChoosePixelFormatARB)
    {
        int int_attrib_list[] = 
        {
            WGL_DRAW_TO_WINDOW_ARB,           GL_TRUE,
            WGL_ACCELERATION_ARB,             WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB,           GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,            GL_TRUE,
            WGL_PIXEL_TYPE_ARB,               WGL_TYPE_RGBA_ARB,
            WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
            WGL_DEPTH_BITS_ARB,               24,
            WGL_SAMPLE_BUFFERS_ARB,           GL_TRUE,
            WGL_SAMPLES_ARB,                  4,
            0,
        };
        float float_attrib_list[] = {0};
        
        wglChoosePixelFormatARB(window_dc, int_attrib_list, float_attrib_list, 1, &suggested_pixel_format_index, &number_of_formats);
    }
    
    if(!number_of_formats)
    {
        PIXELFORMATDESCRIPTOR desired_pixel_format = {};
        desired_pixel_format.nSize      = sizeof(desired_pixel_format);
        desired_pixel_format.nVersion   = 1;
        desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
        desired_pixel_format.dwFlags    = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
        desired_pixel_format.cColorBits = 32;
        desired_pixel_format.cAlphaBits = 8;
        desired_pixel_format.iLayerType = PFD_MAIN_PLANE;
        
        suggested_pixel_format_index    = ChoosePixelFormat(window_dc, &desired_pixel_format);
    }
    
    PIXELFORMATDESCRIPTOR suggested_pixel_format;
    DescribePixelFormat(window_dc, suggested_pixel_format_index, sizeof(suggested_pixel_format), &suggested_pixel_format);
    SetPixelFormat(window_dc, suggested_pixel_format_index, &suggested_pixel_format);
}

FUNCTION void win32_load_wgl_extensions()
{
    WNDCLASS temp_window_class      = {};
    temp_window_class.lpfnWndProc   = DefWindowProcA;
    temp_window_class.hInstance     = GetModuleHandle(0);
    temp_window_class.lpszClassName = "WGLWindowClass";
    
    if(!RegisterClassA(&temp_window_class))
    {
        OS_PRINT("OS Error: win32_load_wgl_extensions() RegisterClassA() failed!\n");
        return;
    }
    
    HWND temp_window = CreateWindowExA(
                                       0,
                                       temp_window_class.lpszClassName,
                                       "Temp Window",
                                       0,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       0,
                                       0,
                                       temp_window_class.hInstance,
                                       0);
    HDC hdc = GetDC(temp_window);
    win32_set_pixel_format(hdc);
    HGLRC temp_opengl_rc = wglCreateContext(hdc);
    
    if(wglMakeCurrent(hdc, temp_opengl_rc))
    {
        wglCreateContextAttribsARB = (wglCreateContextAttribsARB_T *) wglGetProcAddress("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB    = (wglChoosePixelFormatARB_T *)    wglGetProcAddress("wglChoosePixelFormatARB");
        wglSwapIntervalEXT         = (wglSwapIntervalEXT_T *)         wglGetProcAddress("wglSwapIntervalEXT");
        wglGetExtensionsStringEXT  = (wglGetExtensionsStringEXT_T *)  wglGetProcAddress("wglGetExtensionsStringEXT");
    }
    
    wglDeleteContext(temp_opengl_rc);
    ReleaseDC(temp_window, hdc);
    DestroyWindow(temp_window);
}

FUNCTION Opengl_Extensions win32_opengl_init(HDC window_dc)
{
    // @Note: By default, Windows only supports OpenGL 1.1. other versions are implemented 
    // by hardware vendors. To use their drivers, wglCreateContextAttribsARB() must be called.
    // That function binding however doesn't exist by default. To get it, you need to call 
    // wglGetProcAddress(), which you comically can't call without having already made an 
    // OpenGL rendering context. So we need to create a default context and ask if 
    // wglCreateContextAttribsARB() exists, if yes, call it, switch to it 
    // using wglMakeCurrent() and delete the old context using wglDeleteContext().
    //
    // OpenGL has other extended attributes that are set by the window's pixel format that
    // we get using wglChoosePixelFormatARB() instead of the default ChoosePixelFormat(). 
    // Since we can't change the pixel format once we set it, this requires us to create a 
    // temporary window where we query through it for modern OpenGL using wglGetProcAddress() 
    // and based on what we get, set the pixel format for out main window appropriately.
    
    GLOBAL int opengl_attribute_list[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if DEBUG_BUILD
        |WGL_CONTEXT_DEBUG_BIT_ARB
#endif
        ,
#if 0
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#else
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
        0,
    };
    
    win32_load_wgl_extensions();
    
    HGLRC opengl_rc = 0;
    win32_set_pixel_format(window_dc);
    
    if(wglCreateContextAttribsARB)
    {
        opengl_rc = wglCreateContextAttribsARB(window_dc, 0, opengl_attribute_list);
    }
    
    if(!opengl_rc)
    {
        opengl_rc = wglCreateContext(window_dc);
    }
    
    Opengl_Extensions gl_extensions = {};
    
    if(wglMakeCurrent(window_dc, opengl_rc))
    {
        
        HMODULE opengl_dll = LoadLibrary("opengl32.dll");
        
#define GL_GET_EXTENSION(name) \
gl_extensions.name = (name##_T *) wglGetProcAddress(#name); \
if(!gl_extensions.name) gl_extensions.name = (name##_T *) GetProcAddress(opengl_dll, #name); \
if(!gl_extensions.name) ASSERT(false)
        
        GL_GET_EXTENSION(glCreateShader);
        GL_GET_EXTENSION(glShaderSource);
        GL_GET_EXTENSION(glAttachShader);
        GL_GET_EXTENSION(glCompileShader);
        GL_GET_EXTENSION(glGetShaderiv);
        GL_GET_EXTENSION(glCreateProgram);
        GL_GET_EXTENSION(glLinkProgram);
        GL_GET_EXTENSION(glUseProgram);
        GL_GET_EXTENSION(glGetShaderInfoLog);
        GL_GET_EXTENSION(glValidateProgram);
        GL_GET_EXTENSION(glGetProgramiv);
        GL_GET_EXTENSION(glGetProgramInfoLog);
        GL_GET_EXTENSION(glDeleteShader);
        GL_GET_EXTENSION(glGetAttribLocation);
        GL_GET_EXTENSION(glEnableVertexAttribArray);
        GL_GET_EXTENSION(glDisableVertexAttribArray);
        GL_GET_EXTENSION(glVertexAttribPointer);
        GL_GET_EXTENSION(glGenVertexArrays);
        GL_GET_EXTENSION(glBindVertexArray);
        GL_GET_EXTENSION(glGenBuffers);
        GL_GET_EXTENSION(glBindBuffer);
        GL_GET_EXTENSION(glBufferData);
        GL_GET_EXTENSION(glGetDebugMessageLogARB);
        
        GL_GET_EXTENSION(glDepthFunc);
        GL_GET_EXTENSION(glBlendFunc);
        GL_GET_EXTENSION(glDisable);
        GL_GET_EXTENSION(glEnable);
        
        GL_GET_EXTENSION(glGenTextures);
        GL_GET_EXTENSION(glGenerateMipmap);
        GL_GET_EXTENSION(glTexImage2D);
        GL_GET_EXTENSION(glTexParameteri);
        GL_GET_EXTENSION(glActiveTexture);
        GL_GET_EXTENSION(glBindTexture);
        
        GL_GET_EXTENSION(glDrawArrays);
        GL_GET_EXTENSION(glDrawElements);
        GL_GET_EXTENSION(glDrawElementsInstanced);
        
        GL_GET_EXTENSION(glGetUniformLocation);
        GL_GET_EXTENSION(glUniform1f);
        GL_GET_EXTENSION(glUniform1i);
        GL_GET_EXTENSION(glUniform3fv);
        GL_GET_EXTENSION(glUniform4fv);
        GL_GET_EXTENSION(glUniformMatrix4fv);
        GL_GET_EXTENSION(glPolygonMode);
        GL_GET_EXTENSION(glClear);
        
        // Turn on vsync.
        if(wglSwapIntervalEXT)
        {
            wglSwapIntervalEXT(1);
        }
    }
    
    // @Todo: You need to return opengl_rc if you want to share this context with other threads.
    return gl_extensions;
}

FUNCTION void win32_process_button_state(Button_State *button_state, b32 is_down)
{
    if(button_state->ended_down != is_down)
    {
        button_state->ended_down = is_down;
        button_state->half_transition_count++;
    }
}

FUNCTION void win32_process_pending_messages(Game_Input *new_input)
{
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_QUIT:
            {
                global_running = 0;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 vk_code          = (u32) message.wParam;
                b32 alt_key_was_down = (message.lParam & (1 << 29));
                b32 was_down         = (message.lParam & (1 << 30)) != 0;
                b32 is_down          = (message.lParam & (1 << 31)) == 0;
                if(was_down != is_down)
                {
                    switch(vk_code)
                    {
                        case 'Q':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_Q], is_down);
                        } break;
                        case 'W':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_W], is_down);
                        } break;
                        case 'E':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_E], is_down);
                        } break;
                        case 'R':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_R], is_down);
                        } break;
                        case 'T':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_T], is_down);
                        } break;
                        case 'Y':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_Y], is_down);
                        } break;
                        case 'U':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_U], is_down);
                        } break;
                        case 'I':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_I], is_down);
                        } break;
                        case 'O':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_O], is_down);
                        } break;
                        case 'P':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_P], is_down);
                        } break;
                        case 'A':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_A], is_down);
                        } break;
                        case 'S':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_S], is_down);
                        } break;
                        case 'D':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_D], is_down);
                        } break;
                        case 'F':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_F], is_down);
                        } break;
                        case 'G':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_G], is_down);
                        } break;
                        case 'H':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_H], is_down);
                        } break;
                        case 'J':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_J], is_down);
                        } break;
                        case 'K':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_K], is_down);
                        } break;
                        case 'L':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_L], is_down);
                        } break;
                        case 'Z':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_Z], is_down);
                        } break;
                        case 'X':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_X], is_down);
                        } break;
                        case 'C':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_C], is_down);
                        } break;
                        case 'V':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_V], is_down);
                        } break;
                        case 'B':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_B], is_down);
                        } break;
                        case 'N':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_N], is_down);
                        } break;
                        case 'M':
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_M], is_down);
                        } break;
                        case VK_MENU:
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_ALT], is_down);
                        } break;
                        case VK_SHIFT:
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_SHIFT], is_down);
                        } break;
                        case VK_SPACE:
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_SPACE], is_down);
                        } break;
                        case VK_ESCAPE:
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_ESCAPE], is_down);
                        } break;
                        case VK_CONTROL:
                        {
                            win32_process_button_state(&new_input->keyboard_buttons[Key_CONTROL], is_down);
                        } break;
                    }
                    
                    
                    if(is_down)
                    {
                        if((vk_code == VK_F4) && alt_key_was_down)
                        {
                            global_running = false;
                        }
                        
                        else if((vk_code == VK_RETURN) && alt_key_was_down)
                        {
                            if(message.hwnd)
                            {
                                win32_toggle_fullsceen(message.hwnd);
                            }
                        }
                        
#if INTERNAL_BUILD
                        else if((vk_code >= VK_F1) && (vk_code <= VK_F12))
                        {
                            new_input->fk_pressed[vk_code - VK_F1 + 1] = true;
                        }
                        
                        else if(vk_code == VK_DELETE)
                        {
                            new_input->del_pressed = true;
                        }
#endif
                    }
                }
                
            } break;
            
            default:
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            } break;
        }
    }
}

LRESULT CALLBACK win32_main_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;
    
    switch(message)
    {
        case WM_CLOSE:
        {
            global_running = false;
        } break;
        
        case WM_DESTROY:
        {
            global_running = false;
        } break;
        
        default:
        {
            result = DefWindowProcA(window, message, wparam, lparam);
        } break;
    }
    
    return result;
}

inline FILETIME win32_get_last_write_time(char *file_name)
{
    FILETIME last_write_time = {};
    
    WIN32_FILE_ATTRIBUTE_DATA data;
    if(GetFileAttributesEx(file_name, GetFileExInfoStandard, &data))
    {
        last_write_time = data.ftLastWriteTime;
    }
    
    return last_write_time;
}

FUNCTION Win32_Game_Code win32_load_game_code(char *dll_source_full_path,
                                              char *dll_to_load_full_path,
                                              char *lock_file_full_path)
{
    Win32_Game_Code game_code = {};
    
    // @Note: GetFileAttributesEx returns 0 on failure, i.e. when the provided
    // file doesn't exist. We want to load the .dll, when lock.tmp is deleted.
    WIN32_FILE_ATTRIBUTE_DATA unused;
    if(GetFileAttributesEx(lock_file_full_path, GetFileExInfoStandard, &unused) == 0)
    {
        game_code.dll_last_write_time = win32_get_last_write_time(dll_source_full_path);
        
        CopyFile(dll_source_full_path, dll_to_load_full_path, FALSE);
        game_code.game_code_dll = LoadLibrary(dll_to_load_full_path);
        HMODULE dll = game_code.game_code_dll;
        if(dll)
        {
            game_code.game_init = (game_init_T *) GetProcAddress(dll, "game_init");
            game_code.game_update = (game_update_T *) GetProcAddress(dll, "game_update");
            game_code.game_render = (game_render_T *) GetProcAddress(dll, "game_render");
            
            game_code.is_valid = game_code.game_render != 0;
        }
    }
    
    if(!game_code.is_valid)
    {
        game_code.game_init = 0;
        game_code.game_update = 0;
        game_code.game_render = 0;
    }
    
    return game_code;
}

FUNCTION void win32_unload_game_code(Win32_Game_Code *game_code)
{
    if(game_code->game_code_dll)
    {
        FreeLibrary(game_code->game_code_dll);
        game_code->game_code_dll = 0;
    }
    
    game_code->is_valid = false;
    game_code->game_init = 0;
    game_code->game_update = 0;
    game_code->game_render = 0;
}

inline LARGE_INTEGER win32_get_wall_clock()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

inline f32 win32_get_seconds_elapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 result = (f32)(end.QuadPart - start.QuadPart) / (f32)global_perf_count_frequency;
    
    return result;
}



//
// @Note: Miniaudio.
//
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }
    
    /* Reading PCM frames will loop based on what we specified when called ma_data_source_set_looping(). */
    ma_data_source_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);
    
    (void)pInput;
}


FUNCTION void win32_audio_init_and_start(String audio_full_path)
{
    // @Note: miniaudio simple_looping.c
    
    ma_result         result;
    ma_decoder       *decoder       = &global_miniaudio_state.decoder;
    ma_device_config *device_config = &global_miniaudio_state.device_config;
    ma_device        *device        = &global_miniaudio_state.device;
    
    result = ma_decoder_init_file((char*)audio_full_path.data, NULL, decoder);
    if (result != MA_SUCCESS) {
        PRINT("MINIAUDIO Error: couldn't initialize decoder.\n");
        return;
    }
    
    
    // A decoder is a data source which means we just use ma_data_source_set_looping() 
    // to set the looping state. We will read data using ma_data_source_read_pcm_frames() 
    // in the data callback.
    
    ma_data_source_set_looping(decoder, MA_TRUE);
    
    *device_config = ma_device_config_init(ma_device_type_playback);
    device_config->playback.format   = decoder->outputFormat;
    device_config->playback.channels = decoder->outputChannels;
    device_config->sampleRate        = decoder->outputSampleRate;
    device_config->dataCallback      = data_callback;
    device_config->pUserData         = decoder;
    
    if (ma_device_init(NULL, device_config, device) != MA_SUCCESS) {
        PRINT("MINIAUDIO Error: failed to open playback device.\n");
        ma_decoder_uninit(decoder);
        return;
    }
    
    if (ma_device_start(device) != MA_SUCCESS) {
        PRINT("MINIAUDIO Error: failed to start playback device.\n");
        ma_device_uninit(device);
        ma_decoder_uninit(decoder);
        return;
    }
}

FUNCTION void win32_audio_deinit()
{
    ma_device_uninit(&global_miniaudio_state.device);
    ma_decoder_uninit(&global_miniaudio_state.decoder);
}
//
//
//



int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance,
                   LPSTR command_line, int show_code)
{
    LARGE_INTEGER perf_count_frequency_result;
    QueryPerformanceFrequency(&perf_count_frequency_result);
    global_perf_count_frequency = perf_count_frequency_result.QuadPart;
    
    //
    // Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular.
    //
    UINT desired_scheduler_ms = 1;
    b32 sleep_is_granular = (timeBeginPeriod(desired_scheduler_ms) == TIMERR_NOERROR);
    
    //
    // Register window_class.
    //
    WNDCLASS window_class = {};
    
    window_class.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    window_class.lpfnWndProc   = win32_main_window_callback;
    window_class.hInstance     = instance;
    window_class.hCursor       = LoadCursor(0, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    //window_class.hIcon;
    window_class.lpszClassName = "MainWindowClass";
    
    if(!RegisterClassA(&window_class))
    {
        OutputDebugStringA("OS Error: WinMain() RegisterClassA() failed!\n");
        MessageBox(0, "Couldn't register window class.\n", 0, 0);
        return 0;
    }
    
    //
    // Create window.
    //
    HWND window = CreateWindowEx(0, //WS_EX_TOPMOST,
                                 window_class.lpszClassName,
                                 "Untitled Roguelike Game",
                                 WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 0,
                                 0,
                                 instance,
                                 0);
    
    if(!window)
    {
        OutputDebugStringA("OS Error: WinMain() CreateWindowEx() failed!\n");
        MessageBox(0, "Couldn't create window.\n", 0, 0);
        return 0;
    }
    
    ShowWindow(window, show_code);
    HDC window_dc = GetDC(window);
    
#if !DEBUG_BUILD
    win32_toggle_fullsceen(window);
    ShowCursor(false);
#endif
    
    //
    // Query monitor refresh rate if possible. Otherwise use 60Hz as default. 
    //
	// @Note: We have a fixed timestep that is the same for all target machines 
    // to achieve consistency of physics and therefore experience. 
	// We decouple simulation/update and rendering. We want to update by a fixed amount and render as much as 
	// we can. If our render tickrate is higher than the update tickrate we chose, it simply means we have to
	// call update() multiple times per-frame to catch up with rendering/ what we see on monitor. 
    f32 target_seconds_per_frame = 1.0f/144.0f;
    
    //
    // Game_Memory init:
    // Platform API, OpenGL context, Memory pre-allocation.
    //
#if INTERNAL_BUILD
    LPVOID base_address = (LPVOID) TERABYTES(2);
#else
    LPVOID base_address = 0;
#endif
    Game_Memory game_memory = {};
    game_memory.permanent_storage_size            = GIGABYTES(1);
    game_memory.platform_api.load_file_group      = win32_load_file_group;
    game_memory.platform_api.free_file_memory     = win32_free_file_memory;
    game_memory.platform_api.read_entire_file     = win32_read_entire_file;
    game_memory.platform_api.write_entire_file    = win32_write_entire_file;
    game_memory.platform_api.print_to_console     = win32_print_to_console;
    game_memory.platform_api.audio_init_and_start = win32_audio_init_and_start;
    platform_api = &game_memory.platform_api;
    
    game_memory.gl_extensions                  = win32_opengl_init(window_dc);
    extensions   = &game_memory.gl_extensions;
    
    Win32_State win32_state = {};
    win32_state.total_memory_size = game_memory.permanent_storage_size;
    win32_state.game_memory_block = VirtualAlloc(base_address, 
                                                 (SIZE_T) win32_state.total_memory_size, 
                                                 MEM_RESERVE|MEM_COMMIT, 
                                                 PAGE_READWRITE);
    
    game_memory.permanent_storage = win32_state.game_memory_block;
    
    if(!game_memory.permanent_storage)
    {
        OutputDebugStringA("OS Error: WinMain() VirtualAlloc() failed!\n");
        MessageBox(0, "Couldn't allocate virtual memory.\n", 0, 0);
        return 0;
    }
    
    //
    // Load game code.
    //
    win32_get_exe_full_path(&win32_state);
    
    char dll_source_full_path[MAX_STRING];
    win32_build_full_path(&win32_state, "game.dll",
                          dll_source_full_path, sizeof(dll_source_full_path));
    char dll_to_load_full_path[MAX_STRING];
    win32_build_full_path(&win32_state, "game_to_load.dll",
                          dll_to_load_full_path, sizeof(dll_to_load_full_path));
    char lock_file_full_path[MAX_STRING];
    win32_build_full_path(&win32_state, "lock.tmp",
                          lock_file_full_path, sizeof(lock_file_full_path));
    
    win32_build_full_path(&win32_state, "..\\data\\",
                          win32_state.data_directory_full_path, sizeof(win32_state.data_directory_full_path));
    game_memory.data_directory_full_path = win32_state.data_directory_full_path;
    
    Win32_Game_Code game_code = win32_load_game_code(dll_source_full_path,
                                                     dll_to_load_full_path,
                                                     lock_file_full_path);
    
    //
    // OpenGL capabilities.
    //
    extensions->glEnable(GL_BLEND);
    extensions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    extensions->glEnable(GL_DEPTH_TEST);
    extensions->glEnable(GL_MULTISAMPLE);
    
    //
    // Game input.
    //
    // @Note: We want to store the previous frame's input. We'll use a pointer to swap,
    // because we don't have to copy the entire thing at the end of each frame.
    Game_Input  inputs[2] = {};
    Game_Input *new_input = &inputs[0]; 
    Game_Input *old_input = &inputs[1]; 
    
    LARGE_INTEGER last_counter = win32_get_wall_clock();
    
    f64 accum = 0.0;
    
    //
    // Game loop
    //
    global_running = true;
    while(global_running)
    {
        // @Debug: This is no longer working after we started using Miniaudio.
        //
        // Re-loading game DLL.
        //
        FILETIME last_write_time = win32_get_last_write_time(dll_source_full_path);
        if(CompareFileTime(&last_write_time, &game_code.dll_last_write_time) != 0)
        {
            win32_unload_game_code(&game_code);
            game_code = win32_load_game_code(dll_source_full_path, 
                                             dll_to_load_full_path,
                                             lock_file_full_path);
        }
        
        
        
        //
        // Dimensions calculation.
        //
        Vector2u render_dim  = {1920, 1080};
        Vector2u window_dim  = win32_get_window_dimensions(window);
        Rect2i   drawing_dim = aspect_ratio_fit(render_dim.width, render_dim.height,
                                                window_dim.width, window_dim.height);
        game_memory.settings.window_dimensions  = window_dim;
        game_memory.settings.render_dimensions  = render_dim;
        game_memory.settings.drawing_dimensions = drawing_dim;
        
        
        
        //
        // Process input.
        //
        new_input->delta_time = target_seconds_per_frame;
        
        // mouse_buttons.
        s32 win32_mouse_buttons[MouseButton_COUNT] = {VK_LBUTTON, VK_RBUTTON, VK_MBUTTON};
        for(s32 button_index = 0; 
            button_index < MouseButton_COUNT; 
            button_index++)
        {
            new_input->mouse_buttons[button_index] = old_input->mouse_buttons[button_index];
            new_input->mouse_buttons[button_index].half_transition_count = 0;
            win32_process_button_state(&new_input->mouse_buttons[button_index], 
                                       GetKeyState(win32_mouse_buttons[button_index]) & (1 << 15));
        }
        
        // mouse_position. 
        POINT mouse;
        GetCursorPos(&mouse);
        ScreenToClient(window, &mouse);
        f32 mouse_x = (f32)mouse.x;
        f32 mouse_y = (f32)window_dim.height - 1.0f - (f32)mouse.y;
        new_input->mouse_ndc.x = clamp_binormal_range((f32)drawing_dim.min_x, mouse_x, (f32)drawing_dim.max_x);
        new_input->mouse_ndc.y = clamp_binormal_range((f32)drawing_dim.min_y, mouse_y, (f32)drawing_dim.max_y);
        new_input->mouse_ndc.z = 0.0f;
        
        // keyboard_buttons.
        for(u32 button_index = 0; button_index < Key_COUNT; button_index++)
        {
            new_input->keyboard_buttons[button_index] = old_input->keyboard_buttons[button_index];
            new_input->keyboard_buttons[button_index].half_transition_count = 0;
        }
        
#if INTERNAL_BUILD
        new_input->shift_down = GetKeyState(VK_SHIFT)   & (1 << 15);
        new_input->ctrl_down  = GetKeyState(VK_CONTROL) & (1 << 15);
        new_input->alt_down   = GetKeyState(VK_MENU)    & (1 << 15);
        
        MEMORY_ZERO_ARRAY(new_input->fk_pressed);
        new_input->del_pressed = 0;
#endif
        win32_process_pending_messages(new_input);
        
        
        //
        // Update and render.
        //
        u32 num_updates_this_frame = 0;
        if(!game_initialized)
        {
            game_code.game_init(&game_memory);
            game_initialized = true;
            last_counter = win32_get_wall_clock();
            continue;
        }
        while(accum >= target_seconds_per_frame)
        {
            game_code.game_update(&game_memory, new_input);
            accum -= target_seconds_per_frame;
            
            if(accum > 0.1)
            {
                accum = 0.0;
                last_counter = win32_get_wall_clock();
                PRINT("=============================\n"
                      "=============================\n");
                break;
            }
            
            num_updates_this_frame++;
        }
        
        glViewport(drawing_dim.min_x, drawing_dim.min_y, 
                   get_rect_width(drawing_dim), get_rect_height(drawing_dim));
        extensions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        game_code.game_render(&game_memory, new_input);
        SwapBuffers(window_dc);
        
        
        
        
        //
        // Update buttons' down_counter.
        //
        // Mouse.
        for(s32 i = 0; i < MouseButton_COUNT; i++)
        {
            if(new_input->mouse_buttons[i].ended_down)
                new_input->mouse_buttons[i].down_counter_seconds += target_seconds_per_frame;
            else
                new_input->mouse_buttons[i].down_counter_seconds  = 0.0f;
        }
        
        // Keyboard.
        for(s32 i = 0; i < Key_COUNT; i++)
        {
            if(new_input->keyboard_buttons[i].ended_down)
                new_input->keyboard_buttons[i].down_counter_seconds += target_seconds_per_frame;
            else
                new_input->keyboard_buttons[i].down_counter_seconds  = 0.0f;
        }
        
        
        
        //
        // If Vsync is off, frame rate will likely increase and game will be faster.
        // To fix it, we Sleep() when frame time is less than target_seconds_per_frame.
        //
        f32 seconds_elapsed_for_frame = win32_get_seconds_elapsed(last_counter, win32_get_wall_clock());
#if 0
        if(seconds_elapsed_for_frame < target_seconds_per_frame)
        {
            if(sleep_is_granular)
            {
                DWORD sleep_ms = (DWORD)(1000.0f * (target_seconds_per_frame -
                                                    seconds_elapsed_for_frame));
                if(sleep_ms > 0)
                {
                    Sleep(sleep_ms);
                }
            }
            
            f32 test_seconds_elapsed_for_frame = win32_get_seconds_elapsed(last_counter,
                                                                           win32_get_wall_clock());
            if(test_seconds_elapsed_for_frame < target_seconds_per_frame)
            {
                // Missed sleep!
            }
            
            while(seconds_elapsed_for_frame < target_seconds_per_frame)
            {
                seconds_elapsed_for_frame = win32_get_seconds_elapsed(last_counter,
                                                                      win32_get_wall_clock());
            }
        }
        else
        {
            // Missed frame rate!
        }
#endif
        
        
        Game_Input *tmp = old_input;
        old_input       = new_input;
        new_input       = old_input;
        
        
        
        LARGE_INTEGER end_counter      = win32_get_wall_clock();
        f32 measured_seconds_per_frame = win32_get_seconds_elapsed(last_counter, end_counter);
        
        //target_seconds_per_frame       = measured_seconds_per_frame;
        last_counter                   = end_counter;
        
        accum += (f64) measured_seconds_per_frame;
        
        //PRINT("%fMS  X: %u\n", seconds_elapsed_for_frame*1000.0f, num_updates_this_frame);
        num_updates_this_frame = 0;
        
    }
    
    
    //
    // @Note: Miniaudio.
    //
    win32_audio_deinit();
    //
    //
    //
    
    return 0;
}
