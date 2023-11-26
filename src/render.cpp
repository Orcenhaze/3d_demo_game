
FUNCTION void render_bounding_box(Game_State *game, Entity *entity, Bounding_Box *bb)
{
    if((game->selected_entity.entity->id == entity->id))
        entity->bb_outline_color = make_vector4(1.0f, 0.0f, 0.0f, 1.0f);
    else
        entity->bb_outline_color = make_vector4(0.0f, 1.0f, 1.0f, 1.0f);
    
    // Bind shader.
    current_shader = &default_shader;
    if(current_shader)
        extensions->glUseProgram(current_shader->id);
    
    // Update transforms.
    Matrix4 unit_cube_to_world_matrix = entity->object_to_world_matrix.non_inverse * bb->unit_cube_to_object_matrix;
    update_render_transform(unit_cube_to_world_matrix);
    
    // Bind buffers and set vertex format.
    extensions->glBindBuffer(GL_ARRAY_BUFFER, bb->vbo_id);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bb->ibo_id);
    set_vertex_format_to_X();
    
    // Set textures and uniforms before drawing.
    set_texture_map(find_texture(default_white_texture)->id, 0, current_shader->location_diffuse_map);
    extensions->glUniform4fv(current_shader->location_base_color, 1, &entity->bb_outline_color.I[0]);
    
    extensions->glDrawElements(GL_LINES, bb->num_indices, GL_UNSIGNED_INT, 0);
    
    // Unbind textures.
    set_texture_map(0);
}

FUNCTION void render_bounding_volume(Game_State *game, Entity *entity, Bounding_Volume *bv)
{
    // Bind shader.
    current_shader = &default_shader;
    if(current_shader)
        extensions->glUseProgram(current_shader->id);
    
    // Update transforms.
    
    
    // Bind buffers and set vertex format.
    extensions->glBindBuffer(GL_ARRAY_BUFFER, bv->vbo_id);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bv->ibo_id);
    set_vertex_format_to_X();
    
    // Set textures and uniforms before drawing.
    set_texture_map(find_texture(default_white_texture)->id, 0, current_shader->location_diffuse_map);
    extensions->glUniform4fv(current_shader->location_base_color, 1, &entity->bv_outline_color.I[0]);
    
    extensions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    extensions->glDrawElements(GL_TRIANGLES, bv->num_indices, GL_UNSIGNED_INT, 0);
    extensions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Unbind textures.
    set_texture_map(0);
}

FUNCTION void render_terrain(Game_State *game, Terrain *terrain)
{
    if(!terrain->vertices) return;
    
    // Bind shader.
    current_shader = &terrain_shader;
    if(!current_shader)
    {
        PRINT("No shader is set.\n");
        ASSERT(false);
    }
    extensions->glUseProgram(current_shader->id);
    
    // Update transforms.
    update_render_transform(terrain->object_to_world_matrix.non_inverse);
    
    // Bind buffers and set vertex format.
    extensions->glBindBuffer(GL_ARRAY_BUFFER, terrain->vbo_id);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain->ibo_id);
    set_vertex_format_to_XU();
    
    // Set textures and uniforms before drawing.
    set_texture_map(terrain->diffuse_map->id, 0, current_shader->location_diffuse_map);
    
    //extensions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    extensions->glDrawElements(GL_TRIANGLES, terrain->num_indices, GL_UNSIGNED_INT, 0);
    //extensions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Unbind textures.
    set_texture_map(0);
}

