
FUNCTION void parse_entity(Lexer *lexer, Entity *e)
{
    while(!lexer->error)
    {
        Token token = get_token(lexer);
        
        if(token.type == TokenType_ERROR)
        {
            lexer->error = true;
            PRINT("Parse Error: TokenType_ERROR \"%S\" at %S::%d::%d\n", 
                  token.string_value, token.file_name, token.line, token.column);
        }
        else if(token_match(token, "type"))
        {
            Token tok = require_token(lexer, TokenType_INTEGER);
            e->type   = (Entity_Type) tok.s32_value;
        }
        else if(token_match(token, "pos"))
        {
            for(s32 i = 0; i < 3; i++)
            {
                Token tok        = require_token(lexer, TokenType_FLOAT);
                e->position.I[i] = tok.f32_value;
            }
        }
        else if(token_match(token, "ori"))
        {
            Vector3 axis = {};
            for(s32 i = 0; i < 3; i++)
            {
                Token tok = require_token(lexer, TokenType_FLOAT);
                axis.I[i] = tok.f32_value;
            }
            
            Token tok_angle = require_token(lexer, TokenType_FLOAT);
            f32 angle       = tok_angle.f32_value;
            
            e->orientation = make_quaternion_from_axis_and_angle(axis, to_radians(angle));
        }
        else if(token_match(token, "scale"))
        {
            for(s32 i = 0; i < 3; i++)
            {
                Token tok     = require_token(lexer, TokenType_FLOAT);
                e->scale.I[i] = tok.f32_value;
            }
        }
        else if(token_match(token, "facing"))
        {
            for(s32 i = 0; i < 3; i++)
            {
                Token tok                = require_token(lexer, TokenType_FLOAT);
                e->facing_direction.I[i] = tok.f32_value;
            }
        }
        else if(token_match(token, "light_color"))
        {
            for(s32 i = 0; i < 3; i++)
            {
                Token tok           = require_token(lexer, TokenType_FLOAT);
                e->light_color.I[i] = tok.f32_value;
            }
        }
        else if(token_match(token, "move_area_scale"))
        {
            for(s32 i = 0; i < 3; i++)
            {
                Token tok               = require_token(lexer, TokenType_FLOAT);
                e->move_area_scale.I[i] = tok.f32_value;
            }
            
        }
        else if(token_match(token, "flags"))
        {
            Token tok = require_token(lexer, TokenType_INTEGER);
            e->flags  = tok.s32_value;
        }
        else if(token_match(token, "mesh"))
        {
            Token tok_mesh = require_token(lexer, TokenType_STRING);
            e->mesh     = find_mesh(tok_mesh.string_value);
            
            ASSERT(e->mesh);
            
            // !!!
            // @Todo: We really shouldn't initalize entities' default data here.
            // !!!
            if(!e->mesh->bounding_volume.vertices)
                clear_flags(e, EntityFlags_COLLIDES);
            else if(e->mesh->bounding_volume.is_sphere)
            {
                set_flags(e, EntityFlags_IS_SPHERE);
            }
            
            
            e->bb_outline_color = make_vector4(0.0f, 1.0f, 1.0f, 1.0f);
            e->bv_outline_color = make_vector4(1.0f);
            e->alpha            = 1.0f;
            
            if(length_squared(e->facing_direction) == 0.0f)
                e->facing_direction = VECTOR3_R;
            
            if(e->type == EntityType_PLAYER)
            {
                global_player     = e;
                e->death_duration = 15.0f;
            }
            
            if(e->type == EntityType_GHOST && !vector3_is_zero(e->move_area_scale))
            {
                e->alpha = 0.8f;
                
                e->move_area_bounds.min = e->position - e->move_area_scale;
                e->move_area_bounds.max = e->position + e->move_area_scale;
                swap_min_max_vectors(&e->move_area_bounds.min, &e->move_area_bounds.max);
                
                e->move_destination = vector3_rand(e->move_area_bounds);
                e->facing_direction = normalize(e->move_destination - e->position);
            }
            
            if(e->type == EntityType_SKULL && !vector3_is_zero(e->move_area_scale))
            {
                e->move_area_bounds.min = e->position - e->move_area_scale;
                e->move_area_bounds.max = e->position + e->move_area_scale;
                swap_min_max_vectors(&e->move_area_bounds.min, &e->move_area_bounds.max);
                
                e->move_destination = e->move_area_bounds.min;
                e->facing_direction = normalize(e->move_destination - e->position);
            }
            
            set_flags(e, EntityFlags_VISIBLE);
            
            update_entity_transform(e);
            
            break;
        }
        else
        {
            lexer->error = true;
            PRINT("Parse Error: can't recognize token \"%S\" at %S::%d::%d\n", 
                  token.string_value, token.file_name, token.line, token.column);
        }
    }
}

