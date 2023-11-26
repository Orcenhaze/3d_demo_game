
FUNCTION b32 box_overlap_test(Rect3 A, Rect3 B)
{
    swap_min_max_vectors(&A.min, &A.max);
    swap_min_max_vectors(&B.min, &B.max);
    
    b32 result = ((A.max.x > B.min.x) &&
                  (A.min.x < B.max.x) && 
                  (A.max.y > B.min.y) &&
                  (A.min.y < B.max.y) &&
                  (A.max.z > B.min.z) &&
                  (A.min.z < B.max.z));
    
    return result;
}

FUNCTION b32 point_inside_box_test(Vector3 p, Rect3 box)
{
    swap_min_max_vectors(&box.min, &box.max);
    b32 result = false;
    
    if((p.x >= box.min.x) && (p.x <= box.max.x) &&
       (p.y >= box.min.y) && (p.y <= box.max.y) &&
       (p.z >= box.min.z) && (p.z <= box.max.z))
        result = true;
    
    return result;
}

FUNCTION b32 sphere_box_intersect(Vector3 center, f32 radius, Rect3 box, Collision_Data *col_data_out)
{
    // @Note: Box has to be axis-aligned.
    //
    
    Vector3 closest_point_on_box_to_sphere = 
    {
        clamp(box.min.x, center.x, box.max.x),
        clamp(box.min.y, center.y, box.max.y),
        clamp(box.min.z, center.z, box.max.z)
    };
    Vector3 d = center - closest_point_on_box_to_sphere;
    
    // @Speed: Unnecessarily Calling length() twice (normalize() calls it).
    col_data_out->penetration = radius - length(d);
    col_data_out->normal      = normalize(d);
    
    // If penetration depth is negative, then they aren't overlapping...
    //
    b32 result = true;
    if(col_data_out->penetration < 0.0f)
    {
        col_data_out->penetration = 0.0f;
        result = false;
    }
    
    col_data_out->result = result;
    
    return result;
}

FUNCTION b32 ray_box_intersect(Ray *ray, Rect3 box)
{
    swap_min_max_vectors(&box.min, &box.max);
    Vector3 inv_rd = 1.0f / ray->d;
    
    Vector3 t_bmin = hadamard((box.min - ray->o), inv_rd);
    Vector3 t_bmax = hadamard((box.max - ray->o), inv_rd);
    
    Vector3 t_min3 = min_vector(t_bmin, t_bmax);
    Vector3 t_max3 = max_vector(t_bmin, t_bmax);
    
    f32     t_min  = MAX(t_min3.x, MAX(t_min3.y, t_min3.z));
    f32     t_max  = MIN(t_max3.x, MIN(t_max3.y, t_max3.z));
    
    // @Note: Ignore collisions that are behind us, i.e. (t_min < 0.0f).
    b32 result = ((t_min > 0.0f) && (t_min < t_max));
    
    if(result)
        ray->t = t_min;
    else
        ray->t = MAX_F32;
    
    return result;
}

FUNCTION b32 ray_plane_intersect(Ray *ray, Plane plane)
{
    f32 d      = dot(plane.normal, plane.center);
    f32 t      = (d - dot(plane.normal, ray->o)) / dot(plane.normal, ray->d);
    b32 result = (t > 0.0f) && (t < MAX_F32);
    
    if(result)
        ray->t = t;
    else
        ray->t = MAX_F32;
    
    return result;
}

FUNCTION b32 interval_overlap_test(Range A, Range B, Collision_Data *col_data_out)
{
    
    if(A.min > A.max) {SWAP(A.min, A.max, f32);}
    if(B.min > B.max) {SWAP(B.min, B.max, f32);}
    
    // @Note: All our SAT axes are positive, because negative axes will give us the same 
    // shadow interval... So we won't compute it twice. 
    // However, for obtaining the Minimum Translation Vector (MTV), we need to differentiate
    // between negative and positive axes to know the direction at which the offset applies.
    // For that, there are two cases that we need to check for.
    //
    //
    // Case 1: A<--B (Negative axis)
    //
    // min      A      max
    //  +---------------+
    //          +---------------+
    //                   B
    //
    // Case 2: B-->A (Positive axis)
    //
    // min      B      max
    //  +---------------+
    //          +---------------+
    //                   A
    
    // Case 1.
    if(A.min < B.min && A.max > B.min)
    {
        col_data_out->penetration = A.max - B.min;
        col_data_out->normal      = -col_data_out->normal;
        return true;
    }
    
    // Case 2.
    if(B.min < A.min && B.max > A.min)
    {
        col_data_out->penetration = B.max - A.min;
        col_data_out->normal      = col_data_out->normal;
        return true;
    }
    
    return false;
}

