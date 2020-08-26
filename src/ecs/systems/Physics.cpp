//
// Created by Jonah on 8/11/2020.
//

#include "Physics.h"
#include <ecs/Components.h>


void Physics::update(entt::registry &registry, double dt) {
    updateVelocity(registry, dt);
    updatePosition(registry, dt);
}

void Physics::updatePosition(entt::registry &registry, double dt) {
    // update position from velocity
    registry.view<Components::Position, Components::Velocity>().each([dt](auto &pos, auto &vel) {
        pos.pos += vel.vel * dt;
    });

    // update chunk positions
    registry.view<Components::Position, Components::ChunkPosition>().each([](auto &pos, auto &cPos) {
        int xCNew = floor(pos.pos.x / CHUNK_SIZE);
        int yCNew = floor(pos.pos.y / CHUNK_SIZE);
        int zCNew = floor(pos.pos.z / CHUNK_SIZE);

        // TODO: on chunk pos change, notify chunk entity lists.
        cPos.x = xCNew;
        cPos.y = yCNew;
        cPos.z = zCNew;
    });
}

void Physics::updateVelocity(entt::registry &registry, double dt) {
    registry.view<Components::Velocity, Components::Acceleration>().each([dt](auto &vel, auto &acc) {
        vel.vel += acc.acc * dt;
    });
}
