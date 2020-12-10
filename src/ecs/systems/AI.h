//
// Created by Jonah on 10/24/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_AI_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_AI_H

#include <entt/entt.hpp>
#include <random>

class AI {
public:
    AI(std::mt19937_64 &seeder);
    void update(entt::registry &registry, double dt);

private:
    void updateRandomWalkingAI(entt::registry &registry, double dt);

    std::mt19937_64 randEngine;
    std::uniform_real_distribution<double> real01Dist;
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_AI_H
