//
// Created by Jonah on 8/19/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_WORLD_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_WORLD_H

#include <ecs/systems/terrain/ChunkManagement.h>
#include <ecs/systems/AI.h>
#include <ecs/systems/Physics.h>
#include <ecs/systems/PlayerControl.h>
#include <ecs/systems/Graphics.h>

#include <entt/entt.hpp>
#include <ecs/Components.h>
#include <graphics/TextureManager.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <random>

class World {
public:
    World(const char* vertexPathInst, const char* fragmentPathInst,
          const char* vertexPathTri, const char* fragmentPathTri);

    void run(double dt, GLFWwindow *window);
    void draw(GLFWwindow *window);

    void updateWindowSize(int width, int height);
    void updateMouse(double xpos, double ypos);
    void updateScroll(double xoffset, double yoffset);

    const glm::vec3 &getSkyColor() const;

private:
    std::mt19937_64 seeder;
    entt::registry registry;

    ChunkManagement chunkManagement;
    AI ai;
    Graphics graphics;
    TextureManager textureManager;


    double pMouseX, pMouseY;
    bool firstMouse;
    int screenWidth, screenHeight;
    glm::vec3 skyColor;


};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_WORLD_H
