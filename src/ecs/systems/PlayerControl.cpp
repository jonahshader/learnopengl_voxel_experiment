//
// Created by Jonah on 8/11/2020.
//

#include "PlayerControl.h"
#include <ecs/Components.h>
#include <other/Constants.h>
#include <glm/glm.hpp>
#include <iostream>

void PlayerControl::updateMovement(entt::registry &registry, double dt, GLFWwindow *window) {
    double xMove = 0.0, yMove = 0.0, zMove = 0.0, multiplier = 1.0;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) xMove--;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) xMove++;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) zMove--;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) zMove++;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) yMove--;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) yMove++;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) multiplier = 15.0;

    auto view = registry.view<Components::Velocity, Components::DirectionPitchYaw, Components::PlayerControl>();

    for (auto entity : view) {
        auto [vel, dir] = registry.get<Components::Velocity, Components::DirectionPitchYaw>(entity);

        double speed = DEFAULT_MAX_SPEED;
        double jumpSpeed = DEFAULT_JUMP_SPEED;
        if (registry.has<Components::TravelMaxSpeed>(entity)) {
            speed = registry.get<Components::TravelMaxSpeed>(entity).maxSpeed;
        }
        if (registry.has<Components::JumpVelocity>(entity)) {
            jumpSpeed = registry.get<Components::JumpVelocity>(entity).jumpUpVelocity;
        }

        speed *= multiplier;

        if (registry.has<Components::TargetVelocity>(entity)) {
            auto &targetVel = registry.get<Components::TargetVelocity>(entity);

            if (xMove != 0.0 || zMove != 0.0) {
                double localWalkYaw = atan2(-zMove, xMove);
                double combinedMoveYaw = localWalkYaw + dir.yaw;
                targetVel.targetVel.x = cos(combinedMoveYaw) * speed;
                targetVel.targetVel.z = sin(combinedMoveYaw) * speed;
            } else {
                targetVel.targetVel.x = 0;
                targetVel.targetVel.z = 0;
            }

//            if (registry.has<Components::ChunkCollision>(entity)) {
//                auto &chunkCollision = registry.get<Components::ChunkCollision>(entity);
//                if (chunkCollision.grounded) {
//                    targetVel.targetVel.y = std::max(yMove * speed, 0.0);
//                }
//            } else {
//                targetVel.targetVel.y = std::max(yMove * speed, 0.0);
//            }
            targetVel.targetVel.y = std::max(yMove * jumpSpeed, 0.0);

        } else {

            if (xMove != 0.0 || zMove != 0.0) {
                double localWalkYaw = atan2(-zMove, xMove);
                double combinedMoveYaw = localWalkYaw + dir.yaw;
                vel.vel.x = cos(combinedMoveYaw) * speed;
                vel.vel.z = sin(combinedMoveYaw) * speed;
            } else {
                vel.vel.x = 0;
                vel.vel.z = 0;
            }
            vel.vel.y = yMove * jumpSpeed;
        }
    }

}

void PlayerControl::updateLook(entt::registry &registry, double dPitch, double dYaw) {
    auto view = registry.view<Components::DirectionPitchYaw, Components::PlayerControl>();

    for (auto entity : view) {
        auto &dir = registry.get<Components::DirectionPitchYaw>(entity);
        dir.yaw += dYaw * DEFAULT_LOOK_SENS;
        dir.pitch += dPitch * DEFAULT_LOOK_SENS;

        // constrain pitch to looking fully up and fully down
        if (dir.pitch > M_PI / 2) dir.pitch = M_PI / 2;
        else if (dir.pitch < -M_PI / 2) dir.pitch = -M_PI / 2;
        // wrap yaw to [0, 2pi)
        dir.yaw -= floor(dir.yaw / (M_PI * 2)) * M_PI * 2;

//        std::cout << "Incremented pitch by " << dPitch << " and yaw by " << dYaw << std::endl;
    }
}
