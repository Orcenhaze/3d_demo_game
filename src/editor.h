#ifndef EDITOR_H
#define EDITOR_H

struct Selected_Entity
{
    f32 sort_index; // Less is better.
    Entity *entity;
    
#define MAX_HISTORY_RECORD_COUNT 32
    u32    current_history_index;
    u32    history_record_count;
    Entity history_stack[MAX_HISTORY_RECORD_COUNT];
};

#endif //EDITOR_H
