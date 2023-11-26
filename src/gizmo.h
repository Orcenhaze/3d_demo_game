#ifndef GIZMO_H
#define GIZMO_H

//
// @Note: This file is NOT independant. It's using the engine's types, collision routines
// and immediate-mode renderer.
//

enum Gizmo_Mode
{
    GizmoMode_TRANSLATION,
    GizmoMode_ROTATION
};

enum Gizmo_Element
{
    GizmoElement_NONE,
    GizmoElement_XAXIS,
    GizmoElement_YAXIS,
    GizmoElement_ZAXIS,
    GizmoElement_XPLANE,
    GizmoElement_YPLANE,
    GizmoElement_ZPLANE,
    GizmoElement_CPLANE, // camera_ray plane.
};

struct Gizmo_Geometry
{
    Ray    x_axis,   y_axis,   z_axis;
    Plane  x_plane,  y_plane,  z_plane;
    Circle x_circle, y_circle, z_circle, c_circle;
};

b32            gizmo_is_active; // True iff close to an element and MouseButton_Left is down.
Gizmo_Mode     gizmo_mode;
Gizmo_Element  gizmo_element;
Gizmo_Geometry gizmo_geom;

Vector3        gizmo_new_point;
Vector3        gizmo_old_point;
Vector3        gizmo_start_point; // For visualizing rotations.

Vector4 gizmo_active_color = make_vector4(1.0f);
Vector4 gizmo_plane_color  = make_vector4(1.0f, 1.0f, 0.0f, 0.5f);
Vector4 gizmo_xaxis_color  = make_vector4(1.0f, 0.0f, 0.0f, 0.3f);
Vector4 gizmo_yaxis_color  = make_vector4(0.0f, 1.0f, 0.0f, 0.3f);
Vector4 gizmo_zaxis_color  = make_vector4(0.0f, 0.0f, 1.0f, 0.3f);

FUNCTION void gizmo_render_translation(f32 axis_scale, f32 thickness, f32 plane_scale,
                                       Vector4 *colors)
{
    s32 color_index     = gizmo_element;
    colors[color_index] = gizmo_active_color;
    
    Ray x_axis          = gizmo_geom.x_axis; 
    Ray y_axis          = gizmo_geom.y_axis; 
    Ray z_axis          = gizmo_geom.z_axis; 
    Plane x_plane       = gizmo_geom.x_plane;
    Plane y_plane       = gizmo_geom.y_plane;
    Plane z_plane       = gizmo_geom.z_plane;
    
    // Ignore z-buffer.
    extensions->glClear(GL_DEPTH_BUFFER_BIT);
    immediate_begin();
    
    immediate_line(x_axis.o, x_axis.o + x_axis.d*axis_scale, colors[GizmoElement_XAXIS], thickness);
    immediate_line(y_axis.o, y_axis.o + y_axis.d*axis_scale, colors[GizmoElement_YAXIS], thickness);
    immediate_line(z_axis.o, z_axis.o + z_axis.d*axis_scale, colors[GizmoElement_ZAXIS], thickness);
    
    immediate_quad(x_plane, plane_scale, colors[GizmoElement_XPLANE]);
    immediate_quad(y_plane, plane_scale, colors[GizmoElement_YPLANE]);
    immediate_quad(z_plane, plane_scale, colors[GizmoElement_ZPLANE]);
    
    immediate_flush();
    
}

FUNCTION void gizmo_render_rotation(f32 thickness, Vector4 *colors)
{
    // Use axis colors for circle planes.
    s32 color_index     = gizmo_element - GizmoElement_XPLANE + 1;
    colors[color_index] = gizmo_active_color;
    
    Circle x_circle     = gizmo_geom.x_circle;
    Circle y_circle     = gizmo_geom.y_circle;
    Circle z_circle     = gizmo_geom.z_circle;
    Circle c_circle     = gizmo_geom.c_circle;
    
    // Assuming they all have the same center.
    Vector3 c = c_circle.center;
    
    // Ignore z-buffer.
    extensions->glClear(GL_DEPTH_BUFFER_BIT);
    immediate_begin();
    
    if(gizmo_is_active)
    {
        immediate_line(c,   gizmo_new_point, gizmo_active_color, thickness*0.50f);
        immediate_line(c, gizmo_start_point, gizmo_active_color, thickness*0.20f);
    }
    
    immediate_torus(c, x_circle.radius, x_circle.normal, colors[GizmoElement_XAXIS], thickness);
    immediate_torus(c, y_circle.radius, y_circle.normal, colors[GizmoElement_YAXIS], thickness);
    immediate_torus(c, z_circle.radius, z_circle.normal, colors[GizmoElement_ZAXIS], thickness);
    immediate_torus(c, c_circle.radius, c_circle.normal, colors[GizmoElement_XPLANE], thickness);
    
    immediate_flush();
}

