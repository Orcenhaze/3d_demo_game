
FUNCTION void zoom_object(Game_State *game, Entity *e)
{
    if(game->active_camera != &game->debug_camera) return;
    
    f32 obj_height    = e->bb_bounds_world.max.y - e->bb_bounds_world.min.y;
    Ray obj_to_camera;
    obj_to_camera.o = e->position;
    obj_to_camera.d = normalize(game->debug_camera.position - e->position);
    
    f32 t    = obj_height + obj_height/3.0f;
    Vector3 target = obj_to_camera.o + t*obj_to_camera.d;
    
    
    game->debug_camera.position = target;
    game->debug_camera.front    = -obj_to_camera.d;
    world_to_view_matrix = look_at(game->debug_camera.position, 
                                   e->position,
                                   game->debug_camera.up);
}

FUNCTION void push(Selected_Entity *se)
{
    se->current_history_index = se->current_history_index % MAX_HISTORY_RECORD_COUNT;
    
    MEMORY_COPY(&se->history_stack[se->current_history_index++], se->entity, sizeof(Entity));
    
    se->history_record_count = se->current_history_index;
}

FUNCTION b32 undo(Selected_Entity *se)
{
    if(se->current_history_index == 0) return false;
    
    MEMORY_COPY(se->entity, &se->history_stack[--se->current_history_index], sizeof(Entity));
    
    return true;
}

FUNCTION b32 redo(Selected_Entity *se)
{
    if(se->current_history_index == se->history_record_count) return false;
    
    MEMORY_COPY(se->entity, &se->history_stack[se->current_history_index++], sizeof(Entity));
    
    return true;
}

FUNCTION void add_editor_entity(Game_State *game, Selected_Entity *se, Ray camera_ray)
{
    game->next_editor_entity = game->next_editor_entity % MAX_EDITOR_ENTITY_COUNT;
    Entity *editor_entity    = game->editor_entities + game->next_editor_entity;
    
    MEMORY_COPY(editor_entity, se->entity, sizeof(Entity));
    
    editor_entity->id       = game->level_entity_count + game->next_editor_entity++;
    editor_entity->position = camera_ray.o + camera_ray.d*length(camera_ray.o - se->entity->position);
}

#include "gizmo.h"