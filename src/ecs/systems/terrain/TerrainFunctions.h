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
using FN_VEC3 = glm::dvec3;
#else
using FN_VEC3 = glm::vec3;
#endif

using posTerrain_t = std::function<FN_DECIMAL(FN_VEC3)>;
using coordShift_t = std::function<FN_VEC3(FN_VEC3)>;
//typedef std::function<FN_DECIMAL(FN_VEC3)> posTerrain_t;

class TerrainFunctions {
public:
    static posTerrain_t noiseToPosTerrain(FastNoise&& noise);
    static posTerrain_t noiseToPosTerrain(FastNoise noise);


private:

};

// posTerrain_t posTerrain_t
posTerrain_t operator+(const posTerrain_t& a, const posTerrain_t& b) {
    return [&](FN_VEC3 pos) {
        return a(pos) + b(pos);
    };
}

posTerrain_t operator-(const posTerrain_t& a, const posTerrain_t& b) {
    return [&](FN_VEC3 pos) {
        return a(pos) - b(pos);
    };
}

posTerrain_t operator*(const posTerrain_t& a, const posTerrain_t& b) {
    return [&](FN_VEC3 pos) {
        return a(pos) * b(pos);
    };
}

posTerrain_t operator/(const posTerrain_t& a, const posTerrain_t& b) {
    return [&](FN_VEC3 pos) {
        return a(pos) / b(pos);
    };
}

// posTerrain_t and decimal
posTerrain_t operator+(const posTerrain_t& a, const FN_DECIMAL b) {
    return [&](FN_VEC3 pos) {
        return a(pos) + b;
    };
}

posTerrain_t operator-(const posTerrain_t& a, const FN_DECIMAL b) {
    return [&](FN_VEC3 pos) {
        return a(pos) - b;
    };
}

posTerrain_t operator*(const posTerrain_t& a, const FN_DECIMAL b) {
    return [&](FN_VEC3 pos) {
        return a(pos) * b;
    };
}

posTerrain_t operator/(const posTerrain_t& a, const FN_DECIMAL b) {
    return [&](FN_VEC3 pos) {
        return a(pos) / b;
    };
}

// decimal posTerrain_t
posTerrain_t operator+(const FN_DECIMAL a, const posTerrain_t& b) {
    return [&](FN_VEC3 pos) {
        return a + b(pos);
    };
}

posTerrain_t operator-(const FN_DECIMAL a, const posTerrain_t& b) {
    return [&](FN_VEC3 pos) {
        return a - b(pos);
    };
}

posTerrain_t operator*(const FN_DECIMAL a, const posTerrain_t& b) {
    return [&](FN_VEC3 pos) {
        return a * b(pos);
    };
}

posTerrain_t operator/(const FN_DECIMAL a, const posTerrain_t& b) {
    return [&](FN_VEC3 pos) {
        return a / b(pos);
    };
}






#endif //LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINFUNCTIONS_H
