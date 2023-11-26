#ifndef STRING_H
#define STRING_H

// @Note: Lots of help from Allen Webster's Splink.

#include <stdarg.h>

#define ARRAY_COUNT(arr) ((sizeof(arr) / sizeof((arr)[0])))
#define MAX_STRING 256
#define BLOCK_SIZE 4096

#define STRING_LITERAL(lit) make_string((u8 *) lit, ARRAY_COUNT(lit) - 1)
#define STRING_ZERO         make_string(0, 0)
#define STRING_TEMPF(name, format, ...)       \
char JOIN(temp, __LINE__)[MAX_STRING];    \
String name;                              \
name.data  = (u8 *) JOIN(temp, __LINE__); \
name.count = string_format(JOIN(temp, __LINE__), sizeof(JOIN(temp, __LINE__)), format, __VA_ARGS__)


// @Note: ASCII string. Allocation size is count+1 to account for null terminator, only to 
// deal with APIs easily.
struct String
{
    union
    {
        u8   *data;
        char *at;
    };
    
    union
    {
        umm count;
        umm size;
    };
};
typedef String Buffer;

FUNCTION u8 * advance_bytes(Buffer *buffer, umm count)
{
    u8 *result = 0;
    
    if(buffer->count >= count)
    {
        result         = buffer->data;
        buffer->data  += count;
        buffer->count -= count;
    }
    else
    {
        buffer->data  += buffer->count;
        buffer->count  = 0;
    }
    
    return result;
}

FUNCTION umm string_length(char *cstring)
{
    umm count = 0;
    
    while(*cstring++)
    {
        count++;
    }
    
    return count;
}

FUNCTION String make_string(u8 *data, umm count)
{
    String result = {data, count};
    return result;
}

FUNCTION String make_string_from_cstring(char *cstring)
{
    String result = {};
    
    result.data  = (u8 *) cstring;
    result.count = string_length(cstring);
    
    return result;
}

FUNCTION String push_string_copy(Memory_Arena *arena, String string)
{
    String result;
    
    result.count = string.count;
    result.data  = PUSH_ARRAY(arena, result.count + 1, u8);
    MEMORY_COPY(result.data, string.data, result.count);
    result.data[result.count] = 0;
    
    return result;
}

FUNCTION String push_string_cat(Memory_Arena *arena, String a, String b)
{
    String result;
    
    result.count = a.count + b.count;
    result.data  = PUSH_ARRAY(arena, result.count + 1, u8);
    MEMORY_COPY(result.data, a.data, a.count);
    MEMORY_COPY(result.data + a.count, b.data, b.count);
    result.data[result.count] = 0;
    
    return result;
}

FUNCTION b32 string_empty(String string)
{
    b32 result = !string.data || string.count == 0;
    return result;
}

FUNCTION b32 string_match(String a, String b)
{
    if(a.count != b.count) return false;
    
    b32 result = true;
    
    for(umm i = 0; i < a.count; i++)
    {
        if(a.data[i] != b.data[i])
        {
            result = false;
            break;
        }
    }
    
    return result;
}







FUNCTION b32 is_spacing(char c)
{
    b32 result = ((c == ' ')  ||
                  (c == '\t') ||
                  (c == '\v') ||
                  (c == '\f'));
    
    return result;
}

FUNCTION b32 is_end_of_line(char c)
{
    b32 result = ((c == '\n') ||
                  (c == '\r'));
    
    return result;
}

FUNCTION b32 is_whitespace(char c)
{
    b32 result = (is_spacing(c) || is_end_of_line(c));
    return result;
}

FUNCTION b32 is_alpha(char c)
{
    b32 result = (((c >= 'A') && (c <= 'Z')) ||
                  ((c >= 'a') && (c <= 'z')));
    
    return result;
}

FUNCTION b32 is_numeric(char c)
{
    b32 result = (((c >= '0') && (c <= '9')));
    return result;
}

FUNCTION b32 is_alphanumeric(char c)
{
    b32 result = (is_alpha(c) || is_numeric(c));
    return result;
}

FUNCTION b32 is_file_separator(char c)
{
    b32 result = ((c == '\\') || (c == '/'));
    return result;
}







