//
// Created by Jonah on 8/10/2020.
//

#include "ChunkManagement.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>
#include <iostream>
#include <random>

#include "other/Profiler.h"

entt::registry* ChunkManagement::chunkCompareRegistry = nullptr;
Components::Position ChunkManagement::chunkComparePos = {glm::dvec3(0.0)};

ChunkManagement::ChunkManagement(const char* vertexPathInstVer, const char* fragmentPathInstVer,
                                 const char* vertexPathTriVer, const char* fragmentPathTriVer,
                                 std::mt19937_64 &seeder) :
        chunkKeyToChunkEntity(),
        chunks(),
        sortedChunkPositionLoadSphere(),
        pool(MAX_CONCURRENT_GENERATES + 1 + MAX_CONCURRENT_MESH_GENS),
        voxelShader(vertexPathInstVer, fragmentPathInstVer),
        voxelShaderTris(vertexPathTriVer, fragmentPathTriVer),
        mainTerrainFunction(seeder()),
        mainNoise(),
        terraceOffset(),
        terraceSelect(),
        smoothHillSelect(),
        smoothHill(),
        overallTerrainHeightOffset(),
        caveNoiseA(),
        caveNoiseB(),
        caveNoiseC(),
        smoothHillMagnitude(0.2f),
        fogDistance(1.0f),
        chunksCurrentlyGenerating(0),
        chunksCurrentlyMeshing(0),
        cubeVbo(0)
{
    computeSortedChunkPositionLoadSphere();
    std::mt19937 mt(seeder());
    std::uniform_int_distribution intDist;

    mainNoise.SetNoiseType(FastNoise::SimplexFractal);
    mainNoise.SetFrequency(0.0010);
    mainNoise.SetFractalOctaves(7);
    mainNoise.SetSeed(mt());

    terraceOffset.SetNoiseType(FastNoise::SimplexFractal);
    terraceOffset.SetFrequency(0.006);
    terraceOffset.SetFractalOctaves(3);
    terraceOffset.SetSeed(mt());

    terraceSelect.SetNoiseType(FastNoise::SimplexFractal);
    terraceSelect.SetFrequency(0.002);
    terraceSelect.SetFractalOctaves(2);
    terraceSelect.SetSeed(mt());

    smoothHillSelect.SetNoiseType(FastNoise::SimplexFractal);
    smoothHillSelect.SetFrequency(0.001);
    smoothHillSelect.SetFractalOctaves(3);
    smoothHillSelect.SetSeed(mt());

    smoothHill.SetNoiseType(FastNoise::SimplexFractal);
    smoothHill.SetFrequency(0.0010);
    smoothHill.SetFractalOctaves(5);
    smoothHill.SetSeed(mt());

    overallTerrainHeightOffset.SetNoiseType(FastNoise::SimplexFractal);
    overallTerrainHeightOffset.SetFrequency(0.00001);
    overallTerrainHeightOffset.SetFractalOctaves(3);
    overallTerrainHeightOffset.SetSeed(mt());

    caveNoiseA.SetNoiseType(FastNoise::SimplexFractal);
    caveNoiseA.SetFrequency(0.005);
    caveNoiseA.SetFractalOctaves(3);
    caveNoiseA.SetSeed(mt());

    caveNoiseB.SetNoiseType(FastNoise::SimplexFractal);
    caveNoiseB.SetFrequency(0.005);
    caveNoiseB.SetFractalOctaves(3);
    caveNoiseB.SetSeed(mt());

//    caveNoiseC.SetNoiseType(FastNoise::Cellular);
//    caveNoiseC.SetSeed(intDist(mt));
//    caveNoiseC.SetFrequency(0.02);
//    caveNoiseC.SetCellularReturnType(FastNoise::Distance2Div);
//    caveNoiseC.SetCellularDistanceFunction(FastNoise::Euclidean);
//    caveNoiseC.SetGradientPerturbAmp(2.5f);
//    caveNoiseC.Fr
//    caveNoiseC.SetFractalOctaves(3);

    caveNoiseC.SetNoiseType(FastNoise::NoiseType::CubicFractal);
//    caveNoiseC.SetCellularDistanceFunction()
    caveNoiseC.SetSeed(mt());
    caveNoiseC.SetFractalOctaves(7);
    caveNoiseC.SetFrequency(0.01);
    caveNoiseC.SetFractalType(FastNoise::RigidMulti);


    // buffer cube data
    glGenBuffers(1, &cubeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


ChunkManagement::chunkKey ChunkManagement::chunkPositionToKey(int xChunk, int yChunk, int zChunk) {

//    long output = (xChunk & 0x000FFFFF) + ((yChunk & 0x000FFFFF) << 16) + (((long)zChunk & 0x000FFFFF) << 24);
    return chunkKey{xChunk, yChunk, zChunk};
}

entt::entity* ChunkManagement::getChunk(int xChunk, int yChunk, int zChunk) {
    auto key = chunkPositionToKey(xChunk, yChunk, zChunk);
    auto found = chunkKeyToChunkEntity.find(key);
    if (found == chunkKeyToChunkEntity.end()) {
        return nullptr;
    } else {
        return &found->second;
    }
}

bool ChunkManagement::isChunkDataLoaded(entt::registry &registry, int xChunk, int yChunk, int zChunk) {
    auto key = chunkPositionToKey(xChunk, yChunk, zChunk);
    auto found = chunkKeyToChunkEntity.find(key);
    if (found == chunkKeyToChunkEntity.end()) {
        return false;
    } else {
        if (registry.has<Components::ChunkStatus>(found->second)) {
            auto &status = registry.get<Components::ChunkStatus>(found->second);
            if (status.status != Components::ChunkStatusEnum::NEW &&
                    status.status != Components::ChunkStatusEnum::GENERATING_OR_LOADING) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
}

bool ChunkManagement::isChunkFullyLoaded(entt::registry &registry, int xChunk, int yChunk, int zChunk) {
    auto key = chunkPositionToKey(xChunk, yChunk, zChunk);
    auto found = chunkKeyToChunkEntity.find(key);
    if (found == chunkKeyToChunkEntity.end()) {
//        std::cout << "not in unordered map" << std::endl;
        return false;
    } else {
        if (registry.has<Components::ChunkStatus>(found->second)) {
            auto &status = registry.get<Components::ChunkStatus>(found->second);
            if (status.status == Components::ChunkStatusEnum::MESH_BUFFERED) {
//                std::cout << "success!" << std::endl;
                return true;
            } else {
//                std::cout << "bad mesh status: " << *status.status << std::endl;
                return false;
            }
        } else {
            std::cout << "doesn't have mesh status" << std::endl;
            return false;
        }
    }
}

bool ChunkManagement::getVoxel(entt::registry &registry, int x, int y, int z, blockid &voxel) {
    int xChunk = x >= 0 ? x / CHUNK_SIZE : ((x+1) / CHUNK_SIZE) - 1;
    int yChunk = y >= 0 ? y / CHUNK_SIZE : ((y+1) / CHUNK_SIZE) - 1;
    int zChunk = z >= 0 ? z / CHUNK_SIZE : ((z+1) / CHUNK_SIZE) - 1;

    int xLocal = x - xChunk * CHUNK_SIZE;
    int yLocal = y - yChunk * CHUNK_SIZE;
    int zLocal = z - zChunk * CHUNK_SIZE;

    if (chunkHasData(registry, xChunk, yChunk, zChunk)) {
        voxel = Components::chunkDataGet(getChunkData(registry, xChunk, yChunk, zChunk).data.data(), xLocal, yLocal, zLocal);
        return true;
    } else {
        return false;
    }
}

bool ChunkManagement::inSolidBlock(entt::registry &registry, glm::dvec3 &pos) {
    int xChunk = floor(pos.x / CHUNK_SIZE);
    int yChunk = floor(pos.y / CHUNK_SIZE);
    int zChunk = floor(pos.z / CHUNK_SIZE);

    int xLocal = floor(pos.x) - xChunk * CHUNK_SIZE;
    int yLocal = floor(pos.y) - yChunk * CHUNK_SIZE;
    int zLocal = floor(pos.z) - zChunk * CHUNK_SIZE;
    if (chunkHasData(registry, xChunk, yChunk, zChunk)) {
        if (Components::chunkDataGet(getChunkData(registry, xChunk, yChunk, zChunk).data.data(), xLocal, yLocal, zLocal) != 0)
            return true;
        else return false;
    } else return true;
}

bool ChunkManagement::inSolidBlock(entt::registry &registry, glm::dvec3 &pos, int cx, int cy, int cz) {
    int xLocal = floor(pos.x) - cx * CHUNK_SIZE;
    int yLocal = floor(pos.y) - cy * CHUNK_SIZE;
    int zLocal = floor(pos.z) - cz * CHUNK_SIZE;
    if (chunkHasData(registry, cx, cy, cz)) {
        if (Components::chunkDataGet(getChunkData(registry, cx, cy, cz).data.data(), xLocal, yLocal, zLocal) != 0)
            return true;
        else return false;
    } else return true;
}


Components::ChunkData& ChunkManagement::getChunkData(entt::registry &registry, int xChunk, int yChunk, int zChunk) {
    return registry.get<Components::ChunkData>(chunkKeyToChunkEntity[chunkPositionToKey(xChunk, yChunk, zChunk)]);
}

void ChunkManagement::run(entt::registry &registry) {
    // find central point to load chunks around
    // this is the entity that has the CameraAttach component
    auto playerView = registry.view<Components::CameraAttach, Components::Position, Components::ChunkPosition>();

    // hopefully there is only one with the camera attached :)
    // TODO: this little bit of code is poo
    // i just want to grab one player from playerView, idk how to do that
    entt::entity player;
    bool playerFound = false;
    for (entt::entity e : playerView) {
        player = e;
        playerFound = true;
    }

    if (playerFound) {
        Profiler::getInstance()->start("chunk_load_nested_for_loop");
        // find player location
        auto &playerPos = registry.get<Components::Position>(player);
        auto &playerChunkPos = registry.get<Components::ChunkPosition>(player);



        for (auto& posInSphere : sortedChunkPositionLoadSphere) {
            tryCreateChunk(registry,
                           playerChunkPos.x + posInSphere.x,
                           playerChunkPos.y + posInSphere.y,
                           playerChunkPos.z + posInSphere.z);
        }

        // calculate load bounds for the chunk generate loop
//        int cRadius = ceil(CHUNK_LOAD_RADIUS / (double) CHUNK_SIZE);
//        int xCMin = playerChunkPos.x - cRadius;
//        int xCMax = playerChunkPos.x + cRadius;
//        int yCMin = playerChunkPos.y - cRadius;
//        int yCMax = playerChunkPos.y + cRadius;
//        int zCMin = playerChunkPos.z - cRadius;
//        int zCMax = playerChunkPos.z + cRadius;

//        for (int x = 0; x <= cRadius; ++x) {
//            for (int y = 0; y <= cRadius; ++y) {
//                for (int z = 0; z <= cRadius; ++z) {
//                    int xx = playerChunkPos.x + x;
//                    int yy = playerChunkPos.y + y;
//                    int zz = playerChunkPos.z + z;
//                    int xxi = playerChunkPos.x - x;
//                    int yyi = playerChunkPos.y - y;
//                    int zzi = playerChunkPos.z - z;
//                    double dist = worldPosChunkPosDist2(xx, yy, zz, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
//                    if (dist < CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS) {
//                        // try create
//                        tryCreateChunk(registry, xx, yy, zz);
//                    }
//
//                    dist = worldPosChunkPosDist2(xxi, yy, zz, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
//                    if (dist < CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS) {
//                        // try create
//                        tryCreateChunk(registry, xxi, yy, zz);
//                    }
//
//                    dist = worldPosChunkPosDist2(xx, yyi, zz, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
//                    if (dist < CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS) {
//                        // try create
//                        tryCreateChunk(registry, xx, yyi, zz);
//                    }
//
//                    dist = worldPosChunkPosDist2(xxi, yyi, zz, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
//                    if (dist < CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS) {
//                        // try create
//                        tryCreateChunk(registry, xxi, yyi, zz);
//                    }
//
//                    dist = worldPosChunkPosDist2(xx, yy, zzi, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
//                    if (dist < CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS) {
//                        // try create
//                        tryCreateChunk(registry, xx, yy, zzi);
//                    }
//
//                    dist = worldPosChunkPosDist2(xxi, yy, zzi, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
//                    if (dist < CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS) {
//                        // try create
//                        tryCreateChunk(registry, xxi, yy, zzi);
//                    }
//
//                    dist = worldPosChunkPosDist2(xx, yyi, zzi, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
//                    if (dist < CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS) {
//                        // try create
//                        tryCreateChunk(registry, xx, yyi, zzi);
//                    }
//
//                    dist = worldPosChunkPosDist2(xxi, yyi, zzi, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
//                    if (dist < CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS) {
//                        // try create
//                        tryCreateChunk(registry, xxi, yyi, zzi);
//                    }
//                }
//            }
//        }

        /*
         * TODO:
         * check how many loaded + loading chunks there are. if this is less than max gen then load more at the player pos
         * iterate through all loaded, visible, non-air, non-solid chunks and try to load the neighboring chunks that
         * have air touching the neighbor. put these all into the list and sort them like usual. this should significantly
         * reduce the number of chunks that need to be sorted while only loading in visible chunks for the most part.
         * caves won't be loaded in unless they are touching the surface chunks in some way. air chunks won't be loaded
         * in which could be a problem if there are floating islands. might implement chunk sampling to determine if
         * it is worth loading the rest of the chunk. if that fails then i could just load all air chunks like normal.
         * i would still get a performance boost from not generating fully solid chunks. i'm not sure how to determine
         * the fog distance yet. the current method won't work anymore. maybe i should do per chunk blurring. like
         * a fog-of-war kind of thing. that way the chunk regions could be loaded asymmetrically and rendered without
         * having to cut out the parts that like outside of the fog distance.
         */

//        for (int x = xCMin; x <= xCMax; x++) {
//            for (int y = yCMin; y <= yCMax; y++) {
//                for (int z = zCMin; z <= zCMax; z++) {
//                    // try to create an entity for every chunk here if they are in range
//                    double dist = worldPosChunkPosDist(x, y, z, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
////                    std::cout << "dist: " << dist << std::endl;
//                    if (dist < CHUNK_LOAD_RADIUS) {
//                        // try create
//                        tryCreateChunk(registry, x, y, z);
//                    }
//                }
//            }
//        }
        Profiler::getInstance()->end("chunk_load_nested_for_loop");
        Profiler::getInstance()->start("sort_chunk_list");
        // sort chunks by closest to furthest
        // first need to set variables used by the comparor function
        chunkComparePos = playerPos;
        chunkCompareRegistry = &registry;
        // perform sort
//        std::cout << chunks.size() << std::endl;
        std::sort(chunks.begin(), chunks.end(), chunkCompareFun);
        Profiler::getInstance()->end("sort_chunk_list");
        Profiler::getInstance()->start("progress_chunk_stages");

        // init fogDistance to CHUNK_LOAD_RADIUS
        fogDistance = CHUNK_LOAD_RADIUS;

        // progress chunks through stages
        int buffers = 0;
        int generates = 0;
        std::vector<entt::entity> chunksSuccessfullyRemoved;
        double processTime = glfwGetTime();
        for (auto chunkEntity : chunks) {
            auto [chunkStatus, chunkPosition] = registry.get<Components::ChunkStatus, Components::ChunkPosition>(chunkEntity);

//            updateChunkLocalFog(registry, chunkEntity);
            // check if this chunk is in range
            // as long as these chunks are within the unload radius, they can continue loading
            // TODO: reverse this so that distance starts at zero and goes up to the greatest distance
            // local fog chunk will remove visible chunk seams
            if (chunkStatus.status != Components::ChunkStatusEnum::MESH_BUFFERED) {
                float dist = worldPosChunkPosDist(chunkPosition, playerPos);
                if (dist < fogDistance)
                    fogDistance = dist;
            }

            if (chunkStatus.markedForRemoval) {
                switch (chunkStatus.status) {
                    case Components::ChunkStatusEnum::NEW:
                    case Components::ChunkStatusEnum::GENERATED_OR_LOADED:
                    case Components::ChunkStatusEnum::MESH_GENERATED:
                    case Components::ChunkStatusEnum::MESH_BUFFERED:
                        tryRemoveChunk(registry, chunkEntity);
                        chunksSuccessfullyRemoved.push_back(chunkEntity);
                        break;
                    default:
                        break;
                }
            } else if (worldPosChunkPosDist2(chunkPosition, playerPos) < CHUNK_UNLOAD_RADIUS * CHUNK_UNLOAD_RADIUS) {
                // figure out what the status of this chunk is and make it move to the next stage
                switch (chunkStatus.status) {
                    case Components::ChunkStatusEnum::NEW:
                        if (generates < MAX_GENERATES_PER_FRAME && chunksCurrentlyGenerating < MAX_CONCURRENT_GENERATES) {
                            registry.emplace<Components::ChunkData>(chunkEntity, std::vector<blockid>(VOXELS_PER_CHUNK)); // add chunk data
                            chunkStatus.status = Components::ChunkStatusEnum::GENERATING_OR_LOADING;
                            auto &chunkData = registry.get<Components::ChunkData>(chunkEntity);
                            chunksCurrentlyGenerating++;
//                            updateNeighborChunksLocalFog(registry, chunkPosition);
                            pool.enqueue(&ChunkManagement::generateChunk, this, &chunkStatus.status, chunkPosition.x, chunkPosition.y, chunkPosition.z, chunkData.data.data());
                            ++generates;
                        }
                        break;
                    case Components::ChunkStatusEnum::GENERATED_OR_LOADED:
                        if (chunksCurrentlyMeshing < MAX_CONCURRENT_MESH_GENS)
                        {
                            if (chunkHasAllNeighborData(registry, chunkPosition)) {
#ifdef INST_MODE
                                // add mesh data
                                registry.emplace<Components::ChunkMeshData>(chunkEntity,
                                                                            new std::vector<unsigned char>(),
                                                                            new std::vector<unsigned char>(),
                                                                            new std::vector<unsigned char>(),
                                                                            new std::vector<unsigned char>());
                                *chunkStatus.status = Components::ChunkStatusEnum::MESH_GENERATING;
                                auto [chunkData, chunkMeshData] = registry.get<Components::ChunkData, Components::ChunkMeshData>(chunkEntity);

                                std::vector<blockid*>* neighborChunks = new std::vector<blockid*>(27);
                                for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) {
                                    (*neighborChunks)[x + y * 3 + z * 9] = getChunkData(registry, chunkPosition.x + x - 1, chunkPosition.y + y - 1, chunkPosition.z + z - 1).data.data();
                                }
                                chunksCurrentlyMeshing++;
                                pool.enqueue(&ChunkManagement::generateMesh, this, chunkStatus.status, chunkData.data.data(), chunkMeshData.offsets, chunkMeshData.dims, chunkMeshData.textures, chunkMeshData.brightnesses, neighborChunks);
//                                generateMeshInstances(chunkStatus.status, chunkData.data.data(), chunkMeshData.offsets, chunkMeshData.dims, chunkMeshData.textures, chunkMeshData.brightnesses, neighborChunks);
#endif//INST_MODE
#ifdef TRI_MODE
                                // add tri mesh data
                                registry.emplace<Components::ChunkMeshDataTris>(chunkEntity, new std::vector<unsigned char>());
                                chunkStatus.status = Components::ChunkStatusEnum::MESH_GENERATING;
                                auto [chunkData, chunkMeshDataTris] = registry.get<Components::ChunkData, Components::ChunkMeshDataTris>(chunkEntity);

                                std::vector<blockid*>* neighborChunks = new std::vector<blockid*>(27);
                                for (int z = 0; z < 3; ++z) for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x) {
                                        (*neighborChunks)[x + y * 3 + z * 9] = getChunkData(registry, chunkPosition.x + x - 1, chunkPosition.y + y - 1, chunkPosition.z + z - 1).data.data();
                                    }
//                                updateNeighborChunksLocalFog(registry, chunkPosition);
                                chunksCurrentlyMeshing++;
                                pool.enqueue(&ChunkManagement::generateMeshInstTris, this, &chunkStatus.status, chunkData.data.data(), chunkMeshDataTris.tris, neighborChunks);
//                                pool.enqueue(&ChunkManagement::generateMeshGreedy, this, chunkStatus.status, chunkData.data.data(), chunkMeshDataTris.tris, neighborChunks);
#endif//TRI_MODE
                            }
                        }

                        break;
                    case Components::ChunkStatusEnum::MESH_GENERATED:
                        if (buffers < MAX_MESH_BUFFERS_PER_FRAME) {
#ifdef INST_MODE
                            genVboVaoAndBuffer(registry, chunkEntity);
#endif//INST_MODE
#ifdef TRI_MODE
                            genVboVaoAndBufferTris(registry, chunkEntity);
#endif//TRI_MODE
                            chunkStatus.status = Components::ChunkStatusEnum::MESH_BUFFERED;
                            // add ChunkFog component and compute
                            registry.emplace<Components::ChunkFog>(chunkEntity);
                            updateNeighborChunksLocalFog(registry, chunkPosition);
                            ++buffers;
                        }
                        break;
                    default:
                        break;
                }
            } else { // these are outside of the unload radius, so they will be queued for removal
                chunkStatus.markedForRemoval = true;
            }
        }
        Profiler::getInstance()->end("progress_chunk_stages");



        Profiler::getInstance()->start("erase_chunks");
        //
//        std::cout << "Process time: " << glfwGetTime() - processTime << std::endl;

        // shrink fog distance
        fogDistance -= sqrt(3 * pow(CHUNK_SIZE/2, 2));
        fogDistance = std::max(fogDistance, 1.0f);

        // erase chunks from chunks vector
        for (auto cToRemove : chunksSuccessfullyRemoved) {
            for (int j = 0; j < chunks.size(); j++) {
                if (chunks[j] == cToRemove) {
                    chunks.erase(chunks.begin() + j);
                    j = -1;
                }
            }
        }
        Profiler::getInstance()->end("erase_chunks");

        auto renderableChunksTris = registry.group<Components::ChunkPosition, Components::ChunkOpenGLTriVer, Components::ChunkFog>();

    }
}

void ChunkManagement::tryCreateChunk(entt::registry &registry, int xChunk, int yChunk, int zChunk) {
    // check if this chunk already exists
    auto key = chunkPositionToKey(xChunk, yChunk, zChunk);
    auto found = chunkKeyToChunkEntity.find(key);
    // if a chunk with this position is not already here, continue making the new chunk entity
    if (found == chunkKeyToChunkEntity.end()) {
        auto entity = registry.create();
        registry.emplace<Components::ChunkStatus>(entity, Components::ChunkStatusEnum{Components::ChunkStatusEnum::NEW}, false);
        registry.emplace<Components::ChunkPosition>(entity, xChunk, yChunk, zChunk);

        chunks.emplace_back(entity);
        chunkKeyToChunkEntity.insert({key, entity});

//        std::cout << "made a chunk" << std::endl;
    }
}

void ChunkManagement::tryRemoveChunk(entt::registry &registry, entt::entity chunkEntity) {
    auto [chunkStatus, chunkPosition] = registry.get<Components::ChunkStatus, Components::ChunkPosition>(chunkEntity);

    auto key = chunkPositionToKey(chunkPosition.x, chunkPosition.y, chunkPosition.z);
    // remove from unordered map
    chunkKeyToChunkEntity.erase(key);

    // notify neighbors for fog update
    updateNeighborChunksLocalFog(registry, chunkPosition);

    // try remove gl
    if (registry.has<Components::ChunkOpenGLInstanceVer>(chunkEntity)) {
        auto &chunkgl = registry.get<Components::ChunkOpenGLInstanceVer>(chunkEntity);
        glDeleteVertexArrays(1, &chunkgl.vao);
        glDeleteBuffers(1, &chunkgl.offsetsVbo);
        glDeleteBuffers(1, &chunkgl.texturesVbo);
        glDeleteBuffers(1, &chunkgl.brightnessesVbo);
    }
    if (registry.has<Components::ChunkOpenGLTriVer>(chunkEntity)) {
        auto &chunkgl = registry.get<Components::ChunkOpenGLTriVer>(chunkEntity);
        glDeleteVertexArrays(1, &chunkgl.vao);
        glDeleteBuffers(1, &chunkgl.vbo);
    }
    if (registry.has<Components::ChunkMeshData>(chunkEntity)) {
        auto &chunkMeshData = registry.get<Components::ChunkMeshData>(chunkEntity);
        if (chunkMeshData.offsets != nullptr) {
            delete chunkMeshData.offsets;
            chunkMeshData.offsets = nullptr;
        }
        if (chunkMeshData.dims != nullptr) {
            delete chunkMeshData.dims;
            chunkMeshData.dims = nullptr;
        }
        if (chunkMeshData.textures != nullptr) {
            delete chunkMeshData.textures;
            chunkMeshData.textures = nullptr;
        }
        if (chunkMeshData.brightnesses != nullptr) {
            delete chunkMeshData.brightnesses;
            chunkMeshData.brightnesses = nullptr;
        }
    }

    if (registry.has<Components::ChunkMeshDataTris>(chunkEntity)) {
        auto &chunkMeshDataTris = registry.get<Components::ChunkMeshDataTris>(chunkEntity);
        if (chunkMeshDataTris.tris != nullptr) {
            delete chunkMeshDataTris.tris;
            chunkMeshDataTris.tris = nullptr;
        }
    }

//    if (registry.has<Components::ChunkData>(chunkEntity)) {
//        auto &chunkData = registry.get<Components::ChunkData>(chunkEntity);
//    }

    // remove from registry
    registry.destroy(chunkEntity);
}

double ChunkManagement::worldPosChunkPosDist(Components::ChunkPosition &chunkPos, Components::Position &worldPos) {
    glm::dvec3 chunkWorldPos(chunkPos.x * CHUNK_SIZE + (CHUNK_SIZE / 2.),
                             chunkPos.y * CHUNK_SIZE + (CHUNK_SIZE / 2.),
                             chunkPos.z * CHUNK_SIZE + (CHUNK_SIZE / 2.));
    chunkWorldPos -= worldPos.pos;
    return glm::length(chunkWorldPos);
}

double ChunkManagement::worldPosChunkPosDist2(Components::ChunkPosition &chunkPos, Components::Position &worldPos) {
    return worldPosChunkPosDist2(chunkPos.x, chunkPos.y, chunkPos.z, worldPos.pos.x, worldPos.pos.y, worldPos.pos.z);
//    glm::dvec3 chunkWorldPos(chunkPos.x * CHUNK_SIZE + (CHUNK_SIZE / 2.),
//                             chunkPos.y * CHUNK_SIZE + (CHUNK_SIZE / 2.),
//                             chunkPos.z * CHUNK_SIZE + (CHUNK_SIZE / 2.));
//    chunkWorldPos -= worldPos.pos;
//    return glm::length(chunkWorldPos);
}

double ChunkManagement::worldPosChunkPosDist(int xc, int yc, int zc, double x, double y, double z) {
    double xx = xc * CHUNK_SIZE + (CHUNK_SIZE / 2.);
    double yy = yc * CHUNK_SIZE + (CHUNK_SIZE / 2.);
    double zz = zc * CHUNK_SIZE + (CHUNK_SIZE / 2.);

    return sqrt(pow(xx-x, 2) + pow(yy-y, 2) + pow(zz-z, 2));
}

double ChunkManagement::worldPosChunkPosDist2(int xc, int yc, int zc, double x, double y, double z) {
    double xx = xc * CHUNK_SIZE + (CHUNK_SIZE / 2.);
    double yy = yc * CHUNK_SIZE + (CHUNK_SIZE / 2.);
    double zz = zc * CHUNK_SIZE + (CHUNK_SIZE / 2.);

    return (xx-x) * (xx-x) + (yy-y) * (yy-y) + (zz-z) * (zz-z);
//    return pow(xx-x, 2) + pow(yy-y, 2) + pow(zz-z, 2);
}

bool ChunkManagement::chunkCompareFun(entt::entity chunk1, entt::entity chunk2) {
    double dist1 = worldPosChunkPosDist2(chunkCompareRegistry->get<Components::ChunkPosition>(chunk1), chunkComparePos);
    double dist2 = worldPosChunkPosDist2(chunkCompareRegistry->get<Components::ChunkPosition>(chunk2), chunkComparePos);
    return dist1 < dist2;
}

bool ChunkManagement::chunkPosCompareFun(Components::ChunkPosition chunk1, Components::ChunkPosition chunk2) {
    return (chunk1.x * chunk1.x) + (chunk1.y * chunk1.y) + (chunk1.z * chunk1.z) <
    (chunk2.x * chunk2.x) + (chunk2.y * chunk2.y) + (chunk2.z * chunk2.z);
}

void ChunkManagement::generateChunk(volatile Components::ChunkStatusEnum* chunkStatus, int xChunk, int yChunk, int zChunk,
                                    blockid* chunkData) {
    for (int i = 0; i < VOXELS_PER_CHUNK; ++i) {
        // convert zr yr xr to xr yr zr indices for FastNoiseSIMD library
        int xr = i % CHUNK_SIZE;
        int yr = (i / CHUNK_SIZE) % CHUNK_SIZE;
        int zr = (i / (CHUNK_SIZE * CHUNK_SIZE));
        xr += xChunk * CHUNK_SIZE;
        yr += yChunk * CHUNK_SIZE;
        zr += zChunk * CHUNK_SIZE;
        float x = xr;
        float y = yr;
        float z = zr;

        chunkData[i] = mainTerrainFunction.getMainFun()(FN_VEC3{x, y, z});
    }


    //update state to GENERATED_OR_LOADED
    *chunkStatus = Components::ChunkStatusEnum::GENERATED_OR_LOADED;
    chunksCurrentlyGenerating--;
}

//void ChunkManagement::generateChunk(volatile Components::ChunkStatusEnum* chunkStatus, int xChunk, int yChunk, int zChunk,
//                                    blockid* chunkData) {
//
////    float* noiseSet = mainNoise->GetSimplexFractalSet(chunkPosition.x * CHUNK_SIZE,
////                                                  chunkPosition.y * CHUNK_SIZE,
////                                                  chunkPosition.z * CHUNK_SIZE,
////                                                  chunkPosition.x * CHUNK_SIZE + CHUNK_SIZE,
////                                                  chunkPosition.y * CHUNK_SIZE + CHUNK_SIZE,
////                                                  chunkPosition.z * CHUNK_SIZE + CHUNK_SIZE);
////    chunkData.data.resize(VOXELS_PER_CHUNK);
//
//    // convert float data to voxel data
////#pragma omp parallel for
//    for (int i = 0; i < VOXELS_PER_CHUNK; ++i) {
//        // convert zr yr xr to xr yr zr indices for FastNoiseSIMD library
//        int xr = i % CHUNK_SIZE;
//        int yr = (i / CHUNK_SIZE) % CHUNK_SIZE;
//        int zr = (i / (CHUNK_SIZE * CHUNK_SIZE));
//        xr += xChunk * CHUNK_SIZE;
//        yr += yChunk * CHUNK_SIZE;
//        zr += zChunk * CHUNK_SIZE;
//        float x = xr;
//        float y = yr;
//        float z = zr;
//        y += overallTerrainHeightOffset.GetNoise(xr, zr) * 1000.0f;
//
//
//        float slopeScaler = 0.0015f;
////        float terraceScalar = 5.0f;
//        float terraceSize = 12.0;
//
//        float terraceValRaw = terraceOffset.GetNoise(x, z);
//        float terraceSelectRaw = terraceSelect.GetNoise(x, z);
//        float terraceVal = ((terraceValRaw + 1.f) * 0.5f) * ((terraceSelectRaw > 0 ? 1 : -1) + 1.0f) * 0.5f;
////        terraceVal = pow(terraceVal, 0.1f);
////        terraceVal = 1.0f;
//
//        float yTerraced = (floorf(y / terraceSize) + 0.5f) * terraceSize;
////        y = yTerraced;
//        y = (terraceVal * yTerraced) + (1 - terraceVal) * y;
//
//        float slope = -y * slopeScaler;
//        float mainNoiseVal = mainNoise.GetNoise(x, y, z) + slope;
//        float smoothHillVal = smoothHill.GetNoise(x, y, z) * smoothHillMagnitude + slope;
//        float smoothHillSelectVal = pow((smoothHillSelect.GetNoise(x, y, z) + 1.0f) * 0.5f, 0.6f);
//
////        float caveA = caveNoiseA.GetNoise(xr, yr, zr);
////        float caveB = caveNoiseB.GetNoise(xr, yr, zr);
////        float caveWarpScalar = 0.0f;
////        float caveC = caveNoiseC.GetNoise(caveNoiseA.GetNoise(yr, zr) * caveWarpScalar + xr,
////                                          caveNoiseA.GetNoise(xr, zr) * caveWarpScalar + yr,
////                                          caveNoiseA.GetNoise(xr, yr) * caveWarpScalar + zr);
////        float caveMixed = caveC;
////        bool isCave = caveMixed > 0.85;
////        bool isCave = false;
//
//        float c = caveNoiseC.GetNoise(xr, yr, zr);
//        bool isCave = std::abs(c) > 0.15;
//
////        if (((xr == 0) || (zr == 0)) && (((yr % 4)/3) == 0)) {
////            chunkData[i] = 0;
////        } else
//        if (!isCave) {
//            float noiseOut = mainNoiseVal * (1-smoothHillSelectVal) + smoothHillVal * smoothHillSelectVal;
////            if (noiseOut > 0) {
//                if (terraceVal > 0.5 && noiseOut > 0.00 && yTerraced < y) {
//                    chunkData[i] = 4;
//                } else
//                    if (noiseOut > 0.1) {
//                    (chunkData)[i] = 3;
//                } else if (noiseOut > 0.05) {
//                    (chunkData)[i] = 2;
//                } else if (noiseOut > 0.0) {
//                    (chunkData)[i] = 1;
//                } else {
//                    (chunkData)[i] = 0;
//                }
////            } else {
////                chunkData[i] = 0;
////            }
//        } else {
//            chunkData[i] = 0;
//        }
//    }
//
//
//    //update state to GENERATED_OR_LOADED
//    *chunkStatus = Components::ChunkStatusEnum::GENERATED_OR_LOADED;
////    std::cout << "Done generating chunk" << std::endl;
//    chunksCurrentlyGenerating--;
//}

void ChunkManagement::generateMeshInstances(volatile Components::ChunkStatusEnum* chunkStatus,
                                            blockid* chunkData, std::vector<unsigned char>* offsets,
                                            std::vector<unsigned char>* dims, std::vector<unsigned char>* textures,
                                            std::vector<unsigned char>* brightnesses, std::vector<blockid*>* neighborChunks) {

    bool meshed[VOXELS_PER_CHUNK] = {false};
    unsigned char bVals[VOXELS_PER_CHUNK] = {0};

    fixGrass(chunkData, neighborChunks);
    calculateBrightness(bVals, chunkData, neighborChunks);

    for (int z = 0; z < CHUNK_SIZE; ++z) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                if (!meshed[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE]) {
                    auto material = Components::chunkDataGet(chunkData, x, y, z);
                    auto brightness = bVals[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE];
                    bool canUseCheaperVoxelFun = (x > 0 && x < CHUNK_SIZE - 1 && y > 0 && y < CHUNK_SIZE - 1 && z > 0 && z < CHUNK_SIZE - 1);

                    if (material != 0 && Components::voxelIsTouchingAirWithNeighbors(neighborChunks, x, y, z)) {
                        int xSize = 0;
                        int ySize = 1;
                        int zSize = 1;

                        int xCurrent = x;

                        while (xCurrent < CHUNK_SIZE &&
                        !meshed[xCurrent + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] &&
                        Components::chunkDataGet(chunkData, xCurrent, y, z) == material &&
                        bVals[xCurrent + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] == brightness &&
                                Components::voxelIsTouchingAirWithNeighbors(neighborChunks, xCurrent, y, z)) {
                            ++xCurrent;
                            ++xSize;
                        }
                        for (int yl = y + 1; yl < CHUNK_SIZE; ++yl) {
                            for (int xl = x; xl < x + xSize; ++xl) {
                                if (!meshed[xl + yl * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] &&
                                Components::chunkDataGet(chunkData, xl, yl, z) == material &&
                                bVals[xl + yl * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] == brightness &&
                                        Components::voxelIsTouchingAirWithNeighbors(neighborChunks, xl, yl, z)) {
                                } else {
                                    goto exitLoops;
                                }
                            }
                            ++ySize;
                        }
                        exitLoops:

                        for (int zl = z + 1; zl < CHUNK_SIZE; ++zl) {
                            for (int yl = y; yl < y + ySize; ++yl) {
                                for (int xl = x; xl < x + xSize; ++xl) {
                                    if (!meshed[xl + yl * CHUNK_SIZE + zl * CHUNK_SIZE * CHUNK_SIZE] &&
                                        Components::chunkDataGet(chunkData, xl, yl, zl) == material &&
                                        bVals[xl + yl * CHUNK_SIZE + zl * CHUNK_SIZE * CHUNK_SIZE] == brightness &&
                                            Components::voxelIsTouchingAirWithNeighbors(neighborChunks, xl, yl, zl)) {
                                    } else {
                                        goto exitLoops2;
                                    }
                                }
                            }
                            ++zSize;
                        }
                        exitLoops2:

                        for (int zl = z; zl < z + zSize; ++zl) {
                            for (int yl = y; yl < y + ySize; ++yl) {
                                for (int xl = x; xl < x + xSize; ++xl) {
                                    meshed[xl + yl * CHUNK_SIZE + zl * CHUNK_SIZE * CHUNK_SIZE] = true;
                                }
                            }
                        }


                        // make mesh
                        brightnesses->emplace_back(brightness);

                        switch (material) {
                            case 1:
                                textures->emplace_back(1);
                                textures->emplace_back(1);
                                textures->emplace_back(1);
                                textures->emplace_back(1);
                                textures->emplace_back(0);
                                textures->emplace_back(2);
                                break;
                            case 2:
                                textures->emplace_back(2);
                                textures->emplace_back(2);
                                textures->emplace_back(2);
                                textures->emplace_back(2);
                                textures->emplace_back(2);
                                textures->emplace_back(2);
                                break;
                            case 3:
                                textures->emplace_back(3);
                                textures->emplace_back(3);
                                textures->emplace_back(3);
                                textures->emplace_back(3);
                                textures->emplace_back(3);
                                textures->emplace_back(3);
                                break;
                        }


                        offsets->emplace_back(x);
                        offsets->emplace_back(y);
                        offsets->emplace_back(z);

                        dims->emplace_back(xSize);
                        dims->emplace_back(ySize);
                        dims->emplace_back(zSize);

                    }
                }
            }
        }
    }


    // after generating, change state
    *chunkStatus = Components::ChunkStatusEnum::MESH_GENERATED;
