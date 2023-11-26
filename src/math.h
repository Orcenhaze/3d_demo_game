#ifndef MATH_LIB_H
#define MATH_LIB_H

#define MIN(A, B) ((A < B) ? (A) : (B))
#define MAX(A, B) ((A > B) ? (A) : (B))

inline s32 clamp(s32 min, s32 value, s32 max)
{
    s32 result = value;
    
    if(result < min)
        result = min;
    else if(result > max)
        result = max;
    
    return result;
}

inline f32 clamp(f32 min, f32 value, f32 max)
{
    f32 result = value;
    
    if(result < min)
        result = min;
    else if(result > max)
        result = max;
    
    return result;
}

inline f32 clamp01(f32 value)
{
    f32 result = clamp(0.0f, value, 1.0f);
    
    return result;
}

inline f32 clamp01_range(f32 min, f32 value, f32 max)
{
    f32 result = 0.0f;
    
    f32 range = max - min;
    if(range != 0.0f)
        result = clamp01((value - min) / range);
    
    return result;
}

inline f32 clamp_binormal(f32 value)
{
    f32 result = 2.0f*clamp01(value) - 1.0f;
    
    return result;
}

inline f32 clamp_binormal_range(f32 min, f32 value, f32 max)
{
    f32 result = 2.0f*clamp01_range(min, value, max) - 1.0f;
    
    return result;
}

inline f32 safe_ratioN(f32 numerator, f32 divisor, f32 N)
{
    f32 result = N;
    
    if(divisor != 0.0f)
        result = numerator/divisor;
    
    return result;
}

inline f32 safe_ratio1(f32 numerator, f32 divisor)
{
    f32 result = safe_ratioN(numerator, divisor, 1.0f);
    
    return result;
}

inline f32 safe_ratio0(f32 numerator, f32 divisor)
{
    f32 result = safe_ratioN(numerator, divisor, 0.0f);
    
    return result;
}

inline Vector3 vector_safe_ratioN(f32 numerator, Vector3 divisor, f32 N)
{
    Vector3 result = {N, N, N};
    
    for(s32 i = 0; i < 3; i++)
    {
        if(divisor.I[i] != 0.0f)
            result.I[i] = numerator/divisor.I[i];
    }
    
    return result;
}

inline Vector3 vector_safe_ratio1(f32 numerator, Vector3 divisor)
{
    Vector3 result = vector_safe_ratioN(numerator, divisor, 1.0f);
    
    return result;
}

inline Vector3 vector_safe_ratio0(f32 numerator, Vector3 divisor)
{
    Vector3 result = vector_safe_ratioN(numerator, divisor, 0.0f);
    
    return result;
}

inline f32 lerp(f32 A, f32 t, f32 B)
{
    f32 result = (1.0f - t)*A + t*B;
    
    return result;
}

inline f32 square(f32 value)
{
    f32 result = value*value;
    
    return result;
}

inline Vector2 make_vector2(f32 x, f32 y)
{
    Vector2 result = {x, y};
    
    return result;
}

inline Vector2 make_vector2(f32 val)
{
    Vector2 result = {val, val};
    
    return result;
}

inline Vector2 operator+(Vector2 A, Vector2 B)
{
    Vector2 result;
    
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    
    return result;
}

inline Vector2 & operator+=(Vector2 &A, Vector2 B)
{
    A = A + B;
    
    return A;
}

inline Vector2 operator-(Vector2 A)
{
    Vector2 result;
    
    result.x = -A.x;
    result.y = -A.y;
    
    return result;
}

inline Vector2 operator-(Vector2 A, Vector2 B)
{
    Vector2 result;
    
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    
    return result;
}

inline Vector2 & operator-=(Vector2 &A, Vector2 B)
{
    A = A - B;
    
    return A;
}

inline Vector2 operator*(Vector2 A, f32 b)
{
    Vector2 result;
    
    result.x = A.x*b;
    result.y = A.y*b;
    
    return result;
}

inline Vector2 operator*(f32 b, Vector2 A)
{
    Vector2 result = A * b;
    
    return result;
}

inline Vector2 & operator*=(Vector2 &A, f32 b)
{
    A = A * b;
    
    return A;
}

inline Vector2 hadamard(Vector2 A, Vector2 B)
{
    Vector2 result;
    
    result.x = A.x*B.x;
    result.y = A.y*B.y;
    
    return result;
}

inline f32 dot(Vector2 A, Vector2 B)
{
    f32 result = A.x*B.x + A.y*B.y;
    
    return result;
}

inline void swap_min_max_vectors(Vector2 *min, Vector2 *max)
{
    if(min->x > max->x) {SWAP(min->x, max->x, f32);}
    if(min->y > max->y) {SWAP(min->y, max->y, f32);}
}














inline Vector3 make_vector3(f32 x, f32 y, f32 z)
{
    Vector3 result = {x, y, z};
    
    return result;
}

inline Vector3 make_vector3(Vector2 xy, f32 z)
{
    Vector3 result = {xy.x, xy.y, z};
    
    return result;
}