FUNCTION void render_skybox(Game_State *game, Cube_Map *skybox)
{
    // Bind shader.
    current_shader = &skybox_shader;
    if(!current_shader)
    {
        PRINT("No shader is set.\n");
        ASSERT(false);
    }
    extensions->glUseProgram(current_shader->id);
    
    // Update transforms.
    Matrix4 world_to_view = world_to_view_matrix.non_inverse;
    world_to_view._14 = 0;
    world_to_view._24 = 0;
    world_to_view._34 = 0;
    update_render_transform(matrix4_identity(), world_to_view);
    
    // Bind buffers and set vertex format.
    extensions->glBindBuffer(GL_ARRAY_BUFFER, skybox->vbo_id);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox->ibo_id);
    set_vertex_format_to_X();
    
    // Set textures and uniforms before drawing.
    set_texture_map(skybox->id, 0, current_shader->location_cube_map, GL_TEXTURE_CUBE_MAP);
    
    extensions->glDepthFunc(GL_LEQUAL);
    extensions->glDrawElements(GL_TRIANGLES, skybox->num_indices, GL_UNSIGNED_INT, 0);
    extensions->glDepthFunc(GL_LESS);
    
    // Unbind textures.
    set_texture_map(0, 0, -1, GL_TEXTURE_CUBE_MAP);
}

FUNCTION void render_entity(Game_State *game, Entity *entity)
{
    Triangle_Mesh *mesh = entity->mesh;
    
    if(!mesh)
    {
        PRINT("Entity %d has no mesh!\n", entity->id);
        return;
    }
    
    // Bind shader.
    current_shader = &default_shader;
    if(current_shader)
        extensions->glUseProgram(current_shader->id);
    
    // Update transforms.
    update_render_transform(entity->object_to_world_matrix.non_inverse);
    
    // Bind buffers and set vertex format.
    extensions->glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_id);
    extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo_id);
    set_vertex_format_to_XTBNUC();
    
    // Set uniforms before drawing.
    for(u32 light_index = 0; light_index < MAX_POINT_LIGHTS; light_index++)
    {
        Entity *light_e = game->point_light_entities[light_index];
        if(!light_e) continue;
        
        GLint loc = current_shader->location_point_lights[light_index].location_invert_normal;
        extensions->glUniform1i(loc, 0);
        if((entity->id == light_e->id))
        {
            extensions->glUniform1i(loc, 1);
        }
    }
    extensions->glUniform3fv(current_shader->location_camera_position, 1, &game->active_camera->position.I[0]);
    extensions->glUniform1f(current_shader->location_final_alpha, entity->alpha);
    
    // Draw triangle lists.
    u32 vertex_index_size = sizeof(mesh->indices[0]);
    for(s32 list_index = 0; list_index < mesh->num_triangle_lists; list_index++)
    {
        Triangle_List_Info *list = mesh->triangle_list_info + list_index;
        Material_Info *m         = &mesh->material_info[list->material_index];
        
        if(!list->diffuse_map) continue;
        
        // Set textures and uniforms before drawing.
        extensions->glUniform4fv(current_shader->location_base_color, 1, &m->base_color.I[0]);
        extensions->glUniform1f(current_shader->location_specular,        m->specular);
        extensions->glUniform1f(current_shader->location_roughness,       m->roughness);
        set_texture_map(list->diffuse_map->id, 0, current_shader->location_diffuse_map);
        
        if(list->normal_map)
        {
            // Use normal map.
            extensions->glUniform1i(current_shader->location_use_normal_map, 1);
            set_texture_map(list->normal_map->id, 1, current_shader->location_normal_map);
        }
        else
        {
            // Don't use normal map.
            extensions->glUniform1i(current_shader->location_use_normal_map, 0);
        }
        
        umm offset = list->first_index * vertex_index_size;
        extensions->glDrawElements(GL_TRIANGLES, list->num_indices, GL_UNSIGNED_INT, (void *) offset);
        
        // Unbind textures.
        set_texture_map(0);
        set_texture_map(0, 1);
    }
    
    
#if DEBUG_BUILD
    if(is_set(entity, EntityFlags_BV_VISIBLE))
    {
        render_bounding_volume(game, entity, &entity->mesh->bounding_volume);
    }
#endif
    
    update_render_transform(matrix4_identity());
}