//    std::cout << "Done generating mesh" << std::endl;
    // also, i dont need that neighborChunks vector anymore. dont wanna leak memory :>
    neighborChunks->clear();
    delete neighborChunks;
    neighborChunks = nullptr;
    chunksCurrentlyMeshing--;
}

void ChunkManagement::genVboVaoAndBuffer(entt::registry& registry, entt::entity chunkEntity) {
    registry.emplace<Components::ChunkOpenGLInstanceVer>(chunkEntity);
    auto [gl, mesh] = registry.get<Components::ChunkOpenGLInstanceVer, Components::ChunkMeshData>(chunkEntity);
    // set numTriangles for rendering later
    gl.numInstances = mesh.brightnesses->size();

    glGenVertexArrays(1, &gl.vao);
    glGenBuffers(1, &gl.offsetsVbo);
    glGenBuffers(1, &gl.dimsVbo);
    glGenBuffers(1, &gl.texturesVbo);
    glGenBuffers(1, &gl.brightnessesVbo);

    glBindBuffer(GL_ARRAY_BUFFER, gl.offsetsVbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.offsets->size() * sizeof(unsigned char), mesh.offsets->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, gl.dimsVbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.dims->size() * sizeof(unsigned char), mesh.dims->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, gl.texturesVbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.textures->size() * sizeof(unsigned char), mesh.textures->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, gl.brightnessesVbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.brightnesses->size() * sizeof(unsigned char), mesh.brightnesses->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(gl.vao);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
    // cube xyz
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0);
    // cube tex
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    // cube norm
    glVertexAttribPointer(2, 1, GL_FLOAT, false, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // offset
    glBindBuffer(GL_ARRAY_BUFFER, gl.offsetsVbo);
    glVertexAttribIPointer(3, 3, GL_UNSIGNED_BYTE, 3 * sizeof(unsigned char), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    // dims
    glBindBuffer(GL_ARRAY_BUFFER, gl.dimsVbo);
    glVertexAttribIPointer(4, 3, GL_UNSIGNED_BYTE, 3 * sizeof(unsigned char), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    // textures
    glBindBuffer(GL_ARRAY_BUFFER, gl.texturesVbo);

    for (unsigned int i = 0; i < 6; i++) {
        glVertexAttribIPointer(5 + i, 1, GL_UNSIGNED_BYTE, 6 * sizeof(unsigned char), (void*)(size_t)i);
        glEnableVertexAttribArray(5 + i );
        glVertexAttribDivisor(5 + i, 1);
    }

    // brightnesses
    glBindBuffer(GL_ARRAY_BUFFER, gl.brightnessesVbo);
    glVertexAttribIPointer(11, 1, GL_UNSIGNED_BYTE, 1 * sizeof(unsigned char), (void*)0);
    glEnableVertexAttribArray(11);
    glVertexAttribDivisor(11, 1);

    // unbind buffer i guess
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // unbind vao
    glBindVertexArray(0);

    // delete mesh data as it is no longer needed
    if (mesh.offsets != nullptr) {
        delete mesh.offsets;
        mesh.offsets = nullptr;
    }
    if (mesh.dims != nullptr) {
        delete mesh.dims;
        mesh.dims = nullptr;
    }
    if (mesh.textures != nullptr) {
        delete mesh.textures;
        mesh.textures = nullptr;
    }
    if (mesh.brightnesses != nullptr) {
        delete mesh.brightnesses;
        mesh.brightnesses = nullptr;
    }
    registry.remove<Components::ChunkMeshData>(chunkEntity);
}

void ChunkManagement::render(entt::registry &registry, TextureManager &tm, int screenWidth, int screenHeight, const glm::vec3 &skyColor) {
    Profiler::getInstance()->start("chunks_render");
    /*
     * find player
     * convert player orientation into transform matrix
     * store this in the opengl uniform buffer thingy
     * set opengl uniform buffer
     * loop through all chunks
     * for each, set the chunkoffset uniform
     * also render if trianges > 0
     */

    auto playerView = registry.view<Components::CameraAttach, Components::Position, Components::ChunkPosition, Components::DirectionPitchYaw>();

    entt::entity player;
    bool playerFound = false;
    for (entt::entity e : playerView) {
        player = e;
        playerFound = true;
    }

    if (playerFound) {
        // set texture unit
//        glActiveTexture(GL_TEXTURE0 + tm.getTextureInfoA("grass_top").textureUnit);
        auto [playerCam, playerPos, playerChunkPos, playerDir] = registry.get<Components::CameraAttach, Components::Position, Components::ChunkPosition, Components::DirectionPitchYaw>(player);

        glm::mat4 projection = glm::perspective(glm::radians((float) playerCam.fov), screenWidth / (float) screenHeight, 0.1f, (float) CHUNK_LOAD_RADIUS);

//        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3((float) -(playerPos.pos.x - playerChunkPos.x * CHUNK_SIZE), (float) -(playerPos.pos.y - playerChunkPos.y * CHUNK_SIZE), (float) -(playerPos.pos.z - playerChunkPos.z * CHUNK_SIZE)));
        glm::mat4 view = glm::rotate(projection, (float) playerDir.pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, (float) playerDir.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3((float) -(playerPos.pos.x + playerCam.posOffset.x - playerChunkPos.x * CHUNK_SIZE),
                                              (float) -(playerPos.pos.y + playerCam.posOffset.y - playerChunkPos.y * CHUNK_SIZE),
                                              (float) -(playerPos.pos.z + playerCam.posOffset.z - playerChunkPos.z * CHUNK_SIZE)));

        // move the frustum back a little
        glm::mat4 frustumForZCut = glm::translate(projection, glm::vec3(0, 0, -CHUNK_SIZE));
        glm::mat4 frustumForXYCut = glm::translate(projection, glm::vec3(0, 0, -CHUNK_SIZE * 1.5f));

        frustumForZCut = glm::rotate(frustumForZCut, (float) playerDir.pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
        frustumForXYCut = glm::rotate(frustumForXYCut, (float) playerDir.pitch, glm::vec3(-1.0f, 0.0f, 0.0f));

        frustumForZCut = glm::rotate(frustumForZCut, (float) playerDir.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        frustumForXYCut = glm::rotate(frustumForXYCut, (float) playerDir.yaw, glm::vec3(0.0f, 1.0f, 0.0f));

        frustumForZCut = glm::translate(frustumForZCut, glm::vec3((float) -(playerPos.pos.x + playerCam.posOffset.x - playerChunkPos.x * CHUNK_SIZE),
                                                                  (float) -(playerPos.pos.y + playerCam.posOffset.y - playerChunkPos.y * CHUNK_SIZE),
                                                                  (float) -(playerPos.pos.z + playerCam.posOffset.z - playerChunkPos.z * CHUNK_SIZE)));
        frustumForXYCut = glm::translate(frustumForXYCut, glm::vec3((float) -(playerPos.pos.x + playerCam.posOffset.x - playerChunkPos.x * CHUNK_SIZE),
                                                                    (float) -(playerPos.pos.y + playerCam.posOffset.y - playerChunkPos.y * CHUNK_SIZE),
                                                                    (float) -(playerPos.pos.z + playerCam.posOffset.z - playerChunkPos.z * CHUNK_SIZE)));

#ifdef INST_MODE
        voxelShader.use();
        voxelShader.setFloat("fogDistance", fogDistance);
        voxelShader.setVec3("skyColor", skyColor.r, skyColor.g, skyColor.b);
        voxelShader.setUInt("chunkSize", CHUNK_SIZE);
        voxelShader.setMatrix4("viewProjection", view);
        voxelShader.setVec3("camPos",
                                playerPos.pos.x + playerCam.posOffset.x - playerChunkPos.x * CHUNK_SIZE,
                                playerPos.pos.y + playerCam.posOffset.y - playerChunkPos.y * CHUNK_SIZE,
                                playerPos.pos.z + playerCam.posOffset.z - playerChunkPos.z * CHUNK_SIZE);

        auto renderableChunks = registry.view<Components::ChunkPosition, Components::ChunkOpenGLInstanceVer>();

        // render visible chunks
        for (auto c : renderableChunks) {
            auto [chunkPos, chunkGL] = renderableChunks.get<Components::ChunkPosition, Components::ChunkOpenGLInstanceVer>(c);

            if (chunkGL.numInstances > 0) {
                glm::vec4 chunkPoint((chunkPos.x - playerChunkPos.x) * CHUNK_SIZE + CHUNK_SIZE / 2, (chunkPos.y - playerChunkPos.y) * CHUNK_SIZE + CHUNK_SIZE / 2, (chunkPos.z - playerChunkPos.z) * CHUNK_SIZE + CHUNK_SIZE / 2, 1);
                chunkPoint = frustum * chunkPoint;
                chunkPoint = chunkPoint / chunkPoint.w;
                if (chunkPoint.z > 0 && chunkPoint.x > -1 && chunkPoint.x < 1 && chunkPoint.y > -1 && chunkPoint.y < 1) {
//                    std::cout << chunkPoint.x << " " << chunkPoint.y << " " << chunkPoint.z << " " << chunkPoint.w << std::endl;
                    voxelShader.setVec3i("chunkPos", chunkPos.x - playerChunkPos.x, chunkPos.y - playerChunkPos.y, chunkPos.z - playerChunkPos.z);
                    glBindVertexArray(chunkGL.vao);
                    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, chunkGL.numInstances);
                    glBindVertexArray(0);
                }
            }
        }
#endif//INST_MODE
#ifdef TRI_MODE

        voxelShaderTris.use();
        voxelShaderTris.setFloat("fogDistance", fogDistance);
        voxelShaderTris.setVec3("skyColor", skyColor.r, skyColor.g, skyColor.b);
        voxelShaderTris.setUInt("chunkSize", CHUNK_SIZE);
        voxelShaderTris.setMatrix4("viewProjection", view);
        voxelShaderTris.setInt("sampler", 0);
        voxelShaderTris.setVec3("camPos",
                                playerPos.pos.x + playerCam.posOffset.x - playerChunkPos.x * CHUNK_SIZE,
                                playerPos.pos.y + playerCam.posOffset.y - playerChunkPos.y * CHUNK_SIZE,
                                playerPos.pos.z + playerCam.posOffset.z - playerChunkPos.z * CHUNK_SIZE);

        auto renderableChunksTris = registry.group<Components::ChunkPosition, Components::ChunkOpenGLTriVer, Components::ChunkFog>(); // was view
        for (auto c : renderableChunksTris) {
            auto[chunkPos, chunkGL, chunkFog] = renderableChunksTris.get<Components::ChunkPosition, Components::ChunkOpenGLTriVer, Components::ChunkFog>(c);

            bool needUpdateVal = true;
            if (chunkFog.xMin == needUpdateVal || chunkFog.xMax == needUpdateVal || chunkFog.yMin == needUpdateVal || chunkFog.yMax == needUpdateVal || chunkFog.zMin == needUpdateVal || chunkFog.zMax == needUpdateVal) {
                updateChunkLocalFog(registry, c);
            }
        }

        int drawCalls = 0;
        int triangles = 0;
        // render visible chunks
        for (auto c : renderableChunksTris) {
            auto [chunkPos, chunkGL, chunkFog] = renderableChunksTris.get<Components::ChunkPosition, Components::ChunkOpenGLTriVer, Components::ChunkFog>(c);

            if (chunkGL.numTriangles > 0) {
                glm::vec4 chunkPoint((chunkPos.x - playerChunkPos.x) * CHUNK_SIZE + CHUNK_SIZE / 2, (chunkPos.y - playerChunkPos.y) * CHUNK_SIZE + CHUNK_SIZE / 2, (chunkPos.z - playerChunkPos.z) * CHUNK_SIZE + CHUNK_SIZE / 2, 1);
                glm::vec4 chunkPointXYCut = chunkPoint;
                chunkPoint = frustumForZCut * chunkPoint;
                chunkPoint = chunkPoint / chunkPoint.w;
                chunkPointXYCut = frustumForXYCut * chunkPointXYCut;
                chunkPointXYCut = chunkPointXYCut / chunkPointXYCut.w;
                if (chunkPoint.z > 0 && chunkPointXYCut.x > -1 && chunkPointXYCut.x < 1 && chunkPointXYCut.y > -1 && chunkPointXYCut.y < 1) {
//                    std::cout << chunkPoint.x << " " << chunkPoint.y << " " << chunkPoint.z << " " << chunkPoint.w << std::endl;
                    voxelShaderTris.setVec3i("chunkPos", chunkPos.x - playerChunkPos.x, chunkPos.y - playerChunkPos.y, chunkPos.z - playerChunkPos.z);

//                    std::cout << chunkFog.xMin << std::endl;
//                    std::cout << chunkFog.xMax << std::endl;
//                    std::cout << chunkFog.yMin << std::endl;
//                    std::cout << chunkFog.yMax << std::endl;
//                    std::cout << chunkFog.zMin << std::endl;
//                    std::cout << chunkFog.zMax << std::endl;
                    voxelShaderTris.setFloat("xMinFog", chunkFog.xMin ? 0.f: 1.f);
                    voxelShaderTris.setFloat("xMaxFog", chunkFog.xMax ? 0.f: 1.f);
                    voxelShaderTris.setFloat("yMinFog", chunkFog.yMin ? 0.f: 1.f);
                    voxelShaderTris.setFloat("yMaxFog", chunkFog.yMax ? 0.f: 1.f);
                    voxelShaderTris.setFloat("zMinFog", chunkFog.zMin ? 0.f: 1.f);
                    voxelShaderTris.setFloat("zMaxFog", chunkFog.zMax ? 0.f: 1.f);
                    glBindVertexArray(chunkGL.vao);
                    glDrawArrays(GL_TRIANGLES, 0, chunkGL.numTriangles);
                    ++drawCalls;
                    triangles += chunkGL.numTriangles;
                    glBindVertexArray(0);
                }
            }
        }

//        std::cout << "Draw calls: " << drawCalls << " Tris: " << triangles << std::endl;
#endif//TRI_MODE
    }

    Profiler::getInstance()->end("chunks_render");
}

bool ChunkManagement::chunkHasAllNeighborData(entt::registry &registry, Components::ChunkPosition &chunkPosition) {
    bool good = true;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            for (int z = -1; z <= 1; ++z) {
                if (!chunkHasData(registry, chunkPosition.x + x, chunkPosition.y + y, chunkPosition.z + z)) {
                    good = false;
                    goto loopExit;
                }
            }
        }
    }
    loopExit:
    return good;
}

bool ChunkManagement::chunkHasData(entt::registry &registry, int xChunk, int yChunk, int zChunk) {
    auto key = chunkPositionToKey(xChunk, yChunk, zChunk);
    auto found = chunkKeyToChunkEntity.find(key);
    // if a chunk with this position is not already here, continue making the new chunk entity
    if (found != chunkKeyToChunkEntity.end()) {
        auto chunkEntity = chunkKeyToChunkEntity[key];
        auto &chunkStatus = registry.get<Components::ChunkStatus>(chunkEntity);
        if (chunkStatus.status == Components::ChunkStatusEnum::GENERATED_OR_LOADED ||
                chunkStatus.status == Components::ChunkStatusEnum::MESH_GENERATED ||
                chunkStatus.status == Components::ChunkStatusEnum::MESH_BUFFERED) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void ChunkManagement::genVboVaoAndBufferTris(entt::registry &registry, entt::entity chunkEntity) {
    registry.emplace<Components::ChunkOpenGLTriVer>(chunkEntity);
    auto [gl, mesh] = registry.get<Components::ChunkOpenGLTriVer, Components::ChunkMeshDataTris>(chunkEntity);
    gl.numTriangles = mesh.tris->size() / NUM_BYTES_PER_VERTEX;

    glGenVertexArrays(1, &gl.vao);
    glGenBuffers(1, &gl.vbo);

    glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.tris->size() * sizeof(unsigned char), mesh.tris->data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(gl.vao);

    glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
    // x y z
    glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)0);
    // tex
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)(3 * sizeof(unsigned char)));
    // xTex yTex
    glVertexAttribIPointer(2, 2, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)(4 * sizeof(unsigned char)));
    // brightness
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)(6 * sizeof(unsigned char)));
    // norm
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)(7 * sizeof(unsigned char)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // delete mesh data as it is no longer needed

    if (mesh.tris != nullptr) {
        delete mesh.tris;
        mesh.tris = nullptr;
    }
    registry.remove<Components::ChunkMeshDataTris>(chunkEntity);
}

