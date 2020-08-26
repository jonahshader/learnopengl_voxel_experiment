//
// Created by Jonah on 8/5/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_SHADER_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shader {
public:
    unsigned int ID; // opengl program id

    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void use() const;
    // utility uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMatrix4(const std::string &name, glm::mat4 value) const;
    void setVec3i(const std::string &name, int x, int y, int z) const;
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_SHADER_H
