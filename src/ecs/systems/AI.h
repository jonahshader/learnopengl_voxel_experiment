//
// Created by Jonah on 10/24/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_AI_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_AI_H

#include <entt/entt.hpp>
#include <random>

class AI {
public:
    AI(std::random_device &randomDevice);
    void update(entt::registry &registry, double dt);

private:
    void updateRandomWalkingAI(entt::registry &registry, double dt);

    std::random_device &randomDevice;
    std::uniform_real_distribution<double> real01Dist;
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_AI_H