void ChunkManagement::fixGrass(blockid *chunkData, std::vector<blockid*>* neighborChunks) {
    for (int z = 0; z < CHUNK_SIZE; z++) for (int y = 0; y < CHUNK_SIZE - 1; y++) for (int x = 0; x < CHUNK_SIZE; x++) {
                auto mat = Components::chunkDataGet(chunkData, x, (y + 1), z);
                if (mat == 1 || mat == 2) {
                    (chunkData)[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] = 2;
                }
            }
    for (int z = 0; z < CHUNK_SIZE; z++) for (int x = 0; x < CHUNK_SIZE; x++) {
            int y = CHUNK_SIZE - 1;
            auto mat = Components::chunkDataGet(neighborChunks, x, (y + 1), z);
            if (mat == 1 || mat == 2) {
                (chunkData)[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] = 2;
            }
        }
}

void ChunkManagement::calculateBrightness(unsigned char *bVals, blockid *chunkData,
                                          std::vector<blockid *> *neighborChunks) {
    const int aoWidth = 1;
    const int aoHeight = 5;
//#pragma omp parallel for
    for (int z = 0; z < CHUNK_SIZE; ++z) {
        for(int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                if (x >= aoWidth && x < CHUNK_SIZE - aoWidth && z >= aoWidth && z < CHUNK_SIZE - aoWidth && y < CHUNK_SIZE - aoHeight) {
                    for (int lz = -aoWidth; lz <= aoWidth; ++lz) for (int ly = 1; ly <= aoHeight; ++ly) for (int lx = -aoWidth; lx <= aoWidth; ++lx)  {
                                bVals[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] += Components::chunkDataGet(chunkData, x + lx, y + ly, z + lz) == 0 ? 1 : 0;
                            }
                } else {
                    for (int lz = -aoWidth; lz <= aoWidth; ++lz) for (int ly = 1; ly <= aoHeight; ++ly) for (int lx = -aoWidth; lx <= aoWidth; ++lx)  {
                                bVals[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] += Components::chunkDataGet(neighborChunks, x + lx, y + ly, z + lz) == 0 ? 1 : 0;
                            }
                }
//                bVals[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] *= 0.25f;
            }
        }
    }
