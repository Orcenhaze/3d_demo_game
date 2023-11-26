#ifndef COLLISION_H
#define COLLISION_H

struct Collision_Data
{
    // Penetration depth.
    f32 penetration;
    
    // Resolution normal.
    Vector3 normal;
    
    // Closest point (valid when calling closest_X function-family). 
    Vector3 point;
    
    // Collision result.
    b32 result;
};

#endif //COLLISION_H
