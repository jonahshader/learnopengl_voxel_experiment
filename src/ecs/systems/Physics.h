//
// Created by Jonah on 8/11/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_PHYSICS_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_PHYSICS_H

#include <entt/entt.hpp>
#include <other/Constants.h>
#include <ecs/systems/terrain/ChunkManagement.h>

class Physics {
public:
    Physics() = delete;
    static void update(entt::registry &registry, ChunkManagement &chunkManagement, double dt);

private:
    static void updatePosition(entt::registry &registry, ChunkManagement &chunkManagement, double dt);
    static void updateVelocity(entt::registry &registry, double dt);
    static void updateAccelFromVelocityTarget(entt::registry &registry, double dt);
    static bool checkRectangularCollision(entt::registry &registry, ChunkManagement &chunkManagement, glm::dvec3 &pos, Components::BoxCollider &collider);
    static bool checkPointCollision(entt::registry &registry, ChunkManagement &chunkManagement, glm::dvec3 &pos);

    static void runPointCollision(entt::registry &registry, ChunkManagement &chunkManagement, double dt);
    static void runBoxCollision(entt::registry &registry, ChunkManagement &chunkManagement);

    // runYTeleport makes the entity teleport up if a grounded collision can be resolved by moving up by a certain amount
    static void runYTeleport(entt::registry &registry, ChunkManagement &chunkManagement);
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_PHYSICS_H