FUNCTION void gizmo_execute(Game_Input *input, Ray camera_ray, Selected_Entity *selected_entity)
{
    Entity *entity      = selected_entity->entity;
    Vector3 pos         = entity->position;
    
    f32 dist__          = MAX_F32;
    f32 t__             = MAX_F32;
    b32 is_close        = false;
    
    Vector3 pos_cam     = camera_ray.o - pos;
    f32 cam_dist        = length(pos_cam);
    pos_cam             = normalize(pos_cam);
    
    f32 scale           = cam_dist / 5.00f; // @Hardcode: scale.
    f32 threshold       = scale    * 0.07f; // @Hardcode: threshold ratio.
    f32 thickness       = scale    * 0.05f; // @Hardcode: thickness ratio.
    f32 plane_scale     = scale    * 0.50f; // @Hardcode: plane_scale ratio.
    
    gizmo_geom.x_axis   = make_ray(pos, {1.0f, 0.0f, 0.0f}, MAX_F32);
    gizmo_geom.y_axis   = make_ray(pos, {0.0f, 1.0f, 0.0f}, MAX_F32);
    gizmo_geom.z_axis   = make_ray(pos, {0.0f, 0.0f, 1.0f}, MAX_F32);
    
    Vector3 sx = make_vector3(0.0f, plane_scale, plane_scale);
    Vector3 sy = make_vector3(plane_scale, 0.0f, plane_scale);
    Vector3 sz = make_vector3(plane_scale, plane_scale, 0.0f);
    
    gizmo_geom.x_plane  = make_plane(pos + sx, {1.0f, 0.0f, 0.0f});
    gizmo_geom.y_plane  = make_plane(pos + sy, {0.0f, 1.0f, 0.0f});
    gizmo_geom.z_plane  = make_plane(pos + sz, {0.0f, 0.0f, 1.0f});
    
    gizmo_geom.x_circle = make_circle(pos, scale+(scale*0.10f), {1.0f, 0.0f, 0.0f});
    gizmo_geom.y_circle = make_circle(pos, scale+(scale*0.05f), {0.0f, 1.0f, 0.0f});
    gizmo_geom.z_circle = make_circle(pos, scale+(scale*0.00f), {0.0f, 0.0f, 1.0f});
    gizmo_geom.c_circle = make_circle(pos, scale+(scale*0.30f),            pos_cam);
    
    
    // @Note: Storing everything in Gizmo_Geometry will be useful when rendering the gizmos.
    Ray *axes[3] = 
    {
        &gizmo_geom.x_axis,
        &gizmo_geom.y_axis,
        &gizmo_geom.z_axis
    };
    Plane *planes[3] =
    {
        &gizmo_geom.x_plane,
        &gizmo_geom.y_plane,
        &gizmo_geom.z_plane
    };
    Circle *circles[4] = 
    {
        &gizmo_geom.x_circle,
        &gizmo_geom.y_circle,
        &gizmo_geom.z_circle,
        &gizmo_geom.c_circle
    };
    Vector4 colors[7]
    {
        make_vector4(0.0f),
        gizmo_xaxis_color,
        gizmo_yaxis_color,
        gizmo_zaxis_color,
        gizmo_plane_color,
        gizmo_plane_color,
        gizmo_plane_color,
    };
    
    //
    // Choose a gizmo element based on distance or intersection.
    //
    if(!gizmo_is_active)
    {
        if(gizmo_mode == GizmoMode_TRANSLATION)
        {
            // Axes.
            for(s32 i = 0; i < 3; i++)
            {
                // @Debug: Using closest_distance_ray_ray() causes unwanted behaviour when
                // moving the cursor away from the gizmo element. 
                
                f32 dist    = closest_distance_ray_ray(&camera_ray, axes[i]);
                b32 t_valid = (axes[i]->t > 0) && (axes[i]->t < scale);
                
                if((dist < dist__) && (dist < threshold) && (t_valid))
                {
                    dist__ = dist;
                    
                    is_close      = true;
                    gizmo_element = (Gizmo_Element)(GizmoElement_XAXIS + i);
                }
            }
            
            // Planes.
            for(s32 i = 0; i < 3; i++)
            {
                if(ray_plane_intersect(&camera_ray, *planes[i]))
                {
                    Vector3 on_plane   = camera_ray.o + camera_ray.d*camera_ray.t;
                    Rect3 plane_bounds = get_plane_bounds(*planes[i], plane_scale);
                    
                    // Only compare the dimensions of the plane. So guarantee equality for 
                    // the other dimension to have a proper point_inside_box_test().
                    on_plane.I[i]      = plane_bounds.min.I[i];
                    
                    if(point_inside_box_test(on_plane, plane_bounds) && (camera_ray.t < t__))
                    {
                        t__ = camera_ray.t;
                        
                        is_close      = true;
                        gizmo_element = (Gizmo_Element)(GizmoElement_XPLANE + i);
                    }
                }
            }
            
        } // GizmoMode_Translation
        
        else if(gizmo_mode == GizmoMode_ROTATION)
        {
            for(s32 i = 0; i < 4; i++)
            {
                Vector3 c = circles[i]->center;
                f32     r = circles[i]->radius;
                Vector3 n = circles[i]->normal;
                
                Plane circle_plane = make_plane(c, n);
                
                // @Todo: If we don't intersect the circle_plane, we should find another
                // way to choose a circle. 
                if(ray_plane_intersect(&camera_ray, circle_plane))
                {
                    Vector3 on_plane  = camera_ray.o + camera_ray.d*camera_ray.t;
                    Vector3 on_circle = c + r*normalize(on_plane - c);
                    f32 dist          = length(on_plane - on_circle);
                    
                    if((dist < dist__) && (dist < threshold) && (camera_ray.t < t__))
                    {
                        dist__ = dist;
                        t__    = camera_ray.t;
                        
                        is_close      = true;
                        gizmo_element = (Gizmo_Element)(GizmoElement_XPLANE + i);
                    }
                }
            }
            
        } // GizmoMode_Rotation
        
    } // !gizmo_is_active
    
    
    
    //
    // Decide if gizmo_is_active or not.
    //
    if(is_close && is_down(input->mouse_buttons[MouseButton_LEFT]))
    {
        gizmo_is_active = true;
        push(selected_entity);
    }
    if(!is_down(input->mouse_buttons[MouseButton_LEFT]))
    {
        if(gizmo_is_active)
        {
            push(selected_entity);
        }
        
        gizmo_is_active = false;
        gizmo_element   = is_close? gizmo_element : GizmoElement_NONE;
        
        gizmo_old_point = {};
        gizmo_new_point = {};
    }
    
    
    
    //
    // Process gizmo manipulation.
    //
    if(gizmo_is_active)
    {
        if(gizmo_mode == GizmoMode_TRANSLATION)
        {
            Vector3 delta = {};
            
            // Axes.
            if(gizmo_element < GizmoElement_XPLANE)
            {
                s32 axis_index  = gizmo_element - GizmoElement_XAXIS;
                Ray *axis       = axes[axis_index];
                
                closest_distance_ray_ray(&camera_ray, axis);
                gizmo_new_point = axis->o + axis->t*axis->d;
            }
            // Planes.
            else
            {
                s32 plane_index = gizmo_element - GizmoElement_XPLANE;
                Plane *plane    = planes[plane_index];
                
                if(ray_plane_intersect(&camera_ray, *plane))
                {
                    gizmo_new_point = camera_ray.o + camera_ray.d*camera_ray.t;
                    gizmo_new_point.I[plane_index] = plane->center.I[plane_index];
                }
            }
            
            // Skip if this is the first frame we start interacting with a gizmo.
            if(length_squared(gizmo_old_point) != 0) 
                delta = gizmo_new_point - gizmo_old_point;
            
            entity->position += delta;
            
            // Store point for next frame to calculate the delta.
            gizmo_old_point = gizmo_new_point;
        }
        else if(gizmo_mode == GizmoMode_ROTATION)
        {
            s32 plane_index    = gizmo_element - GizmoElement_XPLANE;
            Vector3 c          = circles[plane_index]->center;
            f32     r          = circles[plane_index]->radius;
            Vector3 n          = circles[plane_index]->normal;
            
            Plane circle_plane = make_plane(c, n);
            
            if(ray_plane_intersect(&camera_ray, circle_plane))
            {
                Vector3 on_plane  = camera_ray.o + camera_ray.d*camera_ray.t;
                gizmo_new_point   = c + r*normalize(on_plane - c);
            }
            
            // The first frame we start interacting with a rotation gizmo.
            if(length_squared(gizmo_old_point) == 0) gizmo_start_point = gizmo_new_point;
            
            if(length_squared(gizmo_old_point) != 0)
            {
                Vector3 a = normalize(gizmo_old_point - c);
                Vector3 b = normalize(gizmo_new_point - c);
                
                f32 cos   = dot(a, b);
                f32 sin   = dot(cross(a, b), n);
                f32 theta = _atan2(sin, cos);
                
                Quaternion q = make_quaternion_from_axis_and_angle(n, theta);
                
                entity->orientation = q * entity->orientation;
            }
            
            gizmo_old_point = gizmo_new_point;
        }
        
    } // gizmo_is_active
    
    
    
    //
    // Render gizmo.
    //
    if(gizmo_mode == GizmoMode_TRANSLATION) 
        gizmo_render_translation(scale, thickness, plane_scale, colors);
    else if(gizmo_mode == GizmoMode_ROTATION)
        gizmo_render_rotation(thickness, colors);
}

#endif //GIZMO_H
