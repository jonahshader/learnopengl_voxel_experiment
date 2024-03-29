//
// Created by Jonah on 8/17/2020.
//

#include "Components.h"

blockid Components::chunkDataGet(blockid* data, int x, int y, int z) {
    return (data)[z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x];
}

blockid Components::chunkDataGet(std::vector<blockid*>* chunks, int x, int y, int z) {
//    int cx = std::floor(x / (float) CHUNK_SIZE);
//    int cy = std::floor(y / (float) CHUNK_SIZE);
//    int cz = std::floor(z / (float) CHUNK_SIZE);
    int cx = x >= 0 ? x / CHUNK_SIZE : ((x+1) / CHUNK_SIZE) - 1;
    int cy = y >= 0 ? y / CHUNK_SIZE : ((y+1) / CHUNK_SIZE) - 1;
    int cz = z >= 0 ? z / CHUNK_SIZE : ((z+1) / CHUNK_SIZE) - 1;

    int i = (cx + 1) + (cy + 1) * 3 + (cz + 1) * 9;
    int xx = x - cx * CHUNK_SIZE;
    int yy = y - cy * CHUNK_SIZE;
    int zz = z - cz * CHUNK_SIZE;

    return (*chunks)[i][xx + yy * CHUNK_SIZE + zz * CHUNK_SIZE * CHUNK_SIZE];
}


void Components::chunkDataSet(blockid* data, blockid value, int x, int y, int z) {
    (data)[z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x] = value;
}

blockid Components::chunkDataGetAirBounds(blockid* data, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return 0;
    } else {
        return (data)[z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x];
    }
}

std::string Components::dvecToString(glm::dvec3 &vec) {
    return "x: " + std::to_string(vec.x) + " y: " + std::to_string(vec.y) + " z: " + std::to_string(vec.z);
}

bool Components::voxelIsTouchingAirAirBounds(blockid* data, int x, int y, int z) {
    return
        chunkDataGetAirBounds(data, x + 1, y + 0, z + 0) == 0 ||
        chunkDataGetAirBounds(data, x - 1, y + 0, z + 0) == 0 ||
        chunkDataGetAirBounds(data, x + 0, y + 1, z + 0) == 0 ||
        chunkDataGetAirBounds(data, x + 0, y - 1, z + 0) == 0 ||
        chunkDataGetAirBounds(data, x + 0, y + 0, z + 1) == 0 ||
        chunkDataGetAirBounds(data, x + 0, y + 0, z - 1) == 0;
}

bool Components::voxelIsTouchingAirWithNeighbors(std::vector<blockid*>* chunks, int x, int y, int z) {
    return
        chunkDataGet(chunks, x + 1, y + 0, z + 0) == 0 ||
        chunkDataGet(chunks, x - 1, y + 0, z + 0) == 0 ||
        chunkDataGet(chunks, x + 0, y + 1, z + 0) == 0 ||
        chunkDataGet(chunks, x + 0, y - 1, z + 0) == 0 ||
        chunkDataGet(chunks, x + 0, y + 0, z + 1) == 0 ||
        chunkDataGet(chunks, x + 0, y + 0, z - 1) == 0;
}

//bool Components::voxelIsTouchingAirOptimizeChoose(ChunkData &data, ChunkData* chunks[], bool optimize, int x, int y, int z) {
//    if (optimize) {
//        return voxelIsTouchingAir(data, x, y, z);
//    } else {
//        return voxelIsTouchingAir(chunks, x, y, z);
//    }
//}
//
//blockid Components::chunkDataGetOptimizeChoose(ChunkData &data, ChunkData* chunks[], bool optimize, int x, int y, int z) {
//    if (optimize) {
//        return chunkDataGet(data, x, y, z);
//    } else {
//        return chunkDataGet(chunks, x, y, z);
//    }
//}

