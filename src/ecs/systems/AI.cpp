//
// Created by Jonah on 10/24/2020.
//

#include "AI.h"
#include <ecs/Components.h>
#include <other/Constants.h>

AI::AI(std::random_device &randomDevice) :
        randomDevice(randomDevice),
        real01Dist(0.0, 1.0)
{
}


void AI::update(entt::registry &registry, double dt) {
    updateRandomWalkingAI(registry, dt);
}

void AI::updateRandomWalkingAI(entt::registry &registry, double dt) {
    auto view = registry.view<Components::Velocity, Components::AIRandomWalking>();

    for (auto entity : view) {
        auto [vel, ai] = registry.get<Components::Velocity, Components::AIRandomWalking>(entity);

        ai.newDirectionTimer += dt;
        if (ai.newDirectionTimer >= ai.newDirectionPeriod) {
            ai.newDirectionTimer -= ai.newDirectionPeriod;
            double speed = DEFAULT_MAX_SPEED;
            if (registry.has<Components::TravelMaxSpeed>(entity)) {
                speed = registry.get<Components::TravelMaxSpeed>(entity).maxSpeed;
            }

            double dir = real01Dist(randomDevice) * M_PI * 2;
            if (registry.has<Components::TargetVelocity>(entity)) {
                auto &targetVel = registry.get<Components::TargetVelocity>(entity);
                targetVel.targetVel.x = cos(dir) * speed;
                targetVel.targetVel.z = sin(dir) * speed;
            } else {
                vel.vel.x = cos(dir) * speed;
                vel.vel.z = sin(dir) * speed;
            }
        }
    }
}

