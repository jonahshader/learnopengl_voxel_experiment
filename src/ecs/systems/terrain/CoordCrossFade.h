//
// Created by Jonah on 9/6/2021.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_COORDCROSSFADE_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_COORDCROSSFADE_H

#include <memory>
#include "TerrainFunctions.h"

class CoordCrossFade {
private:
    std::shared_ptr<coordShift_t> a;
    std::shared_ptr<coordShift_t> b;
    std::shared_ptr<posTerrain_t> cross;

public:
    CoordCrossFade(std::shared_ptr<coordShift_t> a,
                   std::shared_ptr<coordShift_t> b,
                   std::shared_ptr<posTerrain_t> cross) : a(a), b(b), cross(cross) { }

    FN_VEC3 operator() (FN_VEC3 inputPos) {
        auto crossEvaluated = cross(inputPos);
        return (a(inputPos) * (1-crossEvaluated)) + (b(inputPos) * crossEvaluated);
    }


};

#endif //LEARNOPENGL_VOXEL_EXPERIMENT_COORDCROSSFADE_H