FUNCTION void update_point_light_uniforms(Game_State *game)
{
    for(u32 light_index = 0; light_index < MAX_POINT_LIGHTS; light_index++)
    {
        Entity *light_e = game->point_light_entities[light_index];
        if(!light_e) continue; 
        
        for(u32 shader_index = 0; shader_index < ARRAY_COUNT(all_shaders); shader_index++)
        {
            Shader *shader  = all_shaders[shader_index];
            extensions->glUseProgram(shader->id);
            
            GLint   loc_num = shader->location_num_point_lights;
            GLint   loc_pos = shader->location_point_lights[light_index].location_position;
            GLint   loc_col = shader->location_point_lights[light_index].location_color;
            GLint   loc_nor = shader->location_point_lights[light_index].location_invert_normal;
            
            if(loc_num != -1)
            {
                extensions->glUniform1i(loc_num, game->num_point_light_entities);
            }
            
            if(loc_pos != -1)
            {
                extensions->glUniform3fv(loc_pos, 1, &light_e->position.I[0]);
            }
            
            if(loc_col != -1)
            {
                if(is_cleared(light_e, EntityFlags_VISIBLE) && (game->active_camera != &game->game_camera)) 
                    extensions->glUniform3fv(loc_col, 1, &VECTOR3_ZERO.I[0]);
                else
                    extensions->glUniform3fv(loc_col, 1, &light_e->light_color.I[0]);
            }
            
            //
            // Other Point_Light attributes.
            //
        }
    }
}

