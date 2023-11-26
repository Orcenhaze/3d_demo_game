#ifndef SHARED_H
#define SHARED_H

#include "math.h"

#if DEBUG_BUILD
void print(char *format, ...)
{
    char buffer[BLOCK_SIZE];
    
    va_list arg_list;
    va_start(arg_list, format);
    umm bytes_written = string_format_list(buffer, sizeof(buffer), format, arg_list);
    va_end(arg_list);
    
    platform_api->print_to_console(buffer, bytes_written);
}

#    define GL_DEBUG 1
#    if GL_DEBUG
void GLLogProgramErrors_(char *error_tag, GLuint id, GLuint status)
{
    b32 success;
    char info_log[512];
    if(status == GL_COMPILE_STATUS)
    {
        extensions->glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        
        if(!success)
        {
            extensions->glGetShaderInfoLog(id, sizeof(info_log), NULL, info_log);
            print("[%s]: %s", error_tag, info_log);
        }
    }
    else if(status == GL_LINK_STATUS)
    {
        extensions->glGetProgramiv(id, GL_LINK_STATUS, &success);
        
        if(!success)
        {
            extensions->glGetProgramInfoLog(id, sizeof(info_log), NULL, info_log);
            print("[%s]: %s", error_tag, info_log);
        }
    }
    
}

void GLLogErrors_(char *error_tag)
{
    // Just in case.
    if(!extensions->glGetDebugMessageLogARB) return;
    
    for(;;)
    {
        GLenum  source, type, severity; 
        GLuint  id;
        GLsizei length;
        
        char buffer[BLOCK_SIZE];
        if(extensions->glGetDebugMessageLogARB(1, sizeof(buffer), &source, &type, &id, &severity, &length, buffer))
        {
            if(1)//severity != GL_DEBUG_SEVERITY_NOTIFICATION)
            {                
                print("[%s]: %s\n", error_tag, buffer);
                //ASSERT(0);
            }
        }
        else
        {
            break;
        }
    }
}
#    endif

#    define PRINT(format, ...)                  print(format, __VA_ARGS__)
#    define GLLogProgramErrors(tag, id, status) GLLogProgramErrors_(#tag, id, status)
#    define GLLogErrors(tag)                    GLLogErrors_(#tag)

#    define OS_LOG 0
#    if OS_LOG
#        define OS_PRINT(format, ...) PRINT(format, __VA_ARGS__)
#    else
#        define OS_PRINT(format, ...)
#    endif
#else
#    define PRINT(format, ...)
#    define GLLogProgramErrors(tag, id, status)
#    define GLLogErrors(tag)
#    define OS_PRINT(format, ...)
#endif

FUNCTION u64 hash_string(String string)
{
    // @Note: djb2 hash function taken from: http://www.cse.yorku.ca/~oz/hash.html
    
    const char *key_string = (const char *) string.data;
    u64 hash = 5381;
    
    for(umm i = 0; i < string.count; i++)
    {
        // hash * 33 + c;
        hash = ((hash << 5) + hash) + *key_string++;
    }
    
    return hash;
}

FUNCTION u64 hash_u32(u32 key_value)
{
    u64 hash = (u64) (17*key_value + 9);
    
    return hash;
}

#endif //SHARED_H
