//
// Created by Jonah on 9/3/2021.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_NOISEPOSTERRAIN_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_NOISEPOSTERRAIN_H

#include <external/fastnoise/FastNoise.h>
#include "TerrainFunctions.h"

/*
class increment3 {
private:
    vec3 inc;
public:
    increment3(vec3 inc) : inc(inc) { }

    vec3 operator() (vec3 input) const {
        input.x += inc.x;
        input.y += inc.y;
        input.z += inc.z;
        return input;
    }
};
 */

//class NoisePosTerrain {
//private:
//    FastNoise noise;
//
//public:
//    NoisePosTerrain(FastNoise&& noise) : noise(noise) { }
//
//    FN_DECIMAL operator() (FN_VEC3 inputPos) {
//        return noise.GetNoise(inputPos.x, inputPos.y, inputPos.z);
//    }
//
//    const FastNoise &getNoise() const {
//        return noise;
//    }
//};

class NoisePosTerrain : public FastNoise {
public:
    FN_DECIMAL operator() (FN_VEC3 inputPos) const {
        return GetNoise(inputPos.x, inputPos.y, inputPos.z);
    }
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_NOISEPOSTERRAIN_H