inline Vector3 make_vector3(f32 val)
{
    Vector3 result = {val, val, val};
    
    return result;
}

inline b32 vector3_is_zero(Vector3 A)
{
    b32 result = ((A.x == 0.0f) &&
                  (A.y == 0.0f) &&
                  (A.z == 0.0f));
    
    return result;
}

inline Vector3 operator+(Vector3 A, Vector3 B)
{
    Vector3 result;
    
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    
    return result;
}

inline Vector3 & operator+=(Vector3 &A, Vector3 B)
{
    A = A + B;
    
    return A;
}

inline Vector3 operator-(Vector3 A)
{
    Vector3 result;
    
    result.x = -A.x;
    result.y = -A.y;
    result.z = -A.z;
    
    return result;
}

inline Vector3 operator-(Vector3 A, Vector3 B)
{
    Vector3 result;
    
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;
    
    return result;
}

inline Vector3 & operator-=(Vector3 &A, Vector3 B)
{
    A = A - B;
    
    return A;
}

inline Vector3 operator*(Vector3 A, f32 b)
{
    Vector3 result;
    
    result.x = A.x*b;
    result.y = A.y*b;
    result.z = A.z*b;
    
    return result;
}

inline Vector3 operator*(f32 b, Vector3 A)
{
    Vector3 result = A * b;
    
    return result;
}

inline Vector3 & operator*=(Vector3 &A, f32 b)
{
    A = A * b;
    
    return A;
}

inline Vector3 operator/(Vector3 A, f32 b)
{
    Vector3 result = A * (1.0f/b);
    
    return result;
}

inline Vector3 & operator/=(Vector3 &A, f32 b)
{
    A = A / b;
    
    return A;
}

inline Vector3 operator/(f32 a, Vector3 B)
{
    Vector3 result;
    
    result.x = a / B.x;
    result.y = a / B.y;
    result.z = a / B.z;
    
    return result;
}

inline Vector3 hadamard(Vector3 A, Vector3 B)
{
    Vector3 result;
    
    result.x = A.x*B.x;
    result.y = A.y*B.y;
    result.z = A.z*B.z;
    
    return result;
}

inline f32 dot(Vector3 A, Vector3 B)
{
    f32 result = A.x*B.x + A.y*B.y + A.z*B.z;
    
    return result;
}

inline f32 length_squared(Vector3 A)
{
    f32 result = dot(A, A);
    
    return result;
}

inline f32 length(Vector3 A)
{
    f32 result = _square_root(length_squared(A));
    
    return result;
}

inline Vector3 normalize(Vector3 A)
{
    Vector3 result = {};
    
    f32 len = length(A);
    if(len != 0.0f)
        result = A * (1.0f/len);
    
    return result;
}

inline Vector3 min_vector(Vector3 A, Vector3 B)
{
    Vector3 result = 
    {
        MIN(A.x, B.x),
        MIN(A.y, B.y),
        MIN(A.z, B.z)
    };
    
    return result;
}

inline void swap_min_max_vectors(Vector3 *min, Vector3 *max)
{
    if(min->x > max->x) {SWAP(min->x, max->x, f32);}
    if(min->y > max->y) {SWAP(min->y, max->y, f32);}
    if(min->z > max->z) {SWAP(min->z, max->z, f32);}
}

inline Vector3 max_vector(Vector3 A, Vector3 B)
{
    Vector3 result = 
    {
        MAX(A.x, B.x),
        MAX(A.y, B.y),
        MAX(A.z, B.z)
    };
    
    return result;
}

inline Vector3 cross(Vector3 A, Vector3 B)
{
    Vector3 result;
    
    result.x = A.y*B.z - A.z*B.y;
    result.y = A.z*B.x - A.x*B.z;
    result.z = A.x*B.y - A.y*B.x;
    
    return result;
}

inline Vector3 lerp(Vector3 A, f32 t, Vector3 B)
{
    Vector3 result = (1.0f - t)*A + t*B;
    
    return result;
}











inline Vector4 make_vector4(f32 x, f32 y, f32 z, f32 w)
{
    Vector4 result = {x, y, z, w};
    
    return result;
}

inline Vector4 make_vector4(Vector2 xy, f32 z, f32 w)
{
    Vector4 result = {xy.x, xy.y, z, w};
    
    return result;
}

inline Vector4 make_vector4(Vector3 xyz, f32 w)
{
    Vector4 result = {xyz.x, xyz.y, xyz.z, w};
    
    return result;
}

inline Vector4 make_vector4(f32 val)
{
    Vector4 result = {val, val, val, val};
    
    return result;
}

/* 
#define VECTOR2_ZERO make_vector2(0.0f)
#define VECTOR3_ZERO make_vector3(0.0f)
#define VECTOR4_ZERO make_vector4(0.0f)
#define VECTOR3_U    make_vector3(0.0f, 1.0f,  0.0f)
#define VECTOR3_R    make_vector3(1.0f, 0.0f,  0.0f)
#define VECTOR3_F    make_vector3(0.0f, 0.0f, -1.0f)
 */

