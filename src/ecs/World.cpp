//
// Created by Jonah on 8/19/2020.
//

#include "World.h"

#include <iostream>
//#include <ctime>

World::World(const char* vertexPathInst, const char* fragmentPathInst,
             const char* vertexPathTri, const char* fragmentPathTri) :
registry(),
chunkManagement(vertexPathInst, fragmentPathInst, vertexPathTri, fragmentPathTri, time(NULL)),
pMouseX(0.0),
pMouseY(0.0),
firstMouse(true),
screenWidth(800),
screenHeight(600),
skyColor(0.5f, 0.45f, 0.85f)
{
    // create player
    entt::entity player = registry.create();
    registry.emplace<Components::Position>(player, glm::dvec3(0.5, 0.5, 0.5));
    registry.emplace<Components::Velocity>(player, glm::dvec3(0.0));
    registry.emplace<Components::Acceleration>(player, glm::dvec3(0.0));
    registry.emplace<Components::CylinderCollider>(player, 1.9, 0.4, glm::dvec3(0.5, 0.5, 0.5));
    registry.emplace<Components::ChunkCollision>(player, false);
    registry.emplace<Components::ChunkPosition>(player, 0, 0, 0);
    registry.emplace<Components::DirectionPitchYaw>(player, M_PI / 2, 0.0);
    registry.emplace<Components::PlayerControl>(player);
    registry.emplace<Components::TravelMaxSpeed>(player, 5.612);
    registry.emplace<Components::JumpVelocity>(player, 7.0);
    registry.emplace<Components::CameraAttach>(player, 90.0, glm::dvec3(0.0, 1.75, 0.0));
    registry.emplace<Components::Gravity>(player, -20.0);
    registry.emplace<Components::TargetVelocity>(player, glm::dvec3(0.0));
}

void World::run(double dt, GLFWwindow *window) {
    PlayerControl::updateMovement(registry, dt, window);
    Physics::update(registry, chunkManagement, dt);
    chunkManagement.run(registry);

//    registry.view<Components::Position, Components::ChunkPosition, Components::DirectionPitchYaw>().each([](auto &pos, auto &chunkPos, auto &dir) {
//        std::cout << "Position: " << Components::dvecToString(pos.pos)
//        << "\nChunk Position: x: " << chunkPos.x << " y: " << chunkPos.y << " z: " << chunkPos.z
//        << "\nDirection: pitch: " << dir.pitch << " yaw: " << dir.yaw << std::endl;
//    });

}

void World::draw(GLFWwindow *window) {
    chunkManagement.render(registry, screenWidth, screenHeight, skyColor);
}

Shader &World::getShader() {
    return chunkManagement.getShader();
}

void World::updateWindowSize(int width, int height) {
    if (!(width == 0 || height == 0)) {
        screenWidth = width;
        screenHeight = height;
    }
}

void World::updateMouse(double xpos, double ypos) {
    if (firstMouse) {
        firstMouse = false;
    } else {
        PlayerControl::updateLook(registry, -ypos + pMouseY, xpos - pMouseX);
    }
    pMouseX = xpos;
    pMouseY = ypos;
}

void World::updateScroll(double xoffset, double yoffset) {

}

const glm::vec3 &World::getSkyColor() const {
    return skyColor;
}
