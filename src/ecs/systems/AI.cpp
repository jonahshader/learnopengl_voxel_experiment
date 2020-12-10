//
// Created by Jonah on 10/24/2020.
//

#include "AI.h"
#include <ecs/Components.h>
#include <other/Constants.h>

AI::AI(std::mt19937_64 &seeder) :
        randEngine(seeder()),
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
            ai.newDirectionTimer -= (ai.newDirectionPeriod + ai.newDirectionVariance * (-0.5 + real01Dist(randEngine)) * 2);
            double speed = DEFAULT_MAX_SPEED;
            if (registry.has<Components::TravelMaxSpeed>(entity)) {
                speed = registry.get<Components::TravelMaxSpeed>(entity).maxSpeed;
            }

            double dir = real01Dist(randEngine) * M_PI * 2;
            if (registry.has<Components::TargetVelocity>(entity)) {
                auto &targetVel = registry.get<Components::TargetVelocity>(entity);
                targetVel.targetVel.x = cos(dir) * speed;
                targetVel.targetVel.z = sin(dir) * speed;
                if (registry.has<Components::JumpVelocity>(entity)) {
                    targetVel.targetVel.y = registry.get<Components::JumpVelocity>(entity).jumpUpVelocity;
                } else {
                    targetVel.targetVel.y = 7.0;
                }

            } else {
                vel.vel.x = cos(dir) * speed;
                vel.vel.z = sin(dir) * speed;
            }
        }
    }
}

