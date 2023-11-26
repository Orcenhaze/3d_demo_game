#ifndef INTRINSICS_H
#define INTRINSICS_H

// @Todo: Remove <cmath> eventually.
#include <cmath>
#include <intrin.h>

inline s32 _sign_of(s32 val)
{
    s32 result = (val < 0)? -1 : 1;
    
    return result;
}

inline f32 _sign_of(f32 val)
{
    f32 result = (val < 0.0f)? -1.0f : 1.0f;
    
    return result;
}

inline Vector3 _sign_of(Vector3 v)
{
    Vector3 result = {_sign_of(v.x), _sign_of(v.y), _sign_of(v.z)};
    
    return result;
}

inline f32 _absolute(f32 x)
{
    f32 result = fabsf(x);
    
    return result;
}

inline Vector3 _absolute(Vector3 v)
{
    Vector3 result = {_absolute(v.x), _absolute(v.y), _absolute(v.z)};
    
    return result;
}

inline f32 _force_sign(f32 val, f32 sign)
{
    f32 result = _absolute(val) * sign;
    return result;
}

inline f32 _fmodf(f32 numer, f32 denom)
{
    f32 result = fmodf(numer, denom);
    
    return result;
}

inline f32 _square_root(f32 x)
{
    f32 result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
    
    return result;
}

inline f32 _tan(f32 x)
{
    f32 result = tanf(x);
    
    return result;
}

inline f32 _sin(f32 x)
{
    f32 result = sinf(x);
    
    return result;
}

inline f32 _cos(f32 x)
{
    f32 result = cosf(x);
    
    return result;
}

inline f32 _acos(f32 x)
{
    f32 result = acosf(x);
    
    return result;
}

inline f32 _atan2(f32 y, f32 x)
{
    f32 result = atan2f(y, x);
    
    return result;
}

inline s32 _rand()
{
    s32 result = rand();
    return result;
}

inline s32 _round_f32_to_s32(f32 x)
{
    s32 result = _mm_cvtss_si32(_mm_set_ss(x));
    
    return result;
}

#endif //INTRINSICS_H