const Vector2 VECTOR2_ZERO = make_vector2(0.0f);
const Vector3 VECTOR3_ZERO = make_vector3(0.0f);
const Vector4 VECTOR4_ZERO = make_vector4(0.0f);
const Vector3 VECTOR3_U    = make_vector3(0.0f, 1.0f,  0.0f);
const Vector3 VECTOR3_R    = make_vector3(1.0f, 0.0f,  0.0f);
const Vector3 VECTOR3_F    = make_vector3(0.0f, 0.0f, -1.0f);

inline Vector4 operator+(Vector4 A, Vector4 B)
{
    Vector4 result;
    
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    result.w = A.w + B.w;
    
    return result;
}

inline Vector4 & operator+=(Vector4 &A, Vector4 B)
{
    A = A + B;
    
    return A;
}

inline Vector4 operator-(Vector4 A)
{
    Vector4 result;
    
    result.x = -A.x;
    result.y = -A.y;
    result.z = -A.z;
    result.w = -A.w;
    
    return result;
}

inline Vector4 operator-(Vector4 A, Vector4 B)
{
    Vector4 result;
    
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;
    result.w = A.w - B.w;
    
    return result;
}

inline Vector4 & operator-=(Vector4 &A, Vector4 B)
{
    A = A - B;
    
    return A;
}

inline Vector4 operator*(Vector4 A, f32 b)
{
    Vector4 result;
    
    result.x = A.x*b;
    result.y = A.y*b;
    result.z = A.z*b;
    result.w = A.w*b;
    
    return result;
}

inline Vector4 operator*(f32 b, Vector4 A)
{
    Vector4 result = A * b;
    
    return result;
}

inline Vector4 & operator*=(Vector4 &A, f32 b)
{
    A = A * b;
    
    return A;
}

inline Vector4 operator/(Vector4 A, f32 b)
{
    Vector4 result = A * (1.0f/b);
    
    return result;
}

inline Vector4 & operator/=(Vector4 &A, f32 b)
{
    A = A / b;
    
    return A;
}

inline Vector4 hadamard(Vector4 A, Vector4 B)
{
    Vector4 result;
    
    result.x = A.x*B.x;
    result.y = A.y*B.y;
    result.z = A.z*B.z;
    result.w = A.w*B.w;
    
    return result;
}

inline f32 dot(Vector4 A, Vector4 B)
{
    f32 result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
    
    return result;
}

inline f32 length_squared(Vector4 A)
{
    f32 result = dot(A, A);
    
    return result;
}

inline f32 length(Vector4 A)
{
    f32 result = _square_root(length_squared(A));
    
    return result;
}

inline Vector4 normalize(Vector4 A)
{
    Vector4 result = A * (1.0f/length(A));
    
    return result;
}

inline Vector4 lerp(Vector4 A, f32 t, Vector4 B)
{
    Vector4 result = (1.0f - t)*A + t*B;
    
    return result;
}










Matrix4 matrix4_identity()
{
    Matrix4 result = {};
    
    result._11 = 1;
    result._22 = 1;
    result._33 = 1;
    result._44 = 1;
    
    return result;
};

inline Vector4 transform(Matrix4 M, Vector4 V)
{
    Vector4 result;
    
    result.x = M._11*V.x + M._12*V.y + M._13*V.z + M._14*V.w;
    result.y = M._21*V.x + M._22*V.y + M._23*V.z + M._24*V.w;
    result.z = M._31*V.x + M._32*V.y + M._33*V.z + M._34*V.w;
    result.w = M._41*V.x + M._42*V.y + M._43*V.z + M._44*V.w;
    
    return result;
}

inline Vector3 operator*(Matrix4 M, Vector3 V)
{
    Vector3 result = transform(M, make_vector4(V, 1.0f)).xyz;
    
    return result;
}

inline Vector4 operator*(Matrix4 M, Vector4 V)
{
    Vector4 result = transform(M, V);
    
    return result;
}

inline Vector3 transform_direction(Matrix4 M, Vector3 V)
{
    Vector3 result = (M * make_vector4(V, 0.0f)).xyz;
    result         = normalize(result);
    
    return result;
}

inline Matrix4 operator*(Matrix4 A, Matrix4 B)
{
    Matrix4 result = {};
    
    for(int r = 0; r <= 3; ++r) // Rows (of A).
    {
        for(int c = 0; c <= 3; ++c) // Column (of B).
        {
            for(int i = 0; i <= 3; ++i) // Columns of A, rows of B!
            {
                result.II[r][c] += A.II[r][i]*B.II[i][c];
            }
        }
    }
    
    return result;
}

f32 to_radians(f32 degrees)
{
    f32 result = degrees * (TAU/360.0f);
    return result;
}

f32 to_degrees(f32 radians)
{
    f32 result = radians * (360.0f/TAU);
    return result;
}











inline Vector3 get_column(Matrix4 M, u32 c)
{
    Vector3 result = {M.II[0][c], M.II[1][c], M.II[2][c]};
    
    return result;
}

