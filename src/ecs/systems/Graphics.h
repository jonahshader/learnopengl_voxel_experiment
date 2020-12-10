//
// Created by Jonah on 10/26/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_GRAPHICS_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_GRAPHICS_H

#include <random>
#include <vector>
#include <entt/entt.hpp>
#include "ecs/Components.h"
#include "graphics/TextureManager.h"
#include "graphics/Shader.h"
#include "glm/glm.hpp"

class Graphics {
public:
    // TODO: destructor. disconnect listeners. delete vbo vao and other buffers
    Graphics(entt::registry &registry);
    void update(entt::registry &registry);
    void render(entt::registry &registry, TextureManager &tm, int screenWidth, int screenHeight, float fogDistance, const glm::vec3 &skyColor);

private:
    std::vector<float> generateBillboardQuad(float pitch, float yaw, float yOffset);
    void updateBillboardsYLock(entt::registry &registry);


    void setupBillboardYLock(); // vbo, vao setup
    Shader bbYLockShader;
    unsigned int vaoBBYLock;
    unsigned int vboBBYLock; // Bb = Billboard
    unsigned int vboBBTexPosScaleYLock;
    unsigned int vboBBOffsetYLock;
    unsigned int instancesBBYLock;
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_GRAPHICS_H
