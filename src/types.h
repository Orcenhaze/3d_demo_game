#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int32_t   b32;
typedef int64_t   s64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef uintptr_t umm; 
typedef intptr_t  smm; 

typedef float     f32;
typedef double    f64;



#define FUNCTION           static 
#define LOCAL_PERSIST      static
#define GLOBAL             static

#define MAX_F32            3.402823466e+38F
#define MIN_F32            1.175494351e-38F
#define MIN_S32            (-2147483647 - 1)
#define MAX_U32            0xFFFFFFFF

#define PI                 3.14159265359f
#define TAU                6.28318530717f

#define KILOBYTES(value)   ((value) * 1024LL)
#define MEGABYTES(value)   (KILOBYTES(value) * 1024LL)
#define GIGABYTES(value)   (MEGABYTES(value) * 1024LL)
#define TERABYTES(value)   (GIGABYTES(value) * 1024LL)

#define JOIN(x, y)         JOIN2(x, y)
#define JOIN2(x, y)        x##y
#define DOWHILE(stmt)      do{stmt}while(0)
#define ASSERT(expr)       DOWHILE(if(!(expr)) {*(int *) 0 = 0;})
#define SWAP(a, b, Type)   DOWHILE(Type t = a; a = b; b = t;)

union Vector2
{
    struct
    {
        f32 x, y;
    };
    
    
    struct
    {
        f32 u, v;
    };
    
    
    f32 I[2];
};

union Vector3
{
    struct
    {
        f32 x, y, z;
    };
    struct
    {
        Vector2 xy;
        f32     ignored;
    };
    struct
    {
        f32     ignored;
        Vector2 yz;
    };
    
    
    struct
    {
        f32 r, g, b;
    };
    struct
    {
        Vector2 rg;
        f32     ignored;
    };
    struct
    {
        f32     ignored;
        Vector2 gb;
    };
    
    
    struct
    {
        Vector2 uv;
        f32     ignored;
    };
    
    
    f32 I[3];
};

union Vector4
{
    struct
    {
        f32 x, y, z, w;
    };
    struct
    {
        Vector3 xyz;
        f32     ignored;
    };
    struct
    {
        f32     ignored;
        Vector3 yzw;
    };
    struct
    {
        Vector2 xy;
        f32     ignored0;
        f32     ignored1;
    };
    struct
    {
        f32     ignored0;
        Vector2 yz;
        f32     ignored1;
    };
    struct
    {
        f32     ignored0;
        f32     ignored1;
        Vector2 zw;
    };
    
    
    struct
    {
        union
        {
            Vector3 rgb;
            struct
            {
                f32 r, g, b;
            };
        };
        
        f32 a;
    };
    
    
    f32 I[4];
};

union Matrix4
{
    // @Note: We use row-major with column vectors!
    
    f32 II[4][4];
    f32 I[16];
    
    struct
    {
        f32 _11, _12, _13, _14;
        f32 _21, _22, _23, _24;
        f32 _31, _32, _33, _34;
        f32 _41, _42, _43, _44;
    };
};

struct Matrix4_Inverse
{
    Matrix4 non_inverse;
    Matrix4 inverse;
};

union Quaternion
{
    struct
    {
        f32 x, y, z, w;
    };
    
    struct
    {
        Vector3 xyz;
        f32     w;
    };
    
    struct
    {
        Vector3 v;
        f32     w;
    };
};

union Vector2u
{
    struct
    {
        u32 x, y;
    };
    
    struct
    {         
        u32 width, height;
    };
};

union Rect1  // 1D
{
    struct
    {        
        f32 min;
        f32 max;
    };
    
    f32 I[2];
};
typedef Rect1 Range;

struct Rect2 // 2D
{
    Vector2 min;
    Vector2 max;
};

struct Rect2i // 2D
{
    s32 min_x, min_y;
    s32 max_x, max_y;
};

struct Rect3 // 3D
{
    Vector3 min;
    Vector3 max;
};

union Ray
{
    struct
    {
        Vector3 o;
        Vector3 d;
        f32     t;
    };
    
    struct
    {
        Vector3 origin;
        Vector3 direction;
        f32     t;
    };
};

struct Plane
{
    Vector3 center;
    Vector3 normal;
};

struct Circle
{
    Vector3 center;
    f32     radius;
    Vector3 normal;   // The circle is around this axis/normal.
};

#endif //TYPES_H