FUNCTION void parse_level(Game_State *game, String level_full_path)
{
    String file = platform_api->read_entire_file(level_full_path);
    
    if(!file.data)
    {
        PRINT("Load Error: couldn't load level data from file: %S\n", level_full_path);
        platform_api->free_file_memory(file.data);
        
        ASSERT(false);
        //return;
    }
    
    String original_file = file;
    
    Lexer lexer_ = make_lexer(level_full_path, file);
    Lexer *lexer = &lexer_;
    
    // @Note: Index zero is an invalid entity.
    s32 next_entity_index = 1;
    
    while(!lexer->error)
    {
        Token token = peek_token(lexer);
        
        if(token.type == TokenType_ERROR)
        {
            lexer->error = true;
            PRINT("Parse Error: TokenType_ERROR \"%S\" at %S::%d::%d\n", 
                  token.string_value, token.file_name, token.line, token.column);
        }
        else if(token.type == TokenType_END_OF_FILE)
        {
            PRINT("%S was parsed successfully!\n", token.file_name);
            break;
        }
        else if(token_match(token, "terrain_height_map"))
        {
            eat_token(lexer);
            
            Token str_token = require_token(lexer, TokenType_STRING);
            
            if(!string_empty(str_token.string_value))
                game->terrain.height_map = find_texture(str_token.string_value);
        }
        else if(token_match(token, "terrain_diffuse_map"))
        {
            eat_token(lexer);
            
            Token str_token = require_token(lexer, TokenType_STRING);
            
            if(!string_empty(str_token.string_value))
                game->terrain.diffuse_map = find_texture(str_token.string_value);
        }
        else if(token_match(token, "skybox_cube_map"))
        {
            eat_token(lexer);
            
            Token str_token = require_token(lexer, TokenType_STRING);
            
            if(!string_empty(str_token.string_value))
            {
                game->skybox.directory_full_path = push_stringf(&game->arena_list.level_arena, "%stexture_data/cube_maps/%S", global_data_directory_full_path, str_token.string_value);
            }
        }
        else if(token_match(token, "music"))
        {
            eat_token(lexer);
            
            Token str_token = require_token(lexer, TokenType_STRING);
            
            if(!string_empty(str_token.string_value))
                game->music_file_name = push_string_copy(&game->arena_list.level_arena, str_token.string_value);
        }
        else if(token_match(token, "entity_count"))
        {
            eat_token(lexer);
            
            Token count_token = require_token(lexer, TokenType_INTEGER);
            
            game->level_entity_count = count_token.s32_value;
            game->level_entities     = PUSH_ARRAY(&game->arena_list.level_arena, game->level_entity_count, Entity);
            
            // @Note: To always ensure that the first level entity is zero.
            MEMORY_SET(game->level_entities, 0, sizeof(Entity));
        }
        else if(token_match(token, "type"))
        {
            Entity *e = game->level_entities + next_entity_index;
            e->id     = next_entity_index;
            
            parse_entity(lexer, e);
            
            next_entity_index++;
        }
        else
        {
            lexer->error = true;
            PRINT("Parse Error: can't recognize token \"%S\" at %S::%d::%d\n", 
                  token.string_value, token.file_name, token.line, token.column);
        }
    }
    
    platform_api->free_file_memory(original_file.data);
}