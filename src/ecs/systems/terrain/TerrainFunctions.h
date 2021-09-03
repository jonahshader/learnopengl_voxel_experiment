//
// Created by Jonah on 9/3/2021.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINFUNCTIONS_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINFUNCTIONS_H

#include <functional>
#include <memory>
#include <glm/glm.hpp>
#include <external/fastnoise/FastNoise.h>

#ifdef FN_USE_DOUBLES
typedef glm::dvec3 FN_VEC3;
#else
typedef glm::vec3 FN_VEC3;
#endif

using posTerrain_t = std::function<FN_DECIMAL(FN_VEC3)>;
//typedef std::function<FN_DECIMAL(FN_VEC3)> posTerrain_t;

class TerrainFunctions {
public:
    static posTerrain_t noiseToPosTerrain(FastNoise&& noise);
    static posTerrain_t noiseToPosTerrain(FastNoise noise);
private:

};




#endif //LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINFUNCTIONS_H