//    for (int i = 0; i < VOXELS_PER_CHUNK; ++i) {
//        bVals[i] = 45;
//    }
}

void ChunkManagement::generateMeshInstTris(volatile Components::ChunkStatusEnum* chunkStatus,
                                   blockid* chunkData, std::vector<unsigned char>* tris,
                                   std::vector<blockid*>* neighborChunks) {

    bool meshed[VOXELS_PER_CHUNK] = {false};
    unsigned char bVals[VOXELS_PER_CHUNK] = {0};

    fixGrass(chunkData, neighborChunks);
    calculateBrightness(bVals, chunkData, neighborChunks);

    for (int z = 0; z < CHUNK_SIZE; ++z) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                if (!meshed[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE]) {
                    auto material = Components::chunkDataGet(chunkData, x, y, z);
                    auto brightness = bVals[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE];
//                    bool canUseCheaperVoxelFun = (x > 0 && x < CHUNK_SIZE - 1 && y > 0 && y < CHUNK_SIZE - 1 && z > 0 && z < CHUNK_SIZE - 1);

                    if (material != 0 && Components::voxelIsTouchingAirWithNeighbors(neighborChunks, x, y, z)) {
                        int xSize = 0;
                        int ySize = 1;
                        int zSize = 1;

                        int xCurrent = x;

                        while (xCurrent < CHUNK_SIZE &&
                               !meshed[xCurrent + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] &&
                               Components::chunkDataGet(chunkData, xCurrent, y, z) == material &&
                               bVals[xCurrent + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] == brightness &&
                               Components::voxelIsTouchingAirWithNeighbors(neighborChunks, xCurrent, y, z)) {
                            ++xCurrent;
                            ++xSize;
                        }
                        for (int yl = y + 1; yl < CHUNK_SIZE; ++yl) {
                            for (int xl = x; xl < x + xSize; ++xl) {
                                if (!meshed[xl + yl * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] &&
                                    Components::chunkDataGet(chunkData, xl, yl, z) == material &&
                                    bVals[xl + yl * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] == brightness &&
                                    Components::voxelIsTouchingAirWithNeighbors(neighborChunks, xl, yl, z)) {
                                } else {
                                    goto exitLoops;
                                }
                            }
                            ++ySize;
                        }
                        exitLoops:

                        for (int zl = z + 1; zl < CHUNK_SIZE; ++zl) {
                            for (int yl = y; yl < y + ySize; ++yl) {
                                for (int xl = x; xl < x + xSize; ++xl) {
                                    if (!meshed[xl + yl * CHUNK_SIZE + zl * CHUNK_SIZE * CHUNK_SIZE] &&
                                        Components::chunkDataGet(chunkData, xl, yl, zl) == material &&
                                        bVals[xl + yl * CHUNK_SIZE + zl * CHUNK_SIZE * CHUNK_SIZE] == brightness &&
                                        Components::voxelIsTouchingAirWithNeighbors(neighborChunks, xl, yl, zl)) {
                                    } else {
                                        goto exitLoops2;
                                    }
                                }
                            }
                            ++zSize;
                        }
                        exitLoops2:

                        for (int zl = z; zl < z + zSize; ++zl) {
                            for (int yl = y; yl < y + ySize; ++yl) {
                                for (int xl = x; xl < x + xSize; ++xl) {
                                    meshed[xl + yl * CHUNK_SIZE + zl * CHUNK_SIZE * CHUNK_SIZE] = true;
                                }
                            }
                        }

                        // determine textures for faces
                        unsigned char topTexture = 0;
                        unsigned char sideTexture = 0;
                        unsigned char bottomTexture = 0;

                        switch (material) {
                            case 1:
                                topTexture = 0;
                                sideTexture = 1;
                                bottomTexture = 2;
                                break;
//                            case 2:
//                                topTexture = 2;
//                                sideTexture = 2;
//                                bottomTexture = 2;
//                                break;
//                            case 3:
//                                topTexture = 3;
//                                sideTexture = 3;
//                                bottomTexture = 3;
//                                break;

                            default:
                                topTexture = material;
                                sideTexture = material;
                                bottomTexture = material;
                                break;
                        }

                        bool visible = false;
                        // top face
                        for (int zl = z; zl < z + zSize; ++zl) {
                            for (int xl = x; xl < x + xSize; ++xl) {
                                if (Components::chunkDataGet(neighborChunks, xl, y + ySize, zl) == 0) {
                                    visible = true;
                                    goto topFaceBreak;
                                }
                            }
                        }
                        topFaceBreak:
                        if (visible) {
                            // front tri
                            makeVertex(*tris, x, y + ySize, z + zSize, topTexture, 0, zSize, brightness, 4);
                            makeVertex(*tris, x + xSize, y + ySize, z + zSize, topTexture, xSize, zSize, brightness, 4);
                            makeVertex(*tris, x + xSize, y + ySize, z, topTexture, xSize, 0, brightness, 4);

                            // back tri
                            makeVertex(*tris, x, y + ySize, z + zSize, topTexture, 0, zSize, brightness, 4);
                            makeVertex(*tris, x + xSize, y + ySize, z, topTexture, xSize, 0, brightness, 4);
                            makeVertex(*tris, x, y + ySize, z, topTexture, 0, 0, brightness, 4);
                        }

                        visible = false;
                        // bottom face
                        for (int zl = z; zl < z + zSize; ++zl) {
                            for (int xl = x; xl < x + xSize; ++xl) {
                                if (Components::chunkDataGet(neighborChunks, xl, y - 1, zl) == 0) {
                                    visible = true;
                                    goto bottomFaceBreak;
                                }
                            }
                        }
                        bottomFaceBreak:
                        if (visible) {
                            makeVertex(*tris, x + xSize, y, z, bottomTexture, xSize, 0, brightness * BOTTOM_BRIGHTNESS_MULT, 5);
                            makeVertex(*tris, x + xSize, y, z + zSize, bottomTexture, xSize, zSize, brightness * BOTTOM_BRIGHTNESS_MULT, 5);
                            makeVertex(*tris, x, y, z + zSize, bottomTexture, 0, zSize, brightness * BOTTOM_BRIGHTNESS_MULT, 5);

                            makeVertex(*tris, x, y, z, bottomTexture, 0, 0, brightness * BOTTOM_BRIGHTNESS_MULT, 5);
                            makeVertex(*tris, x + xSize, y, z, bottomTexture, xSize, 0, brightness * BOTTOM_BRIGHTNESS_MULT, 5);
                            makeVertex(*tris, x, y, z + zSize, bottomTexture, 0, zSize, brightness * BOTTOM_BRIGHTNESS_MULT, 5);
                        }

                        visible = false;
                        // front face
                        for (int yl = y; yl < y + ySize; ++yl) {
                            for (int xl = x; xl < x + xSize; ++xl) {
                                if (Components::chunkDataGet(neighborChunks, xl, yl, z + zSize) == 0) {
                                    visible = true;
                                    goto frontFaceBreak;
                                }
                            }
                        }
                        frontFaceBreak:
                        if (visible) {
                            makeVertex(*tris, x, y, z + zSize, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 0);
                            makeVertex(*tris, x + xSize, y, z + zSize, sideTexture, xSize, ySize, brightness * SIDE_BRIGHTNESS_MULT, 0);
                            makeVertex(*tris, x + xSize, y + ySize, z + zSize, sideTexture, xSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 0);

                            makeVertex(*tris, x, y, z + zSize, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 0);
                            makeVertex(*tris, x + xSize, y + ySize, z + zSize, sideTexture, xSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 0);
                            makeVertex(*tris, x, y + ySize, z + zSize, sideTexture, 0, 0, brightness * SIDE_BRIGHTNESS_MULT, 0);
                        }

                        visible = false;
                        // right face
                        for (int zl = z; zl < z + zSize; ++zl) {
                            for (int yl = y; yl < y + ySize; ++yl) {
                                if (Components::chunkDataGet(neighborChunks, x + xSize, yl, zl) == 0) {
                                    visible = true;
                                    goto rightFaceBreak;
                                }
                            }
                        }
                        rightFaceBreak:
                        if (visible) {
                            makeVertex(*tris, x + xSize, y, z + zSize, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 1);
                            makeVertex(*tris, x + xSize, y, z, sideTexture, zSize, ySize, brightness * SIDE_BRIGHTNESS_MULT, 1);
                            makeVertex(*tris, x + xSize, y + ySize, z, sideTexture, zSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 1);

                            makeVertex(*tris, x + xSize, y, z + zSize, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 1);
                            makeVertex(*tris, x + xSize, y + ySize, z, sideTexture, zSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 1);
                            makeVertex(*tris, x + xSize, y + ySize, z + zSize, sideTexture, 0, 0, brightness * SIDE_BRIGHTNESS_MULT, 1);
                        }

                        visible = false;
                        // back face
                        for (int yl = y; yl < y + ySize; ++yl) {
                            for (int xl = x; xl < x + xSize; ++xl) {
                                if (Components::chunkDataGet(neighborChunks, xl, yl, z - 1) == 0) {
                                    visible = true;
                                    goto backFaceBreak;
                                }
                            }
                        }
                        backFaceBreak:
                        if (visible) {
                            makeVertex(*tris, x + xSize, y + ySize, z, sideTexture, xSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 0);
                            makeVertex(*tris, x + xSize, y, z, sideTexture, xSize, ySize, brightness * SIDE_BRIGHTNESS_MULT, 0);
                            makeVertex(*tris, x, y, z, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 0);

                            makeVertex(*tris, x, y + ySize, z, sideTexture, 0, 0, brightness * SIDE_BRIGHTNESS_MULT, 0);
                            makeVertex(*tris, x + xSize, y + ySize, z, sideTexture, xSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 0);
                            makeVertex(*tris, x, y, z, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 0);
                        }

                        visible = false;
                        // left face
                        for (int zl = z; zl < z + zSize; ++zl) {
                            for (int yl = y; yl < y + ySize; ++yl) {
                                if (Components::chunkDataGet(neighborChunks, x - 1, yl, zl) == 0) {
                                    visible = true;
                                    goto leftFaceBreak;
                                }
                            }
                        }
                        leftFaceBreak:
                        if (visible) {
                            makeVertex(*tris, x, y + ySize, z, sideTexture, zSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 1);
                            makeVertex(*tris, x, y, z, sideTexture, zSize, ySize, brightness * SIDE_BRIGHTNESS_MULT, 1);
                            makeVertex(*tris, x, y, z + zSize, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 1);

                            makeVertex(*tris, x, y + ySize, z + zSize, sideTexture, 0, 0, brightness * SIDE_BRIGHTNESS_MULT, 1);
                            makeVertex(*tris, x, y + ySize, z, sideTexture, zSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 1);
                            makeVertex(*tris, x, y, z + zSize, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 1);
                        }
                    }
                }
            }
        }
    }


    // after generating, change state
    *chunkStatus = Components::ChunkStatusEnum::MESH_GENERATED;
