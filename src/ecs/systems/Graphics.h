//
// Created by Jonah on 10/26/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_GRAPHICS_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_GRAPHICS_H

#include <random>
#include <vector>
#include <entt/entt.hpp>
#include "ecs/Components.h"

class Graphics {
public:
    // TODO: destructor. disconnect listeners. delete vbo vao and other buffers
    Graphics(entt::registry &registry, std::random_device &rd);
    void update(entt::registry &registry);
    void render(entt::registry &registry);

private:
    std::vector<float> generateBillboardQuad(float pitch, float yaw, float yOffset);
    void updateBillboardsYLock(entt::registry &registry);
    std::random_device &rd;


    void setupBillboardYLock(); // vbo, vao setup
    unsigned int vboBBYLock; // Bb = Billboard
    unsigned int vaoBBYLock;
    unsigned int vboBBTexYLock;
    unsigned int vboBBPosYLock;
    unsigned int instancesBBYLock;
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_GRAPHICS_H
