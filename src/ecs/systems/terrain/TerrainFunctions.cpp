//
// Created by Jonah on 9/3/2021.
//

#include "TerrainFunctions.h"

posTerrain_t TerrainFunctions::noiseToPosTerrain(FastNoise&& noise) {
    return [noise = std::move(noise)](FN_VEC3 pos) {
        return noise.GetNoise(pos.x, pos.y, pos.z);
    };
}

posTerrain_t TerrainFunctions::noiseToPosTerrain(FastNoise noise) {
    return [noise = std::move(noise)](FN_VEC3 pos) {
        return noise.GetNoise(pos.x, pos.y, pos.z);
    };
}