inline Vector3 get_row(Matrix4 M, u32 r)
{
    Vector3 result = {M.II[r][0], M.II[r][1], M.II[r][2]};
    
    return result;
}

inline Vector3 get_translation(Matrix4 M)
{
    Vector3 result = get_column(M, 3);
    
    return result;
}

inline Vector3 get_scale(Matrix4 M)
{
    f32 sx = length(get_column(M, 0));
    f32 sy = length(get_column(M, 1));
    f32 sz = length(get_column(M, 2));
    
    Vector3 result = {sx, sy, sz};
    
    return result;
}

inline Matrix4 get_rotation(Matrix4 M, Vector3 S)
{
    Vector3 c0 = get_column(M, 0) / S.x;
    Vector3 c1 = get_column(M, 1) / S.y;
    Vector3 c2 = get_column(M, 2) / S.z;
    
    Matrix4 result =
    {
        {
            {c0.x, c1.x, c2.x,  0},
            {c0.y, c1.y, c2.y,  0},
            {c0.z, c1.z, c2.z,  0},
            {   0,    0,    0,  1}
        }
    };
    
    return result;
}

FUNCTION b32 inverse(f32 m[16], f32 m_inverse[16])
{
    // @Note: Call this to compute the inverse of a matrix of unknown origin.
    // Otherwise constructing the inverse of a matrix by hand could be faster.
    
    // @Note: Read up on how to compute the inverse:
    // https://mathworld.wolfram.com/MatrixInverse.html
    
    // @Note: Taken from the MESA implementation of the GLU libray:
    // https://www.mesa3d.org/
    //
    //
    //
    // The implementation uses:
    //
    // inverse(M) = adjugate(M) / determinant(M);
    //
    // And the determinants are expanded using the Laplace expansion.
    
    f32 inv[16], det;
    int i;
    
    inv[0] = m[5]  * m[10] * m[15] - 
        m[5]  * m[11] * m[14] - 
        m[9]  * m[6]  * m[15] + 
        m[9]  * m[7]  * m[14] +
        m[13] * m[6]  * m[11] - 
        m[13] * m[7]  * m[10];
    
    inv[4] = -m[4]  * m[10] * m[15] + 
        m[4]  * m[11] * m[14] + 
        m[8]  * m[6]  * m[15] - 
        m[8]  * m[7]  * m[14] - 
        m[12] * m[6]  * m[11] + 
        m[12] * m[7]  * m[10];
    
    inv[8] = m[4]  * m[9] * m[15] - 
        m[4]  * m[11] * m[13] - 
        m[8]  * m[5] * m[15] + 
        m[8]  * m[7] * m[13] + 
        m[12] * m[5] * m[11] - 
        m[12] * m[7] * m[9];
    
    inv[12] = -m[4]  * m[9] * m[14] + 
        m[4]  * m[10] * m[13] +
        m[8]  * m[5] * m[14] - 
        m[8]  * m[6] * m[13] - 
        m[12] * m[5] * m[10] + 
        m[12] * m[6] * m[9];
    
    inv[1] = -m[1]  * m[10] * m[15] + 
        m[1]  * m[11] * m[14] + 
        m[9]  * m[2] * m[15] - 
        m[9]  * m[3] * m[14] - 
        m[13] * m[2] * m[11] + 
        m[13] * m[3] * m[10];
    
    inv[5] = m[0]  * m[10] * m[15] - 
        m[0]  * m[11] * m[14] - 
        m[8]  * m[2] * m[15] + 
        m[8]  * m[3] * m[14] + 
        m[12] * m[2] * m[11] - 
        m[12] * m[3] * m[10];
    
    inv[9] = -m[0]  * m[9] * m[15] + 
        m[0]  * m[11] * m[13] + 
        m[8]  * m[1] * m[15] - 
        m[8]  * m[3] * m[13] - 
        m[12] * m[1] * m[11] + 
        m[12] * m[3] * m[9];
    
    inv[13] = m[0]  * m[9] * m[14] - 
        m[0]  * m[10] * m[13] - 
        m[8]  * m[1] * m[14] + 
        m[8]  * m[2] * m[13] + 
        m[12] * m[1] * m[10] - 
        m[12] * m[2] * m[9];
    
    inv[2] = m[1]  * m[6] * m[15] - 
        m[1]  * m[7] * m[14] - 
        m[5]  * m[2] * m[15] + 
        m[5]  * m[3] * m[14] + 
        m[13] * m[2] * m[7] - 
        m[13] * m[3] * m[6];
    
    inv[6] = -m[0]  * m[6] * m[15] + 
        m[0]  * m[7] * m[14] + 
        m[4]  * m[2] * m[15] - 
        m[4]  * m[3] * m[14] - 
        m[12] * m[2] * m[7] + 
        m[12] * m[3] * m[6];
    
    inv[10] = m[0]  * m[5] * m[15] - 
        m[0]  * m[7] * m[13] - 
        m[4]  * m[1] * m[15] + 
        m[4]  * m[3] * m[13] + 
        m[12] * m[1] * m[7] - 
        m[12] * m[3] * m[5];
    
    inv[14] = -m[0]  * m[5] * m[14] + 
        m[0]  * m[6] * m[13] + 
        m[4]  * m[1] * m[14] - 
        m[4]  * m[2] * m[13] - 
        m[12] * m[1] * m[6] + 
        m[12] * m[2] * m[5];
    
    inv[3] = -m[1] * m[6] * m[11] + 
        m[1] * m[7] * m[10] + 
        m[5] * m[2] * m[11] - 
        m[5] * m[3] * m[10] - 
        m[9] * m[2] * m[7] + 
        m[9] * m[3] * m[6];
    
    inv[7] = m[0] * m[6] * m[11] - 
        m[0] * m[7] * m[10] - 
        m[4] * m[2] * m[11] + 
        m[4] * m[3] * m[10] + 
        m[8] * m[2] * m[7] - 
        m[8] * m[3] * m[6];
    
    inv[11] = -m[0] * m[5] * m[11] + 
        m[0] * m[7] * m[9] + 
        m[4] * m[1] * m[11] - 
        m[4] * m[3] * m[9] - 
        m[8] * m[1] * m[7] + 
        m[8] * m[3] * m[5];
    
    inv[15] = m[0] * m[5] * m[10] - 
        m[0] * m[6] * m[9] - 
        m[4] * m[1] * m[10] + 
        m[4] * m[2] * m[9] + 
        m[8] * m[1] * m[6] - 
        m[8] * m[2] * m[5];
    
    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    
    if (det == 0)
        return false;
    
    det = 1.0f / det;
    
    for (i = 0; i < 16; i++)
        m_inverse[i] = inv[i] * det;
    
    return true;
}