FUNCTION String get_parent_directory(String src)
{
    // @Note: Make sure that the src is around when using the returned string. Because
    // we are only copying the data pointer and not actually allocating anything.
    
    String result = {};
    
    if(!src.count) return result;
    
    u8 *base_name = 0;
    for(umm i = 0; i < src.count; i++)
    {
        if(is_file_separator(src.data[i]))
        {
            base_name = (src.data + i) + 1;
        }
    }
    
    result.data  = src.data;
    result.count = (umm)(base_name - src.data);
    //result.data[result.count] = 0;
    
    return result;
}

FUNCTION String get_base_name(String src)
{
    // @Note: Make sure that the src is around when using the returned string. Because
    // we are only copying the data pointer and not actually allocating anything.
    
    String result = src;
    
    if(!src.count) return result;
    
    u8 *base_name = 0;
    for(umm i = 0; i < src.count; i++)
    {
        if(is_file_separator(src.data[i]) && (i != (src.count - 1)))
        {
            base_name = (src.data + i) + 1;
        }
    }
    
    result.data  = base_name;
    result.count = (umm)((src.data + src.count) - base_name);
    //result.data[result.count] = 0;
    
    return result;
}

FUNCTION void put_char(Buffer *dest, char c)
{
    if(dest->size)
    {
        dest->size--;
        *dest->at++ = c;
    }
}

FUNCTION void put_cstring(Buffer *dest, char *cstring)
{
    while(*cstring)
    {
        put_char(dest, *cstring++);
    }
}

char decimal_digits[] = "0123456789";
FUNCTION void u64_to_ascii(Buffer *dest, u64 value, u32 base, char *digits)
{
    // @Note: From Handmade Hero.
    
    ASSERT(base != 0);
    
    char *start = dest->at;
    do
    {
        char digit = digits[value % base];
        put_char(dest, digit);
        
        value /= base;
        
    } while(value != 0);
    char *end  = dest->at;
    
    // Reverse.
    while(start < end)
    {
        end--;
        
        char temp = *end;
        *end      = *start;
        *start    = temp;
        
        start++;
    }
}

FUNCTION void f64_to_ascii(Buffer *dest, f64 value, u32 precision)
{
    // @Note: From Handmade Hero.
    
    if(value < 0.0f)
    {
        put_char(dest, '-');
        value = -value;
    }
    
    u64 integer_part = (u64) value;
    value           -= (f64)integer_part;
    
    u64_to_ascii(dest, integer_part, 10, decimal_digits);
    put_char(dest, '.');
    
    for(u32 precision_index = 0; precision_index < precision; precision_index++)
    {
        value      *= 10.0f;
        u32 integer = (u32) value;
        value      -= (f32)integer;
        
        put_char(dest, decimal_digits[integer]);
    }
}

