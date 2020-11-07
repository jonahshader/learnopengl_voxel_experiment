//
// Created by Jonah on 10/26/2020.
//

#include "Graphics.h"
#include <vector>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <entt/entt.hpp>
#include "ecs/Components.h"

Graphics::Graphics(entt::registry &registry, std::random_device &rd) :
rd(rd)
{
    setupBillboardYLock();
}

void Graphics::update(entt::registry &registry) {
    updateBillboardsYLock(registry);
}

void Graphics::setupBillboardYLock() {
    glGenVertexArrays(1, &vaoBBYLock);
    glGenBuffers(1, &vboBBYLock);
    glGenBuffers(1, &vboBBPosYLock);
    glGenBuffers(1, &vboBBTexYLock);

    // VAO stuff
    glBindVertexArray(vaoBBYLock);
    glBindBuffer(GL_ARRAY_BUFFER, vboBBYLock);
    // x y z
    glVertexAttribPointer(0, 1, GL_FLOAT_VEC3, false, 5 * sizeof(float), (void*)0);
    // tex: x y
    glVertexAttribPointer(1, 1, GL_FLOAT_VEC2, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // offset
    glBindBuffer(GL_ARRAY_BUFFER, vboBBPosYLock);
    glVertexAttribPointer(2, 1, GL_FLOAT_VEC3, false, 3 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);
    // texture
    glBindBuffer(GL_ARRAY_BUFFER, vboBBTexYLock);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, 1 * sizeof(unsigned char), (void*)0);
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
}

std::vector<float> Graphics::generateBillboardQuad(float pitch, float yaw, float yOffset) {
    // 4 3
    // 1 2

    // 123, 134

    glm::vec4 p1(-0.5f, -0.5f + yOffset, 0.0f, 1.0f);
    glm::vec4 p2(0.5f, -0.5f + yOffset, 0.0f, 1.0f);
    glm::vec4 p3(0.5f, 0.5f + yOffset, 0.0f, 1.0f);
    glm::vec4 p4(-0.5f, 0.5f + yOffset, 0.0f, 1.0f);

    glm::mat4 rotation;
    glm::rotate(rotation, pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
    glm::rotate(rotation, yaw, glm::vec3(0.0f, 1.0f, 0.0f));

    p1 = rotation * p1;
    p2 = rotation * p2;
    p3 = rotation * p3;
    p4 = rotation * p4;

    std::vector<float> quad;
    quad.emplace_back(p1.x);
    quad.emplace_back(p1.y);
    quad.emplace_back(p1.z);

    quad.emplace_back(p2.x);
    quad.emplace_back(p2.y);
    quad.emplace_back(p2.z);

    quad.emplace_back(p3.x);
    quad.emplace_back(p3.y);
    quad.emplace_back(p3.z);

    quad.emplace_back(p1.x);
    quad.emplace_back(p1.y);
    quad.emplace_back(p1.z);

    quad.emplace_back(p3.x);
    quad.emplace_back(p3.y);
    quad.emplace_back(p3.z);

    quad.emplace_back(p4.x);
    quad.emplace_back(p4.y);
    quad.emplace_back(p4.z);

    return quad;
}

void Graphics::updateBillboardsYLock(entt::registry &registry) {
    // just want POV
    auto povView = registry.view<Components::CameraAttach, Components::DirectionPitchYaw>();

    entt::entity pov;
    bool povFound = false;
    for (entt::entity e : povView) {
        pov = e;
        povFound = true;
    }
    std::vector<float> quadPosOnly;
    if (povFound) {
        auto &dir = registry.get<Components::DirectionPitchYaw>(pov);
        // only change yaw for yLock. move up so pivot is on the bottom
        quadPosOnly = generateBillboardQuad(0.0f, dir.yaw, 0.5f);

        // generate full quad for buffering
        std::vector<float> fullQuad;

        for (int i = 0; i < 3; ++i) fullQuad.emplace_back(quadPosOnly[i]);
        fullQuad.emplace_back(0.0f); // tx
        fullQuad.emplace_back(0.0f); // ty
        for (int i = 0; i < 3; ++i) fullQuad.emplace_back(quadPosOnly[i + 3]);
        fullQuad.emplace_back(1.0f); // tx
        fullQuad.emplace_back(0.0f); // ty
        for (int i = 0; i < 3; ++i) fullQuad.emplace_back(quadPosOnly[i + 6]);
        fullQuad.emplace_back(1.0f); // tx
        fullQuad.emplace_back(1.0f); // ty
        for (int i = 0; i < 3; ++i) fullQuad.emplace_back(quadPosOnly[i]);
        fullQuad.emplace_back(0.0f); // tx
        fullQuad.emplace_back(0.0f); // ty
        for (int i = 0; i < 3; ++i) fullQuad.emplace_back(quadPosOnly[i + 6]);
        fullQuad.emplace_back(1.0f); // tx
        fullQuad.emplace_back(1.0f); // ty
        for (int i = 0; i < 3; ++i) fullQuad.emplace_back(quadPosOnly[i + 9]);
        fullQuad.emplace_back(0.0f); // tx
        fullQuad.emplace_back(1.0f); // ty

        // buffer full quad
        glBindBuffer(GL_ARRAY_BUFFER, vboBBYLock);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fullQuad.data()), fullQuad.data(), GL_STATIC_DRAW);
    }

    // build offset, tex buffers
    std::vector<float> offsets;
    std::vector<unsigned char> textures;

    auto bbView = registry.view<Components::GraphicBillboard, Components::Position>();
    for (const auto entity: bbView) {
        auto [bb, pos] = bbView.get<Components::GraphicBillboard, Components::Position>(entity);
        if (bb.yLocked) {
            offsets.emplace_back(pos.pos.x);
            offsets.emplace_back(pos.pos.y);
            offsets.emplace_back(pos.pos.z);
            textures.emplace_back(bb.texture);
        }
    }

    instancesBBYLock = textures.size();

    glBindBuffer(GL_ARRAY_BUFFER, vboBBPosYLock);
    glBufferData(GL_ARRAY_BUFFER, sizeof(offsets.data()), offsets.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboBBTexYLock);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textures.data()), textures.data(), GL_DYNAMIC_DRAW);

}

void Graphics::render(entt::registry &registry) {

}