//    std::cout << "Done generating mesh" << std::endl;
    // also, i dont need that neighborChunks vector anymore. dont wanna leak memory :>
    neighborChunks->clear();
    delete neighborChunks;
    neighborChunks = nullptr;
    chunksCurrentlyMeshing--;
}

void ChunkManagement::makeVertex(std::vector<unsigned char> &mesh, unsigned char x, unsigned char y, unsigned char z,
                                 unsigned char texture, unsigned char xTex, unsigned char yTex,
                                 unsigned char brightness, unsigned char normal) {
    mesh.emplace_back(x);
    mesh.emplace_back(y);
    mesh.emplace_back(z);
    mesh.emplace_back(texture);
    mesh.emplace_back(xTex);
    mesh.emplace_back(yTex);
    mesh.emplace_back(brightness);
    mesh.emplace_back(normal);
}


void ChunkManagement::generateMeshGreedy(volatile Components::ChunkStatusEnum* chunkStatus,
                                              blockid* chunkData, std::vector<unsigned char>* tris,
                                              std::vector<blockid*>* neighborChunks) {

    bool meshed[CHUNK_SIZE * CHUNK_SIZE] = {false};
    unsigned char bVals[VOXELS_PER_CHUNK] = {0};

    fixGrass(chunkData, neighborChunks);
    calculateBrightness(bVals, chunkData, neighborChunks);

    int x = 0;
    int y = 0;
    int z = 0;
    int xSize = 1;
    int ySize = 1;
    int zSize = 1;
    int brightness = 45;
    int sideTexture = 0;


    makeVertex(*tris, x + xSize, y, z + zSize, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 1);
    makeVertex(*tris, x + xSize, y, z, sideTexture, zSize, ySize, brightness * SIDE_BRIGHTNESS_MULT, 1);
    makeVertex(*tris, x + xSize, y + ySize, z, sideTexture, zSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 1);

    makeVertex(*tris, x + xSize, y, z + zSize, sideTexture, 0, ySize, brightness * SIDE_BRIGHTNESS_MULT, 1);
    makeVertex(*tris, x + xSize, y + ySize, z, sideTexture, zSize, 0, brightness * SIDE_BRIGHTNESS_MULT, 1);
    makeVertex(*tris, x + xSize, y + ySize, z + zSize, sideTexture, 0, 0, brightness * SIDE_BRIGHTNESS_MULT, 1);


    // after generating, change state
    *chunkStatus = Components::ChunkStatusEnum::MESH_GENERATED;
//    std::cout << "Done generating mesh" << std::endl;
    // also, i dont need that neighborChunks vector anymore. dont wanna leak memory :>
    neighborChunks->clear();
    delete neighborChunks;
    neighborChunks = nullptr;
    chunksCurrentlyMeshing--;
}

