//
// Created by Jonah on 9/3/2021.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINFUNCTIONS_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINFUNCTIONS_H

#include <functional>
#include <memory>
#include <glm/glm.hpp>
#include <external/fastnoise/FastNoise.h>
#include <other/DataTypes.h>

#ifdef FN_USE_DOUBLES
using FN_VEC3 = glm::dvec3;
#else
using FN_VEC3 = glm::vec3;
#endif

using posTerrain_t = std::function<FN_DECIMAL(FN_VEC3)>;
using coordShift_t = std::function<FN_VEC3(FN_VEC3)>;
//typedef std::function<FN_DECIMAL(FN_VEC3)> posTerrain_t;
using terrainBlockid_t = std::function<blockid(FN_DECIMAL)>;
using posTerrainBlockid_t = std::function<blockid(FN_VEC3)>;

class TerrainFunctions {
};

// posTerrain_t posTerrain_t
posTerrain_t operator+(const posTerrain_t& a, const posTerrain_t& b);

posTerrain_t operator-(const posTerrain_t& a, const posTerrain_t& b);

posTerrain_t operator*(const posTerrain_t& a, const posTerrain_t& b);

posTerrain_t operator/(const posTerrain_t& a, const posTerrain_t& b);

// posTerrain_t and decimal
posTerrain_t operator+(const posTerrain_t& a, const FN_DECIMAL b);

posTerrain_t operator-(const posTerrain_t& a, const FN_DECIMAL b);

posTerrain_t operator*(const posTerrain_t& a, const FN_DECIMAL b);

posTerrain_t operator/(const posTerrain_t& a, const FN_DECIMAL b);

// decimal posTerrain_t
posTerrain_t operator+(const FN_DECIMAL a, const posTerrain_t& b);

posTerrain_t operator-(const FN_DECIMAL a, const posTerrain_t& b);

posTerrain_t operator*(const FN_DECIMAL a, const posTerrain_t& b);

posTerrain_t operator/(const FN_DECIMAL a, const posTerrain_t& b);

posTerrain_t applyCoordShift(const posTerrain_t& terrain, const coordShift_t& coordShift);


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINFUNCTIONS_H
