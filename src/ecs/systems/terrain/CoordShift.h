//
// Created by Jonah on 9/6/2021.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_COORDSHIFT_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_COORDSHIFT_H

#include <functional>
#include "TerrainFunctions.h"


class CoordShift {
private:
    std::shared_ptr<posTerrain_t> xShift;
    std::shared_ptr<posTerrain_t> yShift;
    std::shared_ptr<posTerrain_t> zShift;

public:
    CoordShift(std::shared_ptr<posTerrain_t> xShift,
               std::shared_ptr<posTerrain_t> yShift,
               std::shared_ptr<posTerrain_t> zShift) :
            xShift(xShift), yShift(yShift), zShift(zShift) { }

    FN_VEC3 operator() (FN_VEC3 inputPos) {
        return inputPos + FN_VEC3 { xShift(inputPos), yShift(inputPos), zShift(inputPos) };
    }

};

#endif //LEARNOPENGL_VOXEL_EXPERIMENT_COORDSHIFT_H
