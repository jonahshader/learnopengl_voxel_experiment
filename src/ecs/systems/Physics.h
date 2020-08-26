//
// Created by Jonah on 8/11/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_PHYSICS_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_PHYSICS_H

#include <entt/entt.hpp>
#include <other/Constants.h>

class Physics {
public:
    Physics() = delete;
    static void update(entt::registry &registry, double dt);

private:
    static void updatePosition(entt::registry &registry, double dt);
    static void updateVelocity(entt::registry &registry, double dt);

};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_PHYSICS_H
