#ifndef ENTITY_H
#define ENTITY_H

enum Entity_Type
{
    EntityType_INVALID,
    
    EntityType_PLAYER,
    EntityType_INANIMATE,
    EntityType_GHOST,
    EntityType_SKULL,
    EntityType_PROJECTILE
};

enum Entity_Flags
{
    EntityFlags_VISIBLE         = 0x1,
    EntityFlags_COLLIDES        = 0x2,
    EntityFlags_ON_GROUND       = 0x4,
    EntityFlags_BB_VISIBLE      = 0x8,
    EntityFlags_BV_VISIBLE      = 0x10,
    EntityFlags_ROTATE_CCW      = 0x20,
    EntityFlags_ROTATE_CW       = 0x40,
    EntityFlags_PUSHABLE        = 0x80,
    EntityFlags_EMISSIVE        = 0x100,
    EntityFlags_DEAD            = 0x200,
    EntityFlags_IS_SPHERE       = 0x400,
    
    //EntityFlags_      = 0x800,
};

struct Entity
{
    u32            id;
    
    Vector3        position;
    Vector3        velocity;
    Quaternion     orientation;
    Vector3        scale;
    
    Triangle_Mesh *mesh;
    
    Entity_Type    type;
    u32            flags;
    
    // @Note: Bounding_Box min and max in world space.
    // - Computed in render_entity().
    // - Always (min < max).
    // - Axis aligned.
    Rect3 bb_bounds_world;
    
    Vector3 acceleration;
    Vector3 facing_direction;
    
    // Projectile stuff.
    f32     distance_remaining;
    f32     projectile_speed;
    
    Matrix4_Inverse object_to_world_matrix;
    
    Vector4 bb_outline_color;
    Vector4 bv_outline_color;
    Vector3 light_color;
    
    //Vector3 move_area_center;
    Vector3 move_area_scale;
    Rect3   move_area_bounds;
    Vector3 move_destination;
    
    // In seconds.
    f32     death_duration;
    f32     time_dead;
    f32     alpha;
};

struct Entity_Iterator
{
    Entity *entity;
    umm     index;
    
    u32     level_entity_count;
    Entity *level_entities;
    u32     editor_entity_count;
    Entity *editor_entities;
};

inline b32 is_set(Entity *entity, u32 flag)
{
    b32 result = entity->flags & flag;
    return result;
}

inline b32 is_cleared(Entity *entity, u32 flag)
{
    b32 result = (entity->flags & flag) == 0;
    return result;
}

inline void set_flags(Entity *entity, u32 flag)
{
    entity->flags |= flag;
}

inline void clear_flags(Entity *entity, u32 flag)
{
    entity->flags &= ~flag;
}

#endif //ENTITY_H
