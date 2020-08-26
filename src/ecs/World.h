//
// Created by Jonah on 8/19/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_WORLD_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_WORLD_H

#include <ecs/systems/ChunkManagement.h>
#include <ecs/systems/Physics.h>
#include <ecs/systems/PlayerControl.h>

#include <entt/entt.hpp>
#include <ecs/Components.h>
#include <glad/glad.h>

class World {
public:
    World(const char* vertexPath, const char* fragmentPath);

    void run(double dt, GLFWwindow *window);
    void draw(GLFWwindow *window);

    void updateWindowSize(int width, int height);
    void updateMouse(double xpos, double ypos);
    void updateScroll(double xoffset, double yoffset);
    Shader& getShader();


private:
    entt::registry registry;

    ChunkManagement chunkManagement;

    double pMouseX, pMouseY;
    int screenWidth, screenHeight;

};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_WORLD_H