FUNCTION Range get_shadow_interval(Bounding_Volume *volume, Vector3 axis)
{
    ASSERT(volume->vertices);
    
    f32 min = dot(volume->vertices[0], axis);
    f32 max = min;
    
    for(s32 vert_index = 1; vert_index < volume->num_vertices; vert_index++)
    {
        f32 p = dot(volume->vertices[vert_index], axis);
        
        if(0);
        else if(p < min) min = p;
        else if(p > max) max = p;
    }
    
    Range  result = {min, max};
    return result;
}

FUNCTION b32 sat_test(Bounding_Volume *A, Bounding_Volume *B, Collision_Data *col_data_out)
{
    // @Note: Assuming A and B are not spheres.
    
    ASSERT(A->vertices);
    ASSERT(B->vertices);
    
    col_data_out->penetration = MAX_F32;
    col_data_out->normal      = {};
    
    // face_normal_axes of shape A.
    for(s32 i = 0; i < A->num_face_normal_axes; i++)
    {
        Collision_Data temp_col_data;
        temp_col_data.normal = A->face_normal_axes[i];
        
        Range shadow_a  = get_shadow_interval(A, temp_col_data.normal);
        Range shadow_b  = get_shadow_interval(B, temp_col_data.normal);
        
        if(!interval_overlap_test(shadow_a, shadow_b, &temp_col_data)) return false;
        else if(temp_col_data.penetration < col_data_out->penetration)
        {
            col_data_out->penetration = temp_col_data.penetration;
            col_data_out->normal      = temp_col_data.normal;
        }
    }
    
    // face_normal_axes of shape B.
    for(s32 i = 0; i < B->num_face_normal_axes; i++)
    {
        Collision_Data temp_col_data;
        temp_col_data.normal = B->face_normal_axes[i];
        
        Range shadow_a  = get_shadow_interval(A, temp_col_data.normal);
        Range shadow_b  = get_shadow_interval(B, temp_col_data.normal);
        
        if(!interval_overlap_test(shadow_a, shadow_b, &temp_col_data)) return false;
        else if(temp_col_data.penetration < col_data_out->penetration)
        {
            col_data_out->penetration = temp_col_data.penetration;
            col_data_out->normal      = temp_col_data.normal;
        }
    }
    
    // Permutations of edges between shape A and B.
    if(B->num_edge_axes != 0)
    {
        for(s32 ia = 0; ia < A->num_edge_axes; ia++)
        {
            for(s32 ib = 0; ib < B->num_edge_axes; ib++)
            {
                Collision_Data temp_col_data;
                temp_col_data.normal = cross(A->edge_axes[ia], B->edge_axes[ib]);
                temp_col_data.normal = normalize(temp_col_data.normal);
                
                Range shadow_a  = get_shadow_interval(A, temp_col_data.normal);
                Range shadow_b  = get_shadow_interval(B, temp_col_data.normal);
                
                if(!interval_overlap_test(shadow_a, shadow_b, &temp_col_data)) return false;
                else if(temp_col_data.penetration < col_data_out->penetration)
                {
                    col_data_out->penetration = temp_col_data.penetration;
                    col_data_out->normal      = temp_col_data.normal;
                }
            }
        }
    }
    
    return true;
}

FUNCTION Bounding_Volume transform_bounding_volume(Memory_Arena *temp_arena, Matrix4 transform, Bounding_Volume *volume)
{
    Bounding_Volume result = {};
    
    MEMORY_COPY(&result, volume, sizeof(Bounding_Volume));
    
    result.vertices = PUSH_ARRAY(temp_arena, result.num_vertices, Vector3);
    for(s32 i = 0; i < result.num_vertices; i++)
    {
        result.vertices[i] = transform * volume->vertices[i];
    }
    
    result.face_normal_axes = PUSH_ARRAY(temp_arena, result.num_face_normal_axes, Vector3);
    for(s32 i = 0; i < result.num_face_normal_axes; i++)
    {
        result.face_normal_axes[i] = transform_direction(transform, volume->face_normal_axes[i]);
    }
    
    result.edge_axes = PUSH_ARRAY(temp_arena, result.num_edge_axes, Vector3);
    for(s32 i = 0; i < result.num_edge_axes; i++)
    {
        result.edge_axes[i] = transform_direction(transform, volume->edge_axes[i]);
    }
    
    return result;
}

FUNCTION f32 get_radius(Vector3 center, Bounding_Volume *volume)
{
    f32 result = length(volume->vertices[0] - center);
    
    return result;
}

