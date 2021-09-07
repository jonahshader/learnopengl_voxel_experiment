//
// Created by Jonah on 9/6/2021.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_MAINTERRAINFUNCTION_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_MAINTERRAINFUNCTION_H

#include <random>
#include <cmath>
#include "TerrainFunctions.h"
#include "other/DataTypes.h"
#include "external/fastnoise/FastNoise.h"
#include "NoisePosTerrain.h"
#include "CoordShift.h"

class MainTerrainFunction {
private:
    std::mt19937_64 mt;
    posTerrainBlockid_t mainFun;

public:
    MainTerrainFunction(long seed) : mt(seed) {
        auto mainNoise = NoisePosTerrain();
        mainNoise.SetNoiseType(FastNoise::SimplexFractal);
        mainNoise.SetFrequency(0.0010);
        mainNoise.SetFractalOctaves(7); // 7
        mainNoise.SetSeed(mt());

        coordShift_t twister = CoordShift(0.002, 150.0, 7, mt);

        posTerrain_t mainTerrainFun = applyCoordShift(mainNoise + [](FN_VEC3 inputPos) {
//            if (inputPos.x * inputPos.x + inputPos.z * inputPos.z < 32 * 32 * (std::sin(inputPos.y * 0.5f) * .4f + .6f)) {
//                return 100.0f;
//            } else {
//                return (inputPos.y * 0.015f);
//                return 0.0f;
//            }
            return (inputPos.y * 0.015f);

//            return (inputPos.x * inputPos.x) + inputPos.y * .01f;

        }, twister);



        mainFun = [=](FN_VEC3 inputPos) {
            if (mainTerrainFun (inputPos) < 0) {
                return 1;
            } else {
                return 0;
            }
        };
    }

    posTerrainBlockid_t &getMainFun() {
        return mainFun;
    }
};



#endif //LEARNOPENGL_VOXEL_EXPERIMENT_MAINTERRAINFUNCTION_H