float ChunkManagement::getFogDistance() {
    return fogDistance;
}

void ChunkManagement::updateChunkLocalFog(entt::registry &registry, entt::entity chunk) {
    if (registry.get<Components::ChunkStatus>(chunk).status == Components::ChunkStatusEnum::MESH_BUFFERED) {
        auto chunkPos = registry.get<Components::ChunkPosition>(chunk);
        auto &chunkFog = registry.get<Components::ChunkFog>(chunk);

//        chunkFog.xMin = true;
//        chunkFog.xMax = true;
//        chunkFog.yMin = true;
//        chunkFog.yMin = false;
//        chunkFog.zMin = false;
//        chunkFog.zMax = false;
        chunkFog.xMin = isChunkFullyLoaded(registry, chunkPos.x - 1, chunkPos.y, chunkPos.z);
        chunkFog.xMax = isChunkFullyLoaded(registry, chunkPos.x + 1, chunkPos.y, chunkPos.z);
        chunkFog.yMin = isChunkFullyLoaded(registry, chunkPos.x, chunkPos.y - 1, chunkPos.z);
        chunkFog.yMax = isChunkFullyLoaded(registry, chunkPos.x, chunkPos.y + 1, chunkPos.z);
        chunkFog.zMin = isChunkFullyLoaded(registry, chunkPos.x, chunkPos.y, chunkPos.z - 1);
        chunkFog.zMax = isChunkFullyLoaded(registry, chunkPos.x, chunkPos.y, chunkPos.z + 1);

    }

}