FUNCTION b32 sphere_overlap_test(Vector3 center_a, f32 radius_a,
                                 Vector3 center_b, f32 radius_b,
                                 Collision_Data *col_data_out)
{
    col_data_out->penetration = (radius_a + radius_b) - length(center_a - center_b);
    col_data_out->normal      = normalize(center_a - center_b);
    
    // If penetration depth is negative, then they aren't overlapping...
    if(col_data_out->penetration < 0.0f)
    {
        col_data_out->penetration = 0.0f;
        return false;
    }
    
    return true;
}

FUNCTION Vector3 closest_point_on_triangle(Vector3 p, Vector3 v0, Vector3 v1, Vector3 v2)
{
    // @Note: From Inigo Quilez.
    // Check:
    // https://www.shadertoy.com/view/ttfGWl
    
    Vector3 v10 = v1 - v0; Vector3 p0 = p - v0;
    Vector3 v21 = v2 - v1; Vector3 p1 = p - v1;
    Vector3 v02 = v0 - v2; Vector3 p2 = p - v2;
    Vector3 nor = cross(v10, v02);
    
    if(dot(cross(v10,nor), p0) < 0.0) return v0 + v10*clamp01(dot(p0,v10)/length_squared(v10));
    if(dot(cross(v21,nor), p1) < 0.0) return v1 + v21*clamp01(dot(p1,v21)/length_squared(v21));
    if(dot(cross(v02,nor), p2) < 0.0) return v2 + v02*clamp01(dot(p2,v02)/length_squared(v02));
    return p - nor*dot(nor, p0) / length_squared(nor);
}

FUNCTION f32 closest_distance_point_volume(Vector3 p, Bounding_Volume *A, Collision_Data *col_data_out)
{
    s32 num_faces = A->num_indices / 3;
    f32 result    = MAX_F32;
    
    for(s32 i = 0; i < num_faces; i++)
    {
        u32 i0 = A->indices[i*3 + 0];
        u32 i1 = A->indices[i*3 + 1];
        u32 i2 = A->indices[i*3 + 2];
        
        Vector3 v0 = A->vertices[i0];
        Vector3 v1 = A->vertices[i1];
        Vector3 v2 = A->vertices[i2];
        
        Vector3 closest_p = closest_point_on_triangle(p, v0, v1, v2);
        f32 d             = length_squared(p - closest_p);
        
        if(d < result)
        {
            result = d;
            
            col_data_out->point  = closest_p;
            col_data_out->normal = normalize(p - closest_p);
        }
    }
    
    return _square_root(result);
}

FUNCTION f32 closest_distance_ray_ray(Ray *r0, Ray *r1)
{
    // @Note: Ray directions must be normalized!
    
    // @Note: Originally looked at code from https://nelari.us/post/gizmos/,
    // but it seemingly had a mistake when returning a result, so I corrected the math using
    // Christer_Ericson_Real-Time_Collision_Detection Ch.5.1.8 Page.185.
    //
    // Basically, if the rays are parallel, then the distance between them is constant.
    // We compute it by getting the length of the vector v going from some point p0 on r0 to 
    // a point facing it p1 on r1. 
    // A faster way is to use the parallelogram which is formed by the vector v from r0's and
    // r1's origin and r0's  direction vector. But since the ray direction is normalized,
    // we can simplify further.
    // Check method #3 from: https://www.youtube.com/watch?v=9wznbg_aKOo
    //
    // If they are not parallel, we need to find 2 points (on r0 and r1), such that v is
    // perpendicular to both rays.
    
    f32 result  = MAX_F32;
    
    //r0->d = normalize(r0->d);
    //r1->d = normalize(r1->d);
    
    Vector3 vec_o = r0->o - r1->o;
    f32 dot_d     = dot(r0->d, r1->d);
    
    f32 det       = 1.0f - square(dot_d);
    
    if(_absolute(det) > MIN_F32) // Rays are non-parallel.
    {
        f32 inv_det = 1.0f / det;
        
        f32 c = dot(vec_o, r0->d);
        f32 f = dot(vec_o, r1->d);
        
        r0->t = (dot_d*f - c)       * inv_det;
        r1->t = (f       - c*dot_d) * inv_det;
        
        Vector3 p0 = r0->o + r0->d*r0->t;
        Vector3 p1 = r1->o + r1->d*r1->t;
        
        result = length(p0 - p1);
    }
    else // Rays are parallel.
    {
        result = length(cross(vec_o, r0->d));
    }
    
    return result;
}
