#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <vector>

#include <ecs/World.h>


void framebuffer_size_callback(GLFWwindow*, int, int);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

double xMouseGlobal;
double yMouseGlobal;
int screenWidthGlobal;
int screenHeightGlobal;
double xScrollGlobal;
double yScrollGlobal;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    glfwWindowHint(GLFW_SAMPLES, 32);

    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), NULL, NULL, &screenWidthGlobal, &screenHeightGlobal);

    GLFWwindow* window = glfwCreateWindow(screenWidthGlobal, screenHeightGlobal, "Voxel Engine Test", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // hide cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // load opengl function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    glEnable(GL_DEBUG_OUTPUT);
//    glEnable(GL_MULTISAMPLE);


    // set texture unit
    glActiveTexture(GL_TEXTURE0);

    // load texture
    int textureWidth, textureHeight, nrChannels;
//    stbi_set_flip_vertically_on_load(true);

    std::vector<unsigned char*> textures;
    //TODO: possible rare bug where textures are not tightly packed. might need to switch to unsigned char** instead of vector
    textures.emplace_back(stbi_load("textures/spritesheet_strip_vertical.jpg", &textureWidth, &textureHeight, &nrChannels, 0));
//    textures.emplace_back(stbi_load("textures/grass_top.jpg", &textureWidth, &textureHeight, &nrChannels, 0));
//    textures.emplace_back(stbi_load("textures/grass_to_dirt_side.jpg", &textureWidth, &textureHeight, &nrChannels, 0));
//    textures.emplace_back(stbi_load("textures/dirt.jpg", &textureWidth, &textureHeight, &nrChannels, 0));
//    textures.emplace_back(stbi_load("textures/stone.jpg", &textureWidth, &textureHeight, &nrChannels, 0));

    // create opengl texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);


//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY, 16);

    if (textures[0]) {
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, 16, 16, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, textures[0]);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    for (auto t : textures) {
        stbi_image_free(t);
    }




    World world("shaders/voxel/voxel_shader.vert", "shaders/voxel/voxel_shader.frag",
                "shaders/voxel_tris/voxel_shader.vert", "shaders/voxel_tris/voxel_shader.frag");

    // connect texture to shader program
    world.getShader().use();
    world.getShader().setInt("sampler", 0);
//    glSampler
    // render global config
    // render loop
    double pTime = glfwGetTime();
    double dt = 1/165.0;
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);
        world.updateWindowSize(screenWidthGlobal, screenHeightGlobal);
        world.updateMouse(xMouseGlobal, yMouseGlobal);
        world.updateScroll(xScrollGlobal, yScrollGlobal);
        double worldRunStartTime = glfwGetTime();
        world.run(dt, window);
//        std::cout << "World run time: " << worldRunStartTime - glfwGetTime() << std::endl;


        glClearColor(world.getSkyColor().r, world.getSkyColor().g, world.getSkyColor().b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double worldRenderStartTime = glfwGetTime();
        world.draw(window);
//        std::cout << "World render time: " << worldRenderStartTime - glfwGetTime() << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
        double time = glfwGetTime();
        dt = time - pTime;
        pTime = time;
        glfwSetWindowTitle(window, ("FPS: " + std::to_string(1/dt)).c_str());
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
    screenWidthGlobal = width;
    screenHeightGlobal = height;
}

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
//    if(glfwGetKey(window, GLFW_KEY_LEFT_SUPER))
//        glfwSetWindowAttrib(window, GLFW_FOCUSED, false);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    xMouseGlobal = xpos;
    yMouseGlobal = ypos;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    xScrollGlobal = xoffset;
    yScrollGlobal = yoffset;
}