
FUNCTION void find_next_entity(Entity_Iterator *it)
{
    it->entity = 0;
    
    while(it->index < it->level_entity_count)
    {
        Entity *e = &it->level_entities[it->index];
        
        if(e->type != EntityType_INVALID)
        {
            it->entity = e;
            return;
        }
        
        it->index++;
    }
    
    while((it->index - it->level_entity_count) < it->editor_entity_count)
    {
        Entity *e = &it->editor_entities[(it->index - it->level_entity_count)];
        
        if(e->type != EntityType_INVALID)
        {
            it->entity = e;
            return;
        }
        
        it->index++;
    }
}

FUNCTION Entity_Iterator iterate_all_entities(Game_State *game)
{
    Entity_Iterator it;
    it.index = 0;
    
    it.level_entity_count  = game->level_entity_count;
    it.level_entities      = game->level_entities;
    it.editor_entity_count = game->next_editor_entity;
    it.editor_entities     = game->editor_entities;
    
    find_next_entity(&it);
    
    return it;
}

FUNCTION void advance(Entity_Iterator *it)
{
    ASSERT(it->entity);
    it->index++;
    find_next_entity(it);
}






FUNCTION Entity * get_entity(Game_State *game, u32 id)
{
    Entity *result = game->level_entities + id;
    
    if(!(result) || (result->id != id))
    {
        PRINT("Entity with id %d not found!\n", id);
    }
    
    return result;
}

FUNCTION void update_entity_transform(Entity *entity)
{
    Vector3 position       = entity->position;
    Quaternion orientation = entity->orientation;
    Vector3 scale          = entity->scale;
    
    Matrix4 m_non = matrix4_identity();
    m_non._14 = position.x;
    m_non._24 = position.y;
    m_non._34 = position.z;
    m_non._11 = scale.x;
    m_non._22 = scale.y;
    m_non._33 = scale.z;
    
    Matrix4 m_inv = matrix4_identity();
    m_inv._14 = -position.x;
    m_inv._24 = -position.y;
    m_inv._34 = -position.z;
    m_inv._11 = 1.0f/scale.x;
    m_inv._22 = 1.0f/scale.y;
    m_inv._33 = 1.0f/scale.z;
    
    Matrix4 r = make_rotation_matrix(orientation);
    
    entity->object_to_world_matrix.non_inverse = m_non * r;
    entity->object_to_world_matrix.inverse     = transpose(r) * m_inv;
    
    // Update collision_box.
    entity->bb_bounds_world.min = entity->object_to_world_matrix.non_inverse * entity->mesh->bounding_box.bounds.min;
    entity->bb_bounds_world.max = entity->object_to_world_matrix.non_inverse * entity->mesh->bounding_box.bounds.max;
    swap_min_max_vectors(&entity->bb_bounds_world.min, &entity->bb_bounds_world.max);
}

FUNCTION Entity * select_entity(Game_State *game, Selected_Entity *se, Entity *entity)
{
    Matrix4 world_to_object_matrix = entity->object_to_world_matrix.inverse;
    
    Ray ray   = {};
    ray.o     = world_to_object_matrix * game->active_camera->position;
    Vector3 d = normalize(game->mouse_world - game->active_camera->position);
    ray.d     = (world_to_object_matrix * make_vector4(d, 0.0f)).xyz;
    
    ray_box_intersect(&ray, entity->mesh->bounding_box.bounds);
    
    // @Note: We are not normalizing ray_d because we don't have to...?
    // If you want to normalize it, remember that we may be transforming ray_d by a 
    // non uniform scale matrix, which means we have to store ray_d's length before
    // normalizing and losing the scale information.
    // After doing the intersection test, we "unscale" the t_hit and only then can we
    // use it to compare with other t values.
    //
    // f32 rl = length(ray_d);
    // ray_d  = normalize(ray_d);
    // t_hit  = t_hit/rl;
    
    if(ray.t < se->sort_index)
    {
        se->sort_index = ray.t;
        se->entity     = entity;
    }
    
    return se->entity;
}

