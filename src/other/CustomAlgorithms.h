//
// Created by Jonah on 9/28/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_CUSTOMALGORITHMS_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_CUSTOMALGORITHMS_H

#include <glm/glm.hpp>
#include <vector>
#include <string>

class CustomAlgorithms {
public:
    static void progressPointOnLine(glm::ivec3 &voxel, glm::vec3 &position, glm::vec3 &direction);
    static std::vector<int> stringToInts(std::string input);
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_CUSTOMALGORITHMS_H
