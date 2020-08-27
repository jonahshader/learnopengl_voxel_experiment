//
// Created by Jonah on 8/17/2020.
//

#include "Components.h"

unsigned char Components::chunkDataGet(ChunkData &data, int x, int y, int z) {
    return data.data[z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x];
}

unsigned char Components::chunkDataGet(ChunkData* chunks[], int x, int y, int z) {
    int cx = std::floor(x / (float) CHUNK_SIZE);
    int cy = std::floor(y / (float) CHUNK_SIZE);
    int cz = std::floor(z / (float) CHUNK_SIZE);

    int i = (cx + 1) + (cy + 1) * 3 + (cz + 1) * 9;
    int xx = x - cx * CHUNK_SIZE;
    int yy = y - cy * CHUNK_SIZE;
    int zz = z - cz * CHUNK_SIZE;

    return chunks[i]->data[xx + yy * CHUNK_SIZE + zz * CHUNK_SIZE * CHUNK_SIZE];
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

bool Components::voxelIsTouchingAir(ChunkData &data, int x, int y, int z) {
    return
        chunkDataGetAirBounds(data, x + 1, y + 0, z + 0) == 0 ||
        chunkDataGetAirBounds(data, x - 1, y + 0, z + 0) == 0 ||
        chunkDataGetAirBounds(data, x + 0, y + 1, z + 0) == 0 ||
        chunkDataGetAirBounds(data, x + 0, y - 1, z + 0) == 0 ||
        chunkDataGetAirBounds(data, x + 0, y + 0, z + 1) == 0 ||
        chunkDataGetAirBounds(data, x + 0, y + 0, z - 1) == 0;
}

bool Components::voxelIsTouchingAir(ChunkData* chunks[], int x, int y, int z) {
    return
            chunkDataGet(chunks, x + 1, y + 0, z + 0) == 0 ||
            chunkDataGet(chunks, x - 1, y + 0, z + 0) == 0 ||
            chunkDataGet(chunks, x + 0, y + 1, z + 0) == 0 ||
            chunkDataGet(chunks, x + 0, y - 1, z + 0) == 0 ||
            chunkDataGet(chunks, x + 0, y + 0, z + 1) == 0 ||
            chunkDataGet(chunks, x + 0, y + 0, z - 1) == 0;
}
