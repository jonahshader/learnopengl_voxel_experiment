//
// Created by Jonah on 9/6/2021.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINCROSSFADE_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINCROSSFADE_H

#include <memory>
#include "TerrainFunctions.h"

class TerrainCrossFade {
private:
    std::shared_ptr<posTerrain_t> a;
    std::shared_ptr<posTerrain_t> b;
    std::shared_ptr<posTerrain_t> cross;

public:
    TerrainCrossFade(std::shared_ptr<posTerrain_t> a, std::shared_ptr<posTerrain_t> b,
                     std::shared_ptr<posTerrain_t> cross) : a(a), b(b), cross(cross) {}

    FN_DECIMAL operator() (FN_VEC3 inputPos) {
        auto crossEvaluated = cross(inputPos);
        return (a(inputPos) * (1-crossEvaluated)) + (b(inputPos) * crossEvaluated);
    }
};

#endif //LEARNOPENGL_VOXEL_EXPERIMENT_TERRAINCROSSFADE_H