FUNCTION void update_entities(Game_State *game, Game_Input *input, f32 dt)
{
    Button_State *kb = input->keyboard_buttons;
    
    for(Entity_Iterator it = iterate_all_entities(game);
        it.entity;
        advance(&it))
    {
        Entity *e = it.entity;
        
        if(!game->use_debug_camera)
        {
            switch(e->type)
            {
                case EntityType_PLAYER:
                {
                    if(!game->use_debug_camera)
                    {
                        e->acceleration = {};
                        b32 jump        = false;
                        
                        if(is_down(kb[Key_D]))
                        {
                            e->acceleration.x = 1.0f;
                            
                            if(is_down(kb[Key_A])) e->acceleration.x = 0.0f;
                        }
                        
                        if(is_down(kb[Key_A]))
                        {
                            e->acceleration.x = -1.0f;
                            
                            if(is_down(kb[Key_D])) e->acceleration.x = 0.0f;
                        }
                        
                        if(is_down(kb[Key_W]))
                        {
                            e->acceleration.z = -1.0f;
                            
                            if(is_down(kb[Key_S])) e->acceleration.z = 0.0f;
                        }
                        
                        if(is_down(kb[Key_S]))
                        {
                            e->acceleration.z = 1.0f;
                            
                            if(is_down(kb[Key_W])) e->acceleration.z = 0.0f;
                        }
                        
                        if(is_down(&kb[Key_E], 0.5f))
                        {
                            add_projectile(game, e);
                        }
                        
                        if(was_pressed(kb[Key_SPACE]))
                        {
                            jump = true;
                        }
                        
                        
                        
                        // Update game camera.
                        if(was_pressed(kb[Key_V]))
                        {
                            game->invert_camera = !game->invert_camera;
                        }
                        if(game->invert_camera)
                        {
                            game->offset_camera.z = _force_sign(game->offset_camera.z, -1);
                            
                            // Invert all movement.
                            e->acceleration = -e->acceleration;
                        }
                        else
                        {
                            game->offset_camera.z = _force_sign(game->offset_camera.z,  1);
                        }
                        game->game_camera.position = e->position + game->offset_camera;
                        game->game_camera.front    = e->position;
                        game->game_camera.up       = VECTOR3_U;
                        world_to_view_matrix       = look_at(game->game_camera.position, 
                                                             game->game_camera.front, 
                                                             game->game_camera.up);
                        
                        
                        
                        // Update orientation and position.
                        b32 moved_this_frame = (length_squared(e->acceleration) != 0.0f);
                        if(moved_this_frame)
                            e->facing_direction = normalize(e->acceleration);
                        update_entity_orientation_from_facing_direction(e, e->facing_direction, 
                                                                        dt, 14.0f);
                        update_entity_position(game, e, e->acceleration, dt, 180.f, jump, true);
                        
                        if(string_match(e->mesh->name, STRING_LITERAL("ball.mesh")))
                            set_flags(e, EntityFlags_IS_SPHERE);
                        else
                            clear_flags(e, EntityFlags_IS_SPHERE);
                        
                        // Update death.
                        if(is_set(e, EntityFlags_DEAD))
                        {
                            if(e->time_dead > e->death_duration)
                            {
                                clear_flags(e, EntityFlags_DEAD);
                                e->mesh      = find_mesh(STRING_LITERAL("spaceman.mesh"));
                                e->time_dead = 0.0f;
                            }
                            else
                            {
                                e->time_dead += dt;
                            }
                        }
                    }
                } break;
                
                case EntityType_INANIMATE:
                {
                    if(is_set(e, EntityFlags_ROTATE_CCW))
                    {
                        e->facing_direction = make_quaternion_from_axis_and_angle(VECTOR3_U, dt) * e->facing_direction;
                        update_entity_orientation_from_facing_direction(e, e->facing_direction, dt);
                    }
                    if(is_set(e, EntityFlags_ROTATE_CW))
                    {
                        e->facing_direction = make_quaternion_from_axis_and_angle(VECTOR3_U, -dt) * e->facing_direction;
                        update_entity_orientation_from_facing_direction(e, e->facing_direction, dt);
                    }
                    if(is_set(e, EntityFlags_PUSHABLE))
                    {
                        update_entity_position(game, e, VECTOR3_ZERO, dt, 180.0f, false, true);
                    }
                    if(is_set(e, EntityFlags_IS_SPHERE))
                    {
                        Collision_Data col = {};
                        if(entity_collision_test(game, global_player, e, &col))
                        {
                            Vector3 n = transform_direction(e->object_to_world_matrix.non_inverse, col.normal);
                            
                            if((dot(n, VECTOR3_U) >= 0.80f) &&
                               (is_cleared(global_player, EntityFlags_IS_SPHERE)))
                            {
                                set_flags(global_player, EntityFlags_IS_SPHERE);
                                global_player->mesh = find_mesh(STRING_LITERAL("ball.mesh"));
                            }
                        }
                    }
                } break;
                
                case EntityType_GHOST:
                {
                    if(rect3_is_zero(e->move_area_bounds)) break;
                    
                    f32 distance_to_dest = length(e->move_destination - e->position);
                    f32 speed            = 3.0f;
                    if((length_squared(global_player->position - e->position) <= square(30.0f))                               &&
                       (is_cleared(global_player, EntityFlags_DEAD)) &&
                       (is_cleared(global_player, EntityFlags_IS_SPHERE)))
                    {
                        e->move_destination = global_player->position;
                        distance_to_dest    = length(e->move_destination - e->position);
                        speed               = 15.0f;
                    }
                    else if(distance_to_dest <= 0.1f)
                    {
                        e->move_destination = vector3_rand(e->move_area_bounds);
                        distance_to_dest    = length(e->move_destination - e->position);
                    }
                    
                    f32 t       = dt * speed / distance_to_dest;
                    e->position = lerp(e->position, t, e->move_destination);
                    
                    e->facing_direction = normalize(e->move_destination - e->position);
                    update_entity_orientation_from_facing_direction(e, e->facing_direction, dt, 8.0f);
                    
                    Collision_Data ignored = {};
                    if(entity_collision_test(game, e, global_player, &ignored) &&
                       is_cleared(global_player, EntityFlags_DEAD))
                    {
                        set_flags(global_player, EntityFlags_DEAD);
                        global_player->mesh = find_mesh(STRING_LITERAL("skull.mesh"));
                    }
                } break;
                
                case EntityType_SKULL:
                {
                    if(rect3_is_zero(e->move_area_bounds)) break;
                    
                    // Movement and orientation.
                    Vector3 min = e->move_area_bounds.min;
                    Vector3 max = e->move_area_bounds.max;
                    Vector3 p00 = make_vector3(min.x, e->position.y, min.z);;
                    Vector3 p01 = make_vector3(max.x, e->position.y, min.z);
                    Vector3 p11 = make_vector3(max.x, e->position.y, max.z);
                    Vector3 p10 = make_vector3(min.x, e->position.y, max.z);
                    f32 close_dist   = 0.02f;
                    
                    if(is_set(e, EntityFlags_ROTATE_CW))
                    {
                        if(length_squared(p00 - e->position) < square(close_dist))
                        {
                            e->move_destination = p01;
                        }
                        else if(length_squared(p01 - e->position) < square(close_dist))
                        {
                            e->move_destination = p11;
                        }
                        else if(length_squared(p11 - e->position) < square(close_dist))
                        {
                            e->move_destination = p10;
                        }
                        else if(length_squared(p10 - e->position) < square(close_dist))
                        {
                            e->move_destination = p00;
                        }
                    }
                    else if(is_set(e, EntityFlags_ROTATE_CCW))
                    {
                        if(length_squared(p00 - e->position) < square(close_dist))
                        {
                            e->move_destination = p10;
                        }
                        else if(length_squared(p10 - e->position) < square(close_dist))
                        {
                            e->move_destination = p11;
                        }
                        else if(length_squared(p11 - e->position) < square(close_dist))
                        {
                            e->move_destination = p01;
                        }
                        else if(length_squared(p01 - e->position) < square(close_dist))
                        {
                            e->move_destination = p00;
                        }
                    }
                    
                    e->facing_direction = normalize(e->move_destination - e->position);
                    e->acceleration     = e->facing_direction;
                    update_entity_orientation_from_facing_direction(e, e->facing_direction, dt, 15.0f);
                    update_entity_position(game, e, e->acceleration, dt, 80.f, false, true);
                    
                    // Color.
                    f32 distance_to_player_squared = length_squared(e->position - global_player->position);
                    f32 far_dist = 30.0f;
                    Material_Info *m     = &e->mesh->material_info[0];
                    if((distance_to_player_squared > square(far_dist)) ||
                       is_set(global_player, EntityFlags_DEAD))
                    {
                        m->base_color = m->original_base_color;
                    }
                    else if(distance_to_player_squared < square(close_dist))
                    {
                        m->base_color = RED;
                    }
                    else
                    {
                        f32 t = 1.0f - ((_square_root(distance_to_player_squared) - close_dist)
                                        / 
                                        (far_dist - close_dist));
                        m->base_color = lerp(m->original_base_color, t, RED);
                    }
                } break;
            }
            
            
            update_entity_visibility(game, e);
        }
        
#if DEBUG_BUILD
        // Render move_area_bounds.
        if(!vector3_is_zero(e->move_area_bounds.min))
        {
            immediate_begin(GL_LINE);
            immediate_cuboid(rect3_get_center(e->move_area_bounds), rect3_get_scale(e->move_area_bounds)/2.0f, GREEN);
            immediate_flush();
        }
#endif
        
        update_entity_transform(e);
    }
}

FUNCTION void render_entities(Game_State *game, Game_Input *input, f32 dt)
{
    Button_State *kb = input->keyboard_buttons;
    
    for(Entity_Iterator it = iterate_all_entities(game);
        it.entity;
        advance(&it))
    {
        Entity *e = it.entity;
        
        if(is_set(e, EntityFlags_VISIBLE))
        {
            render_entity(game, e);
        }
    }
}