inline Matrix4 matrix_coefficient(Matrix4 M)
{
    Matrix4 result = matrix4_identity();
    
    Vector3 mr0 = get_row(M, 0);
    Vector3 mr1 = get_row(M, 1);
    Vector3 mr2 = get_row(M, 2);
    
    Vector3 rr0 = cross(mr1, mr2); 
    Vector3 rr1 = cross(mr2, mr0); 
    Vector3 rr2 = cross(mr0, mr1); 
    
    result._11 = rr0.x;
    result._12 = rr0.y;
    result._13 = rr0.z;
    
    result._21 = rr1.x;
    result._22 = rr1.y;
    result._23 = rr1.z;
    
    result._31 = rr2.x;
    result._32 = rr2.y;
    result._33 = rr2.z;
    
    return result;
}

inline Matrix4 transpose(Matrix4 M)
{
    Matrix4 result =
    {
        {        
            {M._11, M._21, M._31, M._41},
            {M._12, M._22, M._32, M._42},
            {M._13, M._23, M._33, M._43},
            {M._14, M._24, M._34, M._44}
        }
    };
    
    return result;
}

















inline Quaternion make_quaternion(Vector3 v, f32 w)
{
    Quaternion result;
    
    result.v = v;
    result.w = w;
    
    return result;
}

inline Quaternion make_quaternion(f32 x, f32 y, f32 z, f32 w)
{
    Quaternion result;
    
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    
    return result;
}

inline Quaternion operator*(Quaternion A, f32 s)
{
    Quaternion result;
    
    result.x = A.x*s;
    result.y = A.y*s;
    result.z = A.z*s;
    result.w = A.w*s;
    
    return result;
}

inline f32 dot(Quaternion A, Quaternion B)
{
    f32 result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
    
    return result;
}

inline f32 length_squared(Quaternion A)
{
    f32 result = dot(A, A);
    
    return result;
}

inline f32 length(Quaternion A)
{
    f32 result = _square_root(length_squared(A));
    
    return result;
}

inline Quaternion normalize(Quaternion A)
{
    Quaternion result = A * (1.0f/length(A));
    
    return result;
}

inline Quaternion make_quaternion_from_axis_and_angle(Vector3 axis, f32 angle_rad)
{
    Quaternion result;
    
    f32 w     = _cos(angle_rad*0.5f);
    f32 vcoef = _sin(angle_rad*0.5f);
    
    result.x = axis.x*vcoef;
    result.y = axis.y*vcoef;
    result.z = axis.z*vcoef;
    result.w = w;
    
    return result;
}

inline Quaternion make_quaternion_from_axis_and_angle(f32 axis_x, f32 axis_y, f32 axis_z,  f32 angle_rad)
{
    Quaternion result;
    
    f32 w     = _cos(angle_rad/2.0f);
    f32 vcoef = _sin(angle_rad/2.0f);
    
    result.x = axis_x*vcoef;
    result.y = axis_y*vcoef;
    result.z = axis_z*vcoef;
    result.w = w;
    
    return result;
}

inline Quaternion operator*(f32 s, Quaternion A)
{
    Quaternion result = A*s;
    
    return result;
}

inline Quaternion operator/(Quaternion A, f32 s)
{
    Quaternion result;
    
    f32 s_inverse = safe_ratio1(1.0f, s);
    result.x = A.x * s_inverse;
    result.y = A.y * s_inverse;
    result.z = A.z * s_inverse;
    result.w = A.w * s_inverse;
    
    return result; 
}