FUNCTION void entity_ground_check(Game_State *game, Entity *entity)
{
    clear_flags(entity, EntityFlags_ON_GROUND);
    
    Vector3 pos            = entity->position;
    f32 distance_to_ground = rect3_get_scale(entity->bb_bounds_world).y;
    distance_to_ground     = (distance_to_ground/2.0f) + 0.1f;
    
    for(Entity_Iterator it = iterate_all_entities(game);
        it.entity;
        advance(&it))
    {
        Entity *e = it.entity;
        if(e->id == entity->id) continue;
        
        Matrix4 world_to_e = e->object_to_world_matrix.inverse;
        
        Ray down  = {};
        down.o    = world_to_e * pos;
        down.d    = (world_to_e * make_vector4(-VECTOR3_U, 0.0f)).xyz;
        
        if(ray_box_intersect(&down, e->mesh->bounding_box.bounds))
        {
            if(down.t < distance_to_ground)
            {
                set_flags(entity, EntityFlags_ON_GROUND);
                break;
            }
        }
    }
}

FUNCTION b32 entity_collision_test(Game_State *game, Entity *main_entity, Entity *test_entity,
                                   Collision_Data *col_data_out)
{
    // @Note: Collisions happen in local space of the test_entity. When the function returns,
    // you would need to transform the data from col_data_out into world space using 
    // test_entity's object_to_world_matrix.
    //
    // The resolution normal goes from test_entity to main_entity, for example:
    // main_entity<--test_entity
    // test_entity-->main_entity
    // etc...
    //
    // A: main_entity's bounding volume.
    // B: test_entity's bounding volume.
    
    b32 result = false;
    
    Bounding_Volume *main_volume = &main_entity->mesh->bounding_volume;
    Matrix4 A_to_world           = main_entity->object_to_world_matrix.non_inverse;
    Memory_Arena *temp_arena     = &game->arena_list.temp_arena;
    
    Matrix4 B_to_world = test_entity->object_to_world_matrix.non_inverse;
    Matrix4 world_to_B = test_entity->object_to_world_matrix.inverse;
    
    // @Debug: Incorrect transform for scaled objects.
    Matrix4 A_to_B     = world_to_B * A_to_world;
    Bounding_Volume A  = transform_bounding_volume(temp_arena, A_to_B, main_volume);
    Bounding_Volume *B = &test_entity->mesh->bounding_volume;
    
    if(!A.is_sphere && !B->is_sphere)    // hull-hull
    {
        if(sat_test(&A, B, col_data_out))
        {
            col_data_out->result = true;
        }
    }
    else if(A.is_sphere && B->is_sphere) // sphere-sphere
    {
        Vector3 center_a = world_to_B * main_entity->position;
        Vector3 center_b = make_vector3(0.0f);
        
        // @Hack: get_radius() is such a dumb hack...
        f32 radius_a     = get_radius(center_a, &A);
        f32 radius_b     = get_radius(center_b,  B);
        
        if(sphere_overlap_test(center_a, radius_a, center_b, radius_b, col_data_out))
        {
            col_data_out->result = true;
        }
    }
    else if(!A.is_sphere && B->is_sphere) // hull-sphere
    {
        Vector3 center_b = make_vector3(0.0f);
        f32 radius_b     = get_radius(center_b, B);
        
        f32 d = closest_distance_point_volume(center_b, &A, col_data_out);
        if(d < radius_b)
        {
            col_data_out->penetration = radius_b - d;
            col_data_out->normal      = -col_data_out->normal;
            col_data_out->result      = true;
        }
    }
    else if(A.is_sphere && !B->is_sphere) // sphere-hull
    {
        // @Debug: Cylinders won't work until we find a good way to export the radius.
        
        Vector3 center_a = world_to_B * main_entity->position;
        f32 radius_a     = get_radius(center_a, &A);
        
        f32 d = closest_distance_point_volume(center_a, B, col_data_out);
        if(d < radius_a)
        {
            col_data_out->penetration = radius_a - d;
            col_data_out->normal      = col_data_out->normal;
            col_data_out->result      = true;
        }
    }
    
    clear_arena(temp_arena);
    
    result = col_data_out->result;
    
    return result;
}