void ChunkManagement::updateNeighborChunksLocalFog(entt::registry &registry, Components::ChunkPosition &chunkPos) {
    if (chunkKeyToChunkEntity.contains(chunkPositionToKey(chunkPos.x, chunkPos.y, chunkPos.z))) {
        updateChunkLocalFog(registry, chunkKeyToChunkEntity[chunkPositionToKey(chunkPos.x, chunkPos.y, chunkPos.z)]);
    }
    if (chunkKeyToChunkEntity.contains(chunkPositionToKey(chunkPos.x - 1, chunkPos.y, chunkPos.z))) {
        updateChunkLocalFog(registry, chunkKeyToChunkEntity[chunkPositionToKey(chunkPos.x - 1, chunkPos.y, chunkPos.z)]);
    }
    if (chunkKeyToChunkEntity.contains(chunkPositionToKey(chunkPos.x + 1, chunkPos.y, chunkPos.z))) {
        updateChunkLocalFog(registry, chunkKeyToChunkEntity[chunkPositionToKey(chunkPos.x + 1, chunkPos.y, chunkPos.z)]);
    }
    if (chunkKeyToChunkEntity.contains(chunkPositionToKey(chunkPos.x, chunkPos.y - 1, chunkPos.z))) {
        updateChunkLocalFog(registry, chunkKeyToChunkEntity[chunkPositionToKey(chunkPos.x, chunkPos.y - 1, chunkPos.z)]);
    }
    if (chunkKeyToChunkEntity.contains(chunkPositionToKey(chunkPos.x, chunkPos.y + 1, chunkPos.z))) {
        updateChunkLocalFog(registry, chunkKeyToChunkEntity[chunkPositionToKey(chunkPos.x, chunkPos.y + 1, chunkPos.z)]);
    }
    if (chunkKeyToChunkEntity.contains(chunkPositionToKey(chunkPos.x, chunkPos.y, chunkPos.z - 1))) {
        updateChunkLocalFog(registry, chunkKeyToChunkEntity[chunkPositionToKey(chunkPos.x, chunkPos.y, chunkPos.z - 1)]);
    }
    if (chunkKeyToChunkEntity.contains(chunkPositionToKey(chunkPos.x, chunkPos.y, chunkPos.z + 1))) {
        updateChunkLocalFog(registry, chunkKeyToChunkEntity[chunkPositionToKey(chunkPos.x, chunkPos.y, chunkPos.z + 1)]);
    }
}

void ChunkManagement::computeSortedChunkPositionLoadSphere() {
    int cRadius = ceil(CHUNK_LOAD_RADIUS / (double) CHUNK_SIZE);
    for (int x = -cRadius; x <= cRadius; ++x) {
        for (int y = -cRadius; y <= cRadius; ++y) {
            for (int z = -cRadius; z <= cRadius; ++z) {
                double length2 = worldPosChunkPosDist2(x, y, z, .0, .0, .0);
                if (length2 < CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS) {
                    // push into array
                    sortedChunkPositionLoadSphere.emplace_back(Components::ChunkPosition{x, y, z});
                }
            }
        }
    }

    // sort it
    std::sort(sortedChunkPositionLoadSphere.begin(), sortedChunkPositionLoadSphere.end(), chunkPosCompareFun);
}