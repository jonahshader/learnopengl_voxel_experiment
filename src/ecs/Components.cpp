//
// Created by Jonah on 8/17/2020.
//

#include "Components.h"

unsigned char Components::chunkDataGet(ChunkData &data, int x, int y, int z) {
    return data.data[z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x];
}

void Components::chunkDataSet(ChunkData &data, unsigned char value, int x, int y, int z) {
    data.data[z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x] = value;
}

unsigned char Components::chunkDataGetAirBounds(Components::ChunkData &data, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return 0;
    } else {
        return data.data[z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x];
    }
}

std::string Components::dvecToString(glm::dvec3 &vec) {
    return "x: " + std::to_string(vec.x) + " y: " + std::to_string(vec.y) + " z: " + std::to_string(vec.z);
}