FUNCTION umm string_format_list(char *dest_start, umm dest_count, char *format, va_list arg_list)
{
    Buffer dest = {(u8*)dest_start, dest_count};
    
    if(!dest_count) return 0;
    
    char *at = format;
    while(at[0])
    {
        if(at[0] != '%')
        {
            put_char(&dest, *at++);
        }
        else
        {
            at++;
            u32 precision = 6;
            
            switch(at[0])
            {
                case 'c':
                {
                    char c = (char) va_arg(arg_list, int);
                    put_char(&dest, c);
                } break;
                
                case 's':
                {
                    char *cstr = va_arg(arg_list, char *);
                    put_cstring(&dest, cstr);
                } break;
                
                case 'S':
                {
                    String str = va_arg(arg_list, String);
                    
                    for(umm i = 0; i < str.count; i++)
                        put_char(&dest, str.data[i]);
                    
                } break;
                
                case 'i':
                case 'd':
                {
                    s64 value = (s64) va_arg(arg_list, s32);
                    
                    if(value < 0) 
                        put_char(&dest, '-');
                    
                    u64_to_ascii(&dest, (u64)value, 10, decimal_digits);
                    
                } break;
                
                case 'u':
                {
                    u64 value = (u64) va_arg(arg_list, u32);
                    u64_to_ascii(&dest, value, 10, decimal_digits);
                } break;
                
                case 'f':
                {
                    f64 value = va_arg(arg_list, f64);
                    f64_to_ascii(&dest, value, precision);
                } break;
                
                case 'm':
                {
                    umm value = va_arg(arg_list, umm);
                    u64_to_ascii(&dest, (u64)value, 10, decimal_digits);
                } break;
                
                case 'v':
                {
                    if((at[1] != '2') && 
                       (at[1] != '3') && 
                       (at[1] != '4'))
                        ASSERT(!"Unrecognized format specifier!");
                    
                    u32 vector_size = (u32)(at[1] - '0');
                    Vector4 *v      = 0;
                    
                    if(0);
                    else if(vector_size == 2)
                        v = (Vector4 *) &va_arg(arg_list, Vector2);
                    else if(vector_size == 3)
                        v = (Vector4 *) &va_arg(arg_list, Vector3);
                    else if(vector_size == 4)
                        v = (Vector4 *) &va_arg(arg_list, Vector4);
                    
                    put_char(&dest, '[');
                    for(u32 i = 0; i < vector_size; i++)
                    {
                        f64_to_ascii(&dest, v->I[i], precision);
                        
                        if(i < vector_size - 1)
                        {
                            put_cstring(&dest, ", ");
                        }
                    }
                    put_char(&dest, ']');
                    
                    // Advance once.
                    at++;
                } break;
                
                case '%':
                {
                    put_char(&dest, '%');
                } break;
                
                default:
                {
                    ASSERT(!"Unrecognized format specifier!");
                } break;
            }
            
            if(*at)
                at++;
        }
        
        if(dest.size)
            dest.at[0]  = 0;
        else
            dest.at[-1] = 0;
    }
    
    // @Debug: The count will be off by 1, iff we null terminate using `at[-1] = 0`
    umm result = dest.at - dest_start;
    return result;
}

FUNCTION umm string_format(char *dest, umm dest_count, char *format, ...)
{
    va_list arg_list;
    
    va_start(arg_list, format);
    umm result = string_format_list(dest, dest_count, format, arg_list);
    va_end(arg_list);
    
    return result;
}

FUNCTION String push_stringf(Memory_Arena *arena, char *format, ...)
{
    char   temp[MAX_STRING];
    String str = {};
    
    va_list arg_list;
    
    va_start(arg_list, format);
    str.data  = (u8 *) temp;
    str.count = string_format_list(temp, sizeof(temp), format, arg_list);
    va_end(arg_list);
    
    String result = push_string_copy(arena, str);
    
    return result;
}







struct String_Builder
{
    Memory_Arena *temp_arena;
    Buffer buffer;
    
    u8 *start;
    umm length;
    umm capacity;
};

FUNCTION String_Builder make_string_builder(Memory_Arena *temp_arena, umm capacity = BLOCK_SIZE)
{
    String_Builder result = {};
    
    result.temp_arena  = temp_arena;
    result.capacity    = capacity;
    result.buffer.size = capacity;
    result.buffer.data = PUSH_ARRAY(temp_arena, capacity, u8);
    result.start       = result.buffer.data;
    
    return result;
}

FUNCTION void append(String_Builder *builder, char *cstring)
{
    umm len = string_length(cstring);
    
    if((builder->length + len) > builder->capacity)
    {
        builder->capacity += BLOCK_SIZE;
        
        String_Builder new_builder = make_string_builder(builder->temp_arena, builder->capacity);
        
        if(builder->length)
        {
            MEMORY_COPY(new_builder.buffer.data, builder->start, builder->length);
        }
        
        builder->buffer = new_builder.buffer;
        builder->start  = new_builder.start;
        
        advance_bytes(&builder->buffer, builder->length);
    }
    
    put_cstring(&builder->buffer, cstring);
    builder->length = builder->buffer.at - (char*)builder->start;
}

FUNCTION void appendf(String_Builder *builder, char *format, ...)
{
    char str[BLOCK_SIZE];
    
    va_list arg_list;
    va_start(arg_list, format);
    umm bytes_written = string_format_list(str, sizeof(str), format, arg_list);
    va_end(arg_list);
    
    append(builder, str);
}

FUNCTION String to_string(String_Builder *builder)
{
    if(builder->buffer.size)
    {
        builder->buffer.at[0] = 0;
    }
    else
    {
        builder->buffer.at[-1] = 0;
        builder->length--;
    }
    
    String result = make_string(builder->start, builder->length);
    return result;
}

#endif //STRING_H
