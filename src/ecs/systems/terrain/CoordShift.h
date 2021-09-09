//
// Created by Jonah on 9/6/2021.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_COORDSHIFT_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_COORDSHIFT_H

#include <functional>
#include <random>
#include "TerrainFunctions.h"
#include "NoisePosTerrain.h"


class CoordShift {
private:
    posTerrain_t xShift;
    posTerrain_t yShift;
    posTerrain_t zShift;

public:
    CoordShift(posTerrain_t xShift,
               posTerrain_t yShift,
               posTerrain_t zShift) :
            xShift(xShift), yShift(yShift), zShift(zShift) { }


    CoordShift(NoisePosTerrain baseNoise, FN_DECIMAL magnitude, std::mt19937_64 &mt) {
        baseNoise.SetSeed(mt());
        NoisePosTerrain yNoise = baseNoise;
        yNoise.SetSeed(mt());
        NoisePosTerrain zNoise = baseNoise;
        zNoise.SetSeed(mt());

        xShift = baseNoise * magnitude;
        yShift = yNoise * magnitude;
        zShift = zNoise * magnitude;
    }

    CoordShift(FN_DECIMAL frequency, FN_DECIMAL magnitude, int octaves, std::mt19937_64 mt) {
        auto xShiftNoise = NoisePosTerrain();
        xShiftNoise.SetNoiseType(FastNoise::SimplexFractal);
        xShiftNoise.SetSeed(mt());
        xShiftNoise.SetFrequency(frequency);
        xShiftNoise.SetFractalOctaves(octaves);
        xShift = xShiftNoise * magnitude;

        auto yShiftNoise = NoisePosTerrain();
        yShiftNoise.SetNoiseType(FastNoise::SimplexFractal);
        yShiftNoise.SetSeed(mt());
        yShiftNoise.SetFrequency(frequency);
        yShiftNoise.SetFractalOctaves(octaves);
        yShift = yShiftNoise * magnitude;

        auto zShiftNoise = NoisePosTerrain();
        zShiftNoise.SetNoiseType(FastNoise::SimplexFractal);
        zShiftNoise.SetSeed(mt());
        zShiftNoise.SetFrequency(frequency);
        zShiftNoise.SetFractalOctaves(octaves);
        zShift = zShiftNoise * magnitude;
    }

    FN_VEC3 operator() (FN_VEC3 inputPos) {
        return inputPos + FN_VEC3 { xShift(inputPos), yShift(inputPos), zShift(inputPos) };
    }

};

#endif //LEARNOPENGL_VOXEL_EXPERIMENT_COORDSHIFT_H