inline Quaternion operator+(Quaternion A, Quaternion B)
{
    Quaternion result;
    
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    result.w = A.w + B.w;
    
    return result;
}

inline Quaternion operator-(Quaternion A)
{
    Quaternion result;
    
    result.x = -A.x;
    result.y = -A.y;
    result.z = -A.z;
    result.w = -A.w;
    
    return result;
}

inline Quaternion quaternion_conjugate(Quaternion A)
{
    Quaternion result;
    
    result.v = -A.v;
    result.w =  A.w;
    
    return result;
}

inline Quaternion quaternion_inverse(Quaternion A)
{
    Quaternion result = quaternion_conjugate(A)/dot(A, A);
    
    return result;
}

inline Quaternion operator*(Quaternion A, Quaternion B)
{
    Quaternion result;
    
    result.v = A.w*B.v + B.w*A.v + cross(A.v, B.v); 
    result.w = A.w*B.w - dot(A.v, B.v);
    
    return result;
}

inline f32 quaternion_get_angle(Quaternion A)
{
    f32 result = _acos(A.w) * 2.0f;
    
    return result;
}

inline Vector3 quaternion_get_axis(Quaternion A)
{
    A = normalize(A);
    
    Vector3 result = A.v * safe_ratio1(1.0f, _sin(_acos(A.w)));
    return result;
}


inline Ray make_ray(Vector3 origin, Vector3 direction, f32 t)
{
    Ray result = {origin, direction, t};
    
    return result;
}

inline Plane make_plane(Vector3 center, Vector3 normal)
{
    Plane result = {center, normal};
    
    return result;
}

inline Circle make_circle(Vector3 center, f32 radius, Vector3 normal)
{
    Circle result = {center, radius, normal};
    
    return result;
}














inline Quaternion lerp(Quaternion q0, f32 t, Quaternion q1)
{
    Quaternion result;
    
    q0 = normalize(q0);
    q1 = normalize(q1);
    
    f32 cos_theta = dot(q0, q1);
    if(cos_theta < 0.0f)
        q1 = -q1;
    
    result.x = lerp(q0.x, t, q1.x);
    result.y = lerp(q0.y, t, q1.y);
    result.z = lerp(q0.z, t, q1.z);
    result.w = lerp(q0.w, t, q1.w);
    
    return result;
}

inline Quaternion slerp(Quaternion q0, f32 t, Quaternion q1)
{
    // Quaternion slerp according to wiki:
    // result = (q1 * q0^-1)^t * q0
    
    // Geometric slerp according to wiki, and help from glm to resolve edge cases:
    Quaternion result;
    
    f32 cos_theta = dot(q0, q1);
    if(cos_theta < 0.0f)
    {
        q1 = -q1;
        cos_theta = -cos_theta;
    }
    
    // When cos_theta is 1.0f, sin_theta is 0.0f. 
    // Avoid the slerp formula when we get close to 1.0f and do a lerp() instead.
    if(cos_theta > 1.0f - 1.192092896e-07F)
    {
        result = lerp(q0, t, q1);
    }
    else
    {
        //result = make_quaternion_from_axis_and_angle(l.v, new_angle) * q0;
        f32 theta = _acos(cos_theta);
        result = (_sin((1-t)*theta)*q0 + _sin(t*theta)*q1) / _sin(theta);
    }
    
    return result;
}

inline Vector3 operator*(Quaternion q, Vector3 v)
{
    // @Note: The rotation formula is by the optimization expert Fabian "ryg" Giesen, 
    // check his blog:
    // https://fgiesen.wordpress.com/2019/02/09/rotating-a-single-vector-using-a-quaternion/
    
    Vector3 result;
    
    // Canonical formula for rotating v by q (slower):
    // result = q * make_quaternion(v, 0) * quaternion_conjugate(q);
    
    // Fabian's method (faster):
    Vector3 t = 2 * cross(q.v, v);
    result    = v + q.w*t + cross(q.v, t);
    
    return result;
}

FUNCTION Matrix4 make_rotation_matrix(Quaternion A)
{
    // @Note: Taken from:
    //https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
    
    /*     
        1 - 2*qy2 - 2*qz2	2*qx*qy - 2*qz*qw	2*qx*qz + 2*qy*qw;
        2*qx*qy + 2*qz*qw	1 - 2*qx2 - 2*qz2	2*qy*qz - 2*qx*qw;
        2*qx*qz - 2*qy*qw	2*qy*qz + 2*qx*qw	1 - 2*qx2 - 2*qy2;
         */
    
    Matrix4 result = matrix4_identity();
    
    Vector4 tmp = normalize(*(Vector4*)&A);
    A = *(Quaternion*)&tmp;
    
    f32 Ayy = A.y * A.y;
    f32 Azz = A.z * A.z; 
    f32 Axy = A.x * A.y; 
    f32 Axz = A.x * A.z;
    f32 Ayw = A.y * A.w; 
    f32 Azw = A.z * A.w; 
    f32 Axx = A.x * A.x; 
    f32 Ayz = A.y * A.z; 
    f32 Axw = A.x * A.w; 
    
    result._11 = 1 - 2 * (Ayy + Azz);
    result._21 =     2 * (Axy + Azw);
    result._31 =     2 * (Axz - Ayw);
    
    result._12 =     2 * (Axy - Azw);
    result._22 = 1 - 2 * (Axx + Azz);
    result._32 =     2 * (Ayz + Axw);
    
    result._13 =     2 * (Axz + Ayw);
    result._23 =     2 * (Ayz - Axw);
    result._33 = 1 - 2 * (Axx + Ayy);
    
    return result;
}

