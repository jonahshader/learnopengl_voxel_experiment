//
// Created by Jonah on 8/11/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_PLAYERCONTROL_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_PLAYERCONTROL_H

#include <entt/entt.hpp>
#include <GLFW/glfw3.h>
#include <cmath>

class PlayerControl {
public:
    PlayerControl() = delete;
    static void updateMovement(entt::registry &registry, double dt, GLFWwindow* window);
    static void updateLook(entt::registry &registry, double dPitch, double dYaw);

private:
    static constexpr double DEFAULT_LOOK_SENS = 0.005/M_PI;
};



#endif //LEARNOPENGL_VOXEL_EXPERIMENT_PLAYERCONTROL_H
