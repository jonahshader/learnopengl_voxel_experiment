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

        coordShift_t twister = CoordShift(0.002, 70.0, 6, mt);



//        posTerrain_t mainTerrainFun = mainNoise;
        posTerrain_t mainTerrainFun = [](FN_VEC3 _){return 0.0f;};
        mainTerrainFun += [](FN_VEC3 inputPos) {
            return (inputPos.y * 0.015f);
        };



        for (int i = 0; i < 1; ++i) {
            auto glitcherNoise = NoisePosTerrain();
            glitcherNoise.SetSeed(mt());
            glitcherNoise.SetFractalOctaves(1);
            glitcherNoise.SetFrequency(0.01 * (i + 1));
            glitcherNoise.SetNoiseType(FastNoise::NoiseType::Cellular);
            glitcherNoise.SetCellularDistanceFunction(FastNoise::CellularDistanceFunction::Manhattan);
//            glitcherNoise.Set
            coordShift_t glitcher = CoordShift(glitcherNoise, 50 / (float)(i+1), mt);
            mainTerrainFun = applyCoordShift(mainTerrainFun, glitcher);
//            mainTerrainFun = applyCoordShift(mainTerrainFun, glitcher);
        }

        mainTerrainFun = applyCoordShift(mainTerrainFun, twister);



//        mainTerrainFun += 5;




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