FUNCTION Matrix4_Inverse look_at(Vector3 pos, Vector3 at, Vector3 up)
{
    Vector3 zaxis = normalize(at - pos);
    Vector3 xaxis = normalize(cross(zaxis, up));
    Vector3 yaxis = cross(xaxis, zaxis);
    Vector3 t     = {-dot(xaxis, pos), -dot(yaxis, pos), dot(zaxis, pos)};
    
    Vector3 xaxisI =  xaxis/length_squared(xaxis);
    Vector3 yaxisI =  yaxis/length_squared(yaxis);
    Vector3 zaxisI = -zaxis/length_squared(-zaxis);
    Vector3 tI     = 
    {
        xaxisI.x*t.x + yaxisI.x*t.y + -zaxis.x*t.z,
        xaxisI.y*t.x + yaxisI.y*t.y + -zaxis.y*t.z,
        xaxisI.z*t.x + yaxisI.z*t.y + -zaxis.z*t.z
    };
    
    Matrix4_Inverse result =
    {
        {{ // The transform itself.
                { xaxis.x,  xaxis.y,  xaxis.z, t.x},
                { yaxis.x,  yaxis.y,  yaxis.z, t.y},
                {-zaxis.x, -zaxis.y, -zaxis.z, t.z},
                {       0,        0,        0,   1}
            }},
        
        {{ // The inverse.
                { xaxisI.x,  yaxisI.x, zaxisI.x,  -tI.x},
                { xaxisI.y,  yaxisI.y, zaxisI.y,  -tI.y},
                { xaxisI.z,  yaxisI.z, zaxisI.z,  -tI.z},
                {        0,         0,        0,      1}
            }}
    };
    
    //Matrix4 identity = result.itself*result.inverse;
    
    return result;
}

FUNCTION Matrix4_Inverse perspective(f32 fov, f32 aspect, f32 near, f32 far)
{
    ASSERT(aspect != 0);
    ASSERT(near   != far);
    
    f32 t = _tan(fov/2.0f);
    f32 x = 1.0f / (aspect * t);
    f32 y = 1.0f / t;
    f32 z = -(far + near) / (far - near);
    f32 e = -(2.0f * far * near) / (far - near);
    
    Matrix4_Inverse result =
    {
        {{ // The transform itself.
                {   x, 0.0f,  0.0f, 0.0f},
                {0.0f,    y,  0.0f, 0.0f},
                {0.0f, 0.0f,     z,    e},
                {0.0f, 0.0f, -1.0f, 0.0f},
            }},
        
        {{ // Its inverse.
                {1.0f/x,   0.0f,    0.0f,  0.0f},
                {  0.0f, 1.0f/y,    0.0f,  0.0f},
                {  0.0f,   0.0f,    0.0f, -1.0f},
                {  0.0f,   0.0f,  1.0f/e,   z/e},
            }},
    };
    
    //Matrix4 identity = result.itself*result.inverse;
    
    return result;
}

FUNCTION Matrix4_Inverse orthographic(f32 l, f32 r, f32 b, f32 t)
{
    ASSERT(l != r);
    ASSERT(b != t);
    
    f32 n  = 0.01f;
    f32 f  = 1000.0f;
    
    f32 tx = -(r+l)/(r-l);
    f32 ty = -(t+b)/(t-b);
    f32 tz = -(f+n)/(f-n);
    f32 x  =  2.0f/(r-l);
    f32 y  =  2.0f/(t-b);
    f32 z  = -2.0f/(f-n);
    
    Matrix4_Inverse result = 
    {
        {{ // The transform itself.
                {   x, 0.0f, 0.0f, tx},
                {0.0f,    y, 0.0f, ty},
                {0.0f, 0.0f,    z, tz},
                {0.0f, 0.0f, 0.0f,  1}
            }},
        {{ // Its inverse.
                {1.0f/x,   0.0f,   0.0f, -tx/x},
                {  0.0f, 1.0f/y,   0.0f, -ty/y},
                {  0.0f,   0.0f, 1.0f/z, -tz/z},
                {  0.0f,   0.0f,   0.0f,   1}
            }}
    };
    
    //Matrix4 identity = result.itself*result.inverse;
    
    return result;
}

