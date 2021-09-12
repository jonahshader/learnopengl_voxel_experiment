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

Graphics::Graphics(entt::registry &registry) :
bbYLockShader("shaders/billboard/billboard.vert", "shaders/billboard/billboard.frag")
{
    setupBillboardYLock();
}

void Graphics::update(entt::registry &registry) {
    updateBillboardsYLock(registry);
}

void Graphics::setupBillboardYLock() {
    glGenVertexArrays(1, &vaoBBYLock);
    glGenBuffers(1, &vboBBYLock);
    glGenBuffers(1, &vboBBOffsetYLock);
    glGenBuffers(1, &vboBBTexPosScaleYLock);

    glBindBuffer(GL_ARRAY_BUFFER, vboBBYLock);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboBBOffsetYLock);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboBBTexPosScaleYLock);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VAO stuff
    glBindVertexArray(vaoBBYLock);
    glBindBuffer(GL_ARRAY_BUFFER, vboBBYLock);
    // x y z
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
    // tex: x y
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // offset
    glBindBuffer(GL_ARRAY_BUFFER, vboBBOffsetYLock);
    glVertexAttribPointer(2, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);

    // texture per instance
    // tex offset
    glBindBuffer(GL_ARRAY_BUFFER, vboBBTexPosScaleYLock);
    glVertexAttribPointer(3, 2, GL_FLOAT, false, 6 * sizeof(float), (void*)0);
    glVertexAttribDivisor(3, 1);

    // tex size
    glVertexAttribPointer(4, 2, GL_FLOAT, false, 6 * sizeof(float), (void*)(sizeof(float) * 2));
    glVertexAttribDivisor(4, 1);

    // bb size
    glVertexAttribPointer(5, 2, GL_FLOAT, false, 6 * sizeof(float), (void*)(sizeof(float) * 4));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

std::vector<float> Graphics::generateBillboardQuad(float pitch, float yaw, float yOffset) {
    // 4 3
    // 1 2

    // 123, 134

    glm::vec4 p1(-0.5f, -0.5f + yOffset, 0.0f, 1.0f);
    glm::vec4 p2(0.5f, -0.5f + yOffset, 0.0f, 1.0f);
    glm::vec4 p3(0.5f, 0.5f + yOffset, 0.0f, 1.0f);
    glm::vec4 p4(-0.5f, 0.5f + yOffset, 0.0f, 1.0f);

    glm::mat4 rotation(1.0f);
    rotation = glm::rotate(rotation, -pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
    rotation =glm::rotate(rotation, -yaw, glm::vec3(0.0f, 1.0f, 0.0f));

//    std::cout << "matrix: " << std::endl;
//    for (int j = 0; j < 4; ++j) {
//        for (int i = 0; i < 4; ++i) {
//            std::cout << rotation[j][i] << " ";
//        }
//        std::cout << std::endl;
//    }


    p1 = rotation * p1;
    p2 = rotation * p2;
    p3 = rotation * p3;
    p4 = rotation * p4;

    p1 = p1 / p1.w;
    p2 = p2 / p2.w;
    p3 = p3 / p3.w;
    p4 = p4 / p4.w;

//    std::cout << "x1: " << p1.x << std::endl;

    std::vector<float> quad;
    quad.emplace_back(p1.x);
    quad.emplace_back(p1.y);
    quad.emplace_back(p1.z);
    quad.emplace_back(0.0f); // tx
    quad.emplace_back(0.0f); // ty

    quad.emplace_back(p2.x);
    quad.emplace_back(p2.y);
    quad.emplace_back(p2.z);
    quad.emplace_back(1.0f);
    quad.emplace_back(0.0f);

    quad.emplace_back(p3.x);
    quad.emplace_back(p3.y);
    quad.emplace_back(p3.z);
    quad.emplace_back(1.0f);
    quad.emplace_back(1.0f);

    quad.emplace_back(p1.x);
    quad.emplace_back(p1.y);
    quad.emplace_back(p1.z);
    quad.emplace_back(0.0f);
    quad.emplace_back(0.0f);

    quad.emplace_back(p3.x);
    quad.emplace_back(p3.y);
    quad.emplace_back(p3.z);
    quad.emplace_back(1.0f);
    quad.emplace_back(1.0f);

    quad.emplace_back(p4.x);
    quad.emplace_back(p4.y);
    quad.emplace_back(p4.z);
    quad.emplace_back(0.0f);
    quad.emplace_back(1.0f);

//    for (auto i : quad) {
//        std::cout << i << " " << std::endl;
//    }

//    std::cout << "Quad floats: " << quad.size() << std::endl;

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
    std::vector<float> quadWithTex;
    if (povFound) {
        auto &dir = registry.get<Components::DirectionPitchYaw>(pov);
        // only change yaw for yLock. move up so pivot is on the bottom
        quadWithTex = generateBillboardQuad(0.0f, dir.yaw, 0.5f);

        // buffer full quad
        glBindBuffer(GL_ARRAY_BUFFER, vboBBYLock);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * quadWithTex.size(), quadWithTex.data(), GL_STATIC_DRAW);
    }

    auto bbView = registry.view<Components::GraphicBillboard, Components::Position>();

    // build offset, tex buffers
    std::vector<float> offsets;
    std::vector<float> texPosSizeSpriteDims;


    for (const auto entity: bbView) {
        auto [bb, pos] = bbView.get<Components::GraphicBillboard, Components::Position>(entity);
        if (bb.yLocked) {
            offsets.emplace_back(pos.pos.x);
            offsets.emplace_back(pos.pos.y);
            offsets.emplace_back(pos.pos.z);
            texPosSizeSpriteDims.emplace_back(bb.sx);
            texPosSizeSpriteDims.emplace_back(bb.sy);
            texPosSizeSpriteDims.emplace_back(bb.sw);
            texPosSizeSpriteDims.emplace_back(bb.sh);
            texPosSizeSpriteDims.emplace_back(bb.width);
            texPosSizeSpriteDims.emplace_back(bb.height);
        }
    }

    instancesBBYLock = offsets.size() / 3;

    glBindBuffer(GL_ARRAY_BUFFER, vboBBOffsetYLock);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * offsets.size(), offsets.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboBBTexPosScaleYLock);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * texPosSizeSpriteDims.size(), texPosSizeSpriteDims.data(), GL_STATIC_DRAW);

}