FUNCTION void update_entity_position(Game_State *game, Entity *entity, 
                                     Vector3 acceleration, f32 dt, f32 movement_speed, b32 jump, b32 do_collision_test)
{
    entity_ground_check(game, entity);
    
    // Terrain collision.
    Collision_Data terrain_col = {};
    if(terrain_collision_world(entity, &game->terrain, &terrain_col))
    {
        set_flags(entity, EntityFlags_ON_GROUND);
        entity->position.y += terrain_col.penetration;
    }
    
    // To make diagonal displacement same as cardinal displacement.
    acceleration     = normalize(acceleration);
    
    f32 entity_speed = movement_speed;
    acceleration    *= entity_speed;
    
    // @Hack: Gravity. Ideally, we want ballistic motion.
    Vector3 gravity  = {0.0f, -entity_speed, 0.0f};
    
    if(is_set(entity, EntityFlags_ON_GROUND))
    {
        gravity = {};
        
        // @Hack: Jump. Ideally, we want ballistic motion.
        if(jump)
        {
            entity->velocity += make_vector3(0.0f, 120.0f, 0.0f);
        }
    }
    
    // @Hack: Friction.
    acceleration         += -7.0f*entity->velocity; 
    acceleration         += gravity;
    Vector3 entity_delta  = entity->velocity*dt + 0.5f*acceleration*square(dt);
    entity->velocity     += acceleration*dt;
    entity->position     += entity_delta;
    
    if(do_collision_test)
    {
        // Collisions of Player Entity vs. Level Entities. 
        for(Entity_Iterator it = iterate_all_entities(game);
            it.entity;
            advance(&it)) 
        {
            Entity *test_entity = it.entity;
            if(entity->id == test_entity->id ||
               is_cleared(test_entity, EntityFlags_COLLIDES)) 
                continue;
            
            Collision_Data col_data = {};
            if(entity_collision_test(game, entity, test_entity, &col_data))
            {
                // @Debug: Penetration depth could be wrong for scaled objects.
                Vector3 mtv      = col_data.normal*col_data.penetration;
                Vector3 pos_in_B = test_entity->object_to_world_matrix.inverse * entity->position;
                pos_in_B        += mtv;
                
                entity->position = test_entity->object_to_world_matrix.non_inverse * pos_in_B;
            }
        }
    }
}

FUNCTION void update_entity_orientation_from_facing_direction(Entity *entity, Vector3 facing_direction, f32 dt, f32 speed = 1.0f)
{
    f32 cos   = dot(VECTOR3_R, facing_direction);
    f32 sin   = dot(cross(VECTOR3_R, facing_direction), VECTOR3_U);
    f32 theta = _atan2(sin, cos);
    
    Quaternion new_orientation = make_quaternion_from_axis_and_angle(VECTOR3_U, theta);
    
    f32 t = dt * speed;
    entity->orientation = lerp(entity->orientation, t, new_orientation);
}

FUNCTION void update_entity_visibility(Game_State *game, Entity *entity)
{
    if(entity->type == EntityType_INVALID) return;
    
    clear_flags(entity, EntityFlags_BB_VISIBLE);
    
#if DEBUG_BUILD
    set_flags(entity, EntityFlags_BV_VISIBLE);
#else
    clear_flags(entity, EntityFlags_BV_VISIBLE);
#endif
    
    if(entity->type != EntityType_PLAYER)
    {
        if(point_inside_box_test(game->game_camera.position, entity->bb_bounds_world))
            clear_flags(entity, EntityFlags_VISIBLE);
        else
            set_flags(entity, EntityFlags_VISIBLE);
    }
}

FUNCTION Entity * add_projectile(Game_State *game, Entity *parent)
{
    game->next_projectile = game->next_projectile % MAX_PROJECTILE_COUNT;
    Entity *p             = game->projectiles + game->next_projectile++;
    
    Vector3 fd            = parent->facing_direction;
    p->id                 = parent->id;
    p->position           = parent->position + 1.5f*fd;
    p->velocity           = VECTOR3_ZERO;
    p->orientation        = make_quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    update_entity_orientation_from_facing_direction(p, fd, 1);
    p->scale              = make_vector3(0.5f);
    
    p->type               = EntityType_PROJECTILE;
    p->mesh               = find_mesh(STRING_LITERAL("stone1.mesh"));
    
    //  We can change the behavior of the projectile based on its type.
    p->facing_direction   = fd;
    p->acceleration       = fd;
    p->projectile_speed   = 800.0f;
    p->distance_remaining = 50.0f;
    
    set_flags(p, EntityFlags_VISIBLE);
    set_flags(p, EntityFlags_COLLIDES);
    set_flags(p, EntityFlags_BV_VISIBLE);
    clear_flags(p, EntityFlags_BB_VISIBLE);
    
    p->bb_outline_color   = make_vector4(0.0f, 1.0f, 1.0f, 1.0f);
    p->bv_outline_color   = make_vector4(1.0f);
    p->alpha              = 1.0f;
    
    update_entity_transform(p);
    
    return p;
}
