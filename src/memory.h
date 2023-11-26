#ifndef MEMORY_H
#define MEMORY_H

struct Memory_Arena
{
    umm  max_size;
    umm  used;
    u8  *base;
};

FUNCTION void memory_arena_init(Memory_Arena *arena, umm size, u8 *base)
{
    arena->max_size = size;
    arena->used     = 0;
    arena->base     = base;
}

#define PUSH_SIZE(arena, size)                  push_size_(arena, size)
#define PUSH_STRUCT(arena, Type)       (Type *) push_size_(arena, sizeof(Type))
#define PUSH_ARRAY(arena, count, Type) (Type *) push_size_(arena, (count) * sizeof(Type))

#define MEMORY_COPY(dest, src, size)            memory_copy_(dest, src, size)
#define MEMORY_SET(dest, val, size)             memory_set_(dest, val, size)
#define MEMORY_ZERO_ARRAY(a)                    memory_set_(a, 0, sizeof(a))
#define MEMORY_ZERO_STRUCT(s)                   memory_set_(s, 0, sizeof(*(s)))

FUNCTION void * push_size_(Memory_Arena *arena, umm size)
{
    ASSERT((arena->used + size) <= arena->max_size);
    
    void *result = arena->base + arena->used;
    arena->used  += size;
    
    return result;
}

FUNCTION void sub_arena(Memory_Arena *result, Memory_Arena *source, umm size)
{
    result->max_size = size;
    result->used     = 0;
    result->base     = (u8 *) PUSH_SIZE(source, size);
}

FUNCTION void clear_arena(Memory_Arena *arena)
{
    memory_arena_init(arena, arena->max_size, arena->base);
}

FUNCTION void * memory_copy_(void *dest, void *src, umm size)
{
    u8 *_dest = (u8 *) dest;
    u8 *_src  = (u8 *) src;
    
    while(size--)
        *_dest++ = *_src++;
    
    return dest;
}

FUNCTION void * memory_set_(void *dest, s32 value, umm size)
{
    u8 *_dest  = (u8 *) dest;
    u8 _value = *(u8 *)&value;
    
    while(size--)
        *_dest++ = _value;
    
    return dest;
}

#endif //MEMORY_H