void Graphics::render(entt::registry &registry, TextureManager &tm, int screenWidth, int screenHeight, float fogDistance, const glm::vec3 &skyColor) {
    auto playerView = registry.view<Components::CameraAttach, Components::Position, Components::ChunkPosition, Components::DirectionPitchYaw>();

    entt::entity player;
    bool playerFound = false;
    for (entt::entity e : playerView) {
        player = e;
        playerFound = true;
    }

    if (playerFound) {
        // set texture unit
//        glActiveTexture(GL_TEXTURE0 + tm.getTextureInfoA("grass_top").textureUnit);
        auto[playerCam, playerPos, playerChunkPos, playerDir] = registry.get<Components::CameraAttach, Components::Position, Components::ChunkPosition, Components::DirectionPitchYaw>(
                player);

        glm::mat4 projection = glm::perspective(glm::radians((float) playerCam.fov), screenWidth / (float) screenHeight,
                                                0.1f, 600.0f);

//        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3((float) -(playerPos.pos.x - playerChunkPos.x * CHUNK_SIZE), (float) -(playerPos.pos.y - playerChunkPos.y * CHUNK_SIZE), (float) -(playerPos.pos.z - playerChunkPos.z * CHUNK_SIZE)));
        glm::mat4 view = glm::rotate(projection, (float) playerDir.pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, (float) playerDir.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(
                (float) -(playerPos.pos.x + playerCam.posOffset.x),
                (float) -(playerPos.pos.y + playerCam.posOffset.y),
                (float) -(playerPos.pos.z + playerCam.posOffset.z)));


        bbYLockShader.use();
        bbYLockShader.setFloat("fogDistance", fogDistance);
        bbYLockShader.setVec3("skyColor", skyColor.r, skyColor.g, skyColor.b);
        bbYLockShader.setUInt("chunkSize", CHUNK_SIZE);
        bbYLockShader.setMatrix4("viewProjection", view);
        bbYLockShader.setInt("sampler", 1);
        bbYLockShader.setVec3("camPos",
                                playerPos.pos.x + playerCam.posOffset.x,
                                playerPos.pos.y + playerCam.posOffset.y,
                                playerPos.pos.z + playerCam.posOffset.z);

        glBindVertexArray(vaoBBYLock);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instancesBBYLock);
//        std::cout << "Billboards rendered: " << instancesBBYLock << std::endl;
        glBindVertexArray(0);
    }
}