FUNCTION Vector3 unproject(Vector3 camera_position, f32 Zworld_distance_from_camera,
                           Vector3 mouse_ndc, Matrix4_Inverse view, Matrix4_Inverse proj)
{
    // @Note: Handmade Hero EP.373 and EP.374
    
    Vector3 camera_zaxis = get_row(view.non_inverse, 2);
    Vector3 new_p        = camera_position - Zworld_distance_from_camera*camera_zaxis;
    Vector4 probe_z      = make_vector4(new_p, 1.0f);
    
    // Get probe_z in clip space.
    probe_z = proj.non_inverse*view.non_inverse*probe_z;
    
    // Undo the perspective divide.
    mouse_ndc.x *= probe_z.w;
    mouse_ndc.y *= probe_z.w;
    
    Vector4 mouse_clip = make_vector4(mouse_ndc.x, mouse_ndc.y, probe_z.z, probe_z.w);
    Vector3 result     = (view.inverse*proj.inverse*mouse_clip).xyz;
    
    return result;
}

inline s32 get_rect_width(Rect2i A)
{
    s32 result = A.max_x - A.min_x;
    
    return result;
}

inline s32 get_rect_height(Rect2i A)
{
    s32 result = A.max_y - A.min_y;
    
    return result;
}

FUNCTION Rect2i aspect_ratio_fit(u32 render_width, u32 render_height,
                                 u32 window_width, u32 window_height)
{
    // @Note: From Handmade Hero.
    Rect2i result = {};
    
    if((render_width > 0) && (render_height > 0) &&
       (window_width > 0) && (window_height > 0))
    {
        f32 optimal_window_width = (f32)window_height * ((f32)render_width/(f32)render_height);
        f32 optimal_window_height = (f32)window_width * ((f32)render_height/(f32)render_width);
        
        if(optimal_window_width > (f32)window_width)
        {
            // Width-constrained display (top and bottom black bars).
            result.min_x = 0;
            result.max_x = window_width;
            
            f32 empty_space = (f32)window_height - optimal_window_height;
            s32 half_empty  = _round_f32_to_s32(0.5f*empty_space);
            s32 used_height = _round_f32_to_s32(optimal_window_height);
            
            result.min_y = half_empty; 
            result.max_y = result.min_y + used_height;
        }
        else
        {
            // Height-constrained display (left and right black bars).
            result.min_y = 0;
            result.max_y = window_height;
            
            f32 empty_space = (f32)window_width - optimal_window_width;
            s32 half_empty  = _round_f32_to_s32(0.5f*empty_space);
            s32 used_height = _round_f32_to_s32(optimal_window_width);
            
            result.min_x = half_empty; 
            result.max_x = result.min_x + used_height;
        }
    }
    
    return result;
}

FUNCTION void calculate_tangents(Vector3 normal, Vector3 *tangent_out, Vector3 *bitangent_out)
{
    Vector3 a = cross(normal, VECTOR3_R);
    Vector3 b = cross(normal, VECTOR3_U);
    
    if(length_squared(a) > length_squared(b)) *tangent_out = normalize(a);
    else                                      *tangent_out = normalize(b);
    
    *bitangent_out = normalize(cross(*tangent_out, normal));
}

inline Range make_range(f32 min, f32 max)
{
    Range result;
    
    result.min = min;
    result.max = max;
    
    return result;
}

inline Rect3 make_rect3(Vector3 min, Vector3 max)
{
    Rect3 result;
    
    result.min = min;
    result.max = max;
    
    return result;
}

FUNCTION Rect3 get_plane_bounds(Plane plane, f32 scale)
{
    Vector3 c = plane.center; Vector3 normal = plane.normal;
    
    // Calculate quad tangent and bitangent.
    Vector3 tangent, bitangent;
    calculate_tangents(normal, &tangent, &bitangent);
    
    scale /= 2.0f;
    
    Rect3 result;
    result.min = c - tangent*scale - bitangent*scale;
    result.max = c + tangent*scale + bitangent*scale;
    
    swap_min_max_vectors(&result.min, &result.max);
    
    return result;
}

inline Vector3 rect3_get_scale(Rect3 A)
{
    Vector3 result = A.max - A.min;
    return result;
}

inline Vector3 rect3_get_center(Rect3 A)
{
    Vector3 result = (A.min + A.max) * 0.5f;
    return result;
}

inline b32 rect3_is_zero(Rect3 A)
{
    b32 result = ((length_squared(A.min) == 0.0f) &&
                  (length_squared(A.max) == 0.0f));
    return result;
}

inline s32 range_rand(Range bounds)
{
    s32 result = (_rand() % ((s32)bounds.max - (s32)bounds.min + 1)) + (s32)bounds.min;
    return result;
}

inline Vector3 vector3_rand(Rect3 bounds = {VECTOR3_ZERO, VECTOR3_ZERO})
{
    Vector3 result = {};
    
    if(!rect3_is_zero(bounds))
    {
        result.x = (f32) range_rand(make_range(bounds.min.x, bounds.max.x));
        result.y = (f32) range_rand(make_range(bounds.min.y, bounds.max.y));
        result.z = (f32) range_rand(make_range(bounds.min.z, bounds.max.z));
    }
    
    return result;
}

#endif //MATH_LIB_H
