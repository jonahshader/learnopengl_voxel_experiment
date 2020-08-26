//
// Created by Jonah on 8/10/2020.
//

#include "ChunkManagement.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ecs/Components.h>
#include <thread>
#include <iostream>

entt::registry* ChunkManagement::chunkCompareRegistry = nullptr;
Components::Position ChunkManagement::chunkComparePos = {glm::dvec3(0.0)};

ChunkManagement::ChunkManagement(const char* vertexPath, const char* fragmentPath) :
        voxelShader(vertexPath, fragmentPath),
        noise()
{
    noise.SetNoiseType(FastNoise::SimplexFractal);
}


std::string ChunkManagement::chunkPositionToKey(int xChunk, int yChunk, int zChunk) {
    return "x" + std::to_string(xChunk) + "y" + std::to_string(yChunk) + "z" + std::to_string(zChunk);
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
        // find player location
        auto &playerPos = registry.get<Components::Position>(player);
        auto &playerChunkPos = registry.get<Components::ChunkPosition>(player);
        // calculate load bounds for the chunk generate loop
        int cRadius = ceil(CHUNK_LOAD_RADIUS / (double) CHUNK_SIZE);
        int xCMin = playerChunkPos.x - cRadius;
        int xCMax = playerChunkPos.x + cRadius;
        int yCMin = playerChunkPos.y - cRadius;
        int yCMax = playerChunkPos.y + cRadius;
        int zCMin = playerChunkPos.z - cRadius;
        int zCMax = playerChunkPos.z + cRadius;

        for (int x = xCMin; x <= xCMax; x++) {
            for (int y = yCMin; y <= yCMax; y++) {
                for (int z = zCMin; z <= zCMax; z++) {
                    // try to create an entity for every chunk here if they are in range
                    double dist = worldPosChunkPosDist(x, y, z, playerPos.pos.x, playerPos.pos.y, playerPos.pos.z);
//                    std::cout << "dist: " << dist << std::endl;
                    if (dist < CHUNK_LOAD_RADIUS) {
                        // try create
                        tryCreateChunk(registry, x, y, z);
                    }
                }
            }
        }


        // sort chunks by closest to furthest
        // first need to set variables used by the comparor function
        chunkComparePos = playerPos;
        chunkCompareRegistry = &registry;
        // perform sort
        std::sort(chunks.begin(), chunks.end(), chunkCompareFun);
        // progress chunks through stages
        int buffers = 0;
        int generates = 0;
        std::vector<entt::entity> chunksSuccessfullyRemoved;
        for (auto chunkEntity : chunks) {
            auto [chunkStatus, chunkPosition] = registry.get<Components::ChunkStatus, Components::ChunkPosition>(chunkEntity);

            // check if this chunk is in range
            // as long as these chunks are within the unload radius, they can continue loading

            if (chunkStatus.markedForRemoval) {
//                std::cout << "stat of chunk marked for removal: " << chunkStatus.status << std::endl;
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
            } else if (worldPosChunkPosDist(chunkPosition, playerPos) < CHUNK_UNLOAD_RADIUS) {
                // figure out the status of this chunk is and make it move to the next stage
                switch (chunkStatus.status) {
                    case Components::ChunkStatusEnum::NEW:
                        if (generates < MAX_GENERATES_PER_FRAME) {
                            registry.emplace<Components::ChunkData>(chunkEntity); // add chunk data
                            chunkStatus.status = Components::ChunkStatusEnum::GENERATING_OR_LOADING;
                            auto &chunkData = registry.get<Components::ChunkData>(chunkEntity);
//                            threadLaunchPointer = new std::thread(generateChunk, std::ref(chunkStatus), std::ref(chunkPosition), std::ref(chunkData));

//                            std::thread coolThread(generateChunk, std::ref(chunkStatus), std::ref(chunkPosition), std::ref(chunkData), std::ref(noise));
//                            coolThread.detach();
//                            threadLaunchPointer->detach();
                            generateChunk(std::ref(chunkStatus), std::ref(chunkPosition), std::ref(chunkData), std::ref(noise));
                            ++generates;
                        }
                        break;
                    case Components::ChunkStatusEnum::GENERATED_OR_LOADED:
                    {
                        // add mesh data
                        registry.emplace<Components::ChunkMeshData>(chunkEntity);
                        chunkStatus.status = Components::ChunkStatusEnum::MESH_GENERATING;
                        auto [chunkData, chunkMeshData] = registry.get<Components::ChunkData, Components::ChunkMeshData>(chunkEntity);
//                        new std::thread(generateMesh, std::ref(chunkStatus), std::ref(chunkPosition), std::ref(chunkData), std::ref(chunkMeshData));
                        generateMesh(chunkStatus, chunkPosition, chunkData, chunkMeshData);
                    }
                        break;
                    case Components::ChunkStatusEnum::MESH_GENERATED:
                        if (buffers < MAX_BUFFERS_PER_FRAME) {
                            genVboVaoAndBuffer(registry, chunkEntity);
                            chunkStatus.status = Components::ChunkStatusEnum::MESH_BUFFERED;
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

        // erase chunks from chunks vector
        for (auto cToRemove : chunksSuccessfullyRemoved) {
            for (int j = 0; j < chunks.size(); j++) {
                if (chunks[j] == cToRemove) {
                    chunks.erase(chunks.begin() + j);
                    j = -1;
                }
            }
        }

    }
}

void ChunkManagement::tryCreateChunk(entt::registry &registry, int xChunk, int yChunk, int zChunk) {
    // check if this chunk already exists
    auto key = chunkPositionToKey(xChunk, yChunk, zChunk);
    auto found = chunkKeyToChunkEntity.find(key);
    // if a chunk with this position is not already here, continue making the new chunk entity
    if (found == chunkKeyToChunkEntity.end()) {
        auto entity = registry.create();
        registry.emplace<Components::ChunkStatus>(entity, Components::ChunkStatusEnum::NEW, false);
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
    // try remove gl
    if (registry.has<Components::ChunkOpenGL>(chunkEntity)) {
        auto &chunkgl = registry.get<Components::ChunkOpenGL>(chunkEntity);
        glDeleteVertexArrays(1, &chunkgl.vao);
        glDeleteBuffers(1, &chunkgl.vbo);
    }
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

double ChunkManagement::worldPosChunkPosDist(int xc, int yc, int zc, double x, double y, double z) {
    double xx = xc * CHUNK_SIZE + (CHUNK_SIZE / 2.);
    double yy = yc * CHUNK_SIZE + (CHUNK_SIZE / 2.);
    double zz = zc * CHUNK_SIZE + (CHUNK_SIZE / 2.);

    return sqrt(pow(xx-x, 2) + pow(yy-y, 2) + pow(zz-z, 2));
}

bool ChunkManagement::chunkCompareFun(entt::entity chunk1, entt::entity chunk2) {
    double dist1 = worldPosChunkPosDist(chunkCompareRegistry->get<Components::ChunkPosition>(chunk1), chunkComparePos);
    double dist2 = worldPosChunkPosDist(chunkCompareRegistry->get<Components::ChunkPosition>(chunk2), chunkComparePos);
    return dist1 < dist2;
}

void ChunkManagement::generateChunk(Components::ChunkStatus &chunkStatus, Components::ChunkPosition &chunkPosition,
                                    Components::ChunkData &chunkData, FastNoise &n) {

//    float* noiseSet = noise->GetSimplexFractalSet(chunkPosition.x * CHUNK_SIZE,
//                                                  chunkPosition.y * CHUNK_SIZE,
//                                                  chunkPosition.z * CHUNK_SIZE,
//                                                  chunkPosition.x * CHUNK_SIZE + CHUNK_SIZE,
//                                                  chunkPosition.y * CHUNK_SIZE + CHUNK_SIZE,
//                                                  chunkPosition.z * CHUNK_SIZE + CHUNK_SIZE);

    // convert float data to voxel data
#pragma omp parallel for
    for (int i = 0; i < VOXELS_PER_CHUNK; ++i) {
        // convert z y x to x y z indices for FastNoiseSIMD library
        int x = i % CHUNK_SIZE;
        int y = (i / CHUNK_SIZE) % CHUNK_SIZE;
        int z = (i / (CHUNK_SIZE * CHUNK_SIZE));
        x += chunkPosition.x * CHUNK_SIZE;
        y += chunkPosition.y * CHUNK_SIZE;
        z += chunkPosition.z * CHUNK_SIZE;
        float noiseOut = n.GetNoise(x, y, z);
//        std::cout << noiseOut << std::endl;
        chunkData.data[i] = noiseOut > 0.0 ? 1 : 0;
    }

    //update state to GENERATED_OR_LOADED
    chunkStatus.status = Components::ChunkStatusEnum::GENERATED_OR_LOADED;
//    std::cout << "Done generating chunk" << std::endl;
}

void ChunkManagement::generateMesh(Components::ChunkStatus &chunkStatus, Components::ChunkPosition &chunkPosition,
                                   Components::ChunkData &chunkData, Components::ChunkMeshData &chunkMeshData) {
    for (int i = 0; i < CHUNK_SIZE; i++) {
        // TODO might be just i instead of i + 1
        addSlice(chunkMeshData.mesh, chunkData, DataTypes::Axis::X, false, i + 1);
        addSlice(chunkMeshData.mesh, chunkData, DataTypes::Axis::X, true, i + 1);
        addSlice(chunkMeshData.mesh, chunkData, DataTypes::Axis::Y, false, i + 1);
        addSlice(chunkMeshData.mesh, chunkData, DataTypes::Axis::Y, true, i + 1);
        addSlice(chunkMeshData.mesh, chunkData, DataTypes::Axis::Z, false, i + 1);
        addSlice(chunkMeshData.mesh, chunkData, DataTypes::Axis::Z, true, i + 1);
    }

//    addSlice(chunkMeshData.mesh, chunkData, DataTypes::Axis::Z, false, 5);

//    Vertex v1{}, v2{}, v3{}, v4{};
//    v1.asNum = makeVertex(0, 0, 0,0, 0, 0, 0, 15);
//    v2.asNum = makeVertex(0, 32, 0, 0, 1, 0, 0, 15);
//    v3.asNum = makeVertex(32, 32, 0, 1, 1, 0, 0, 15);
//    v4.asNum = makeVertex(32, 0, 0, 1, 0, 0, 0, 15);
//
//    // top left tri
//    addVertexToMesh(chunkMeshData.mesh, v1, NUM_BYTES_PER_VERTEX);
//    addVertexToMesh(chunkMeshData.mesh, v2, NUM_BYTES_PER_VERTEX);
//    addVertexToMesh(chunkMeshData.mesh, v4, NUM_BYTES_PER_VERTEX);
//    // bottom right tri
//    addVertexToMesh(chunkMeshData.mesh, v2, NUM_BYTES_PER_VERTEX);
//    addVertexToMesh(chunkMeshData.mesh, v3, NUM_BYTES_PER_VERTEX);
//    addVertexToMesh(chunkMeshData.mesh, v4, NUM_BYTES_PER_VERTEX);


    // after generating, change state
    chunkStatus.status = Components::ChunkStatusEnum::MESH_GENERATED;
//    std::cout << "Done generating mesh" << std::endl;
}

void ChunkManagement::addSlice(std::vector<unsigned char> &mesh, Components::ChunkData &voxelData,
                              DataTypes::Axis axis, bool flipped, int slice) {
    // position to move through slice
    int x = 0;
    int y = 0;
    int z = 0;

    int xSolidLayerOffset = 0;
    int ySolidLayerOffset = 0;
    int zSolidLayerOffset = 0;

    int* majorAxis;
    int* minorAxis;
    int* normalAxis;

    switch (axis) {
        default:
        case DataTypes::X:
            x = slice;
            majorAxis = &z;
            minorAxis = &y;
            normalAxis = &x;
            xSolidLayerOffset = -1;
            break;
        case DataTypes::Y:
            y = slice;
//            majorAxis = &z;
//            minorAxis = &x;
            majorAxis = &x;
            minorAxis = &z;
            normalAxis = &y;
            ySolidLayerOffset = -1;
            break;
        case DataTypes::Z:
            z = slice;
            majorAxis = &y;
            minorAxis = &x;
            normalAxis = &z;
            zSolidLayerOffset = -1;
            break;
    }

    // if flipped, swap air solid layers
    if (flipped) {
        --(*normalAxis);
        xSolidLayerOffset *= -1;
        ySolidLayerOffset *= -1;
        zSolidLayerOffset *= -1;
    }

    bool record[CHUNK_SIZE][CHUNK_SIZE] = {{false}};

    // loop modifies major and minor axis, which is one of x y z
    // now i can just use x y z instead of doing logic in the for loops
    // to figure out what plane the algo is running on
    for (*majorAxis = 0; (*majorAxis) < CHUNK_SIZE; ++(*majorAxis)) {
        for (*minorAxis = 0; (*minorAxis) < CHUNK_SIZE; ++(*minorAxis)) {
            // if this spot has not been meshed yet,
            if (!record[*majorAxis][*minorAxis]) {
                auto material = Components::chunkDataGetAirBounds(voxelData, x + xSolidLayerOffset, y + ySolidLayerOffset, z + zSolidLayerOffset);
                // if this voxel is not air and the face is visible,
                if (material != 0 && Components::chunkDataGetAirBounds(voxelData, x, y, z) == 0) {
                    // build quad
                    int majorSize = 0; // "height"
                    int minorSize = 0; // "width"
                    // record current major minor axis values so that I can put them back later
                    // also used for generating vertices
                    int majorAxisStart = *majorAxis;
                    int minorAxisStart = *minorAxis;

                    // figure out how "wide" our quad will be (by width i mean minorSize)
                    while (Components::chunkDataGetAirBounds(voxelData, x + xSolidLayerOffset, y + ySolidLayerOffset, z + zSolidLayerOffset) == material
                    && !record[*majorAxis][*minorAxis]
                    && Components::chunkDataGetAirBounds(voxelData, x, y, z) == 0
                    && *minorAxis < CHUNK_SIZE) {
                        ++(*minorAxis);
                        ++minorSize;
                    }

                    ++majorSize;

                    for (*majorAxis = majorAxisStart + 1; *majorAxis < CHUNK_SIZE; ++(*majorAxis)) {
                        for (*minorAxis = minorAxisStart; *minorAxis < minorAxisStart + minorSize; ++(*minorAxis)) {

                            // if this piece was already meshed (already in the record)
                            // or the read layer is not the same material
                            // or the face is not visible,
                            // then break out of these loops
                            if (record[*majorAxis][*minorAxis]
                            || Components::chunkDataGetAirBounds(voxelData, x + xSolidLayerOffset, y + ySolidLayerOffset, z + zSolidLayerOffset) != material
                            || Components::chunkDataGetAirBounds(voxelData, x, y, z) != 0) {
                                goto exitLoops;
                            }
                        }
                        ++majorSize;
                    }
                    exitLoops:

                    // return old value to major and minor axis
                    *majorAxis = majorAxisStart;
                    *minorAxis = minorAxisStart;

//                    std::cout << "MAS " << majorAxisStart << " mAS " << minorAxisStart;
//                    std::cout << " Ms " << majorSize << " ms " << minorSize << std::endl;

                    // record meshed tiles
                    for (int yy = majorAxisStart; yy < majorAxisStart + majorSize; ++yy) {
                        for (int xx = minorAxisStart; xx < minorAxisStart + minorSize; ++xx) {
//                            std::cout << "M: " << *majorAxis << " m: " << *minorAxis << std::endl;
                            record[yy][xx] = true;
                        }
                    }



                    /**
                    name | bits | offset
                    x      6      0
                    y      6      6
                    z      6      12
                    tx     6      18
                    ty     6      24
                    n      3      30
                    tex    8      33
                    b      4      41
                     */

                    // i now have the stuff to make the quad

                    // make vertices counter clockwise
//                    Vertex v1{}, v2{}, v3{}, v4{};
//                    v1.asNum = 0;
//                    v2.asNum = 0;
//                    v3.asNum = 0;
//                    v4.asNum = 0;
                    unsigned char v1[NUM_BYTES_PER_VERTEX];
                    unsigned char v2[NUM_BYTES_PER_VERTEX];
                    unsigned char v3[NUM_BYTES_PER_VERTEX];
                    unsigned char v4[NUM_BYTES_PER_VERTEX];

                    unsigned char normalFlipInc = flipped ? 3 : 0;
                    switch (axis) {
                        case DataTypes::X: // major = z, minor = y
//                            std::cout << x << " " << y << " " << z << " " << majorSize << " " << minorSize << std::endl;
//                            v1.asNum = makeVertex(x, y, z, 0, 0, 0 + normalFlipInc, material - 1, 15);
//                            v2.asNum = makeVertex(x, y, z + majorSize, majorSize, 0, 0 + normalFlipInc, material - 1, 15);
//                            v3.asNum = makeVertex(x, y + minorSize, z + majorSize, majorSize, minorSize, 0 + normalFlipInc, material - 1, 15);
//                            v4.asNum = makeVertex(x, y + minorSize, z, 0, minorSize, 0 + normalFlipInc, material - 1, 15);
                            makeVertexArray(x, y, z, 0, 0, 0 + normalFlipInc, material - 1, 15, v1);
                            makeVertexArray(x, y, z + majorSize, majorSize, 0, 0 + normalFlipInc, material - 1, 15, v2);
                            makeVertexArray(x, y + minorSize, z + majorSize, majorSize, minorSize, 0 + normalFlipInc, material - 1, 15, v3);
                            makeVertexArray(x, y + minorSize, z, 0, minorSize, 0 + normalFlipInc, material - 1, 15, v4);
                            break;
                        case DataTypes::Y: // major = z, minor = x
//                            v1.asNum = makeVertex(x, y, z, 0, 0, 1 + normalFlipInc, material - 1, 15);
//                            v2.asNum = makeVertex(x, y, z + majorSize, majorSize, 0, 1 + normalFlipInc, material - 1, 15);
//                            v3.asNum = makeVertex(x + minorSize, y, z + majorSize, majorSize, minorSize, 1 + normalFlipInc, material - 1, 15);
//                            v4.asNum = makeVertex(x + minorSize, y, z, 0, minorSize, 1 + normalFlipInc, material - 1, 15);
                            makeVertexArray(x, y, z, 0, 0, 1 + normalFlipInc, material - 1, 15, v1);
                            makeVertexArray(x, y, z + majorSize, majorSize, 0, 1 + normalFlipInc, material - 1, 15, v2);
                            makeVertexArray(x + minorSize, y, z + majorSize, majorSize, minorSize, 1 + normalFlipInc, material - 1, 15, v3);
                            makeVertexArray(x + minorSize, y, z, 0, minorSize, 1 + normalFlipInc, material - 1, 15, v4);
                            break;
                        case DataTypes::Z: // major = y, minor = x
//                            v1.asNum = makeVertex(x, y, z, 0, 0, 2 + normalFlipInc, material - 1, 15);
//                            v2.asNum = makeVertex(x, y + majorSize, z, majorSize, 0, 2 + normalFlipInc, material - 1, 15);
//                            v3.asNum = makeVertex(x + minorSize, y + majorSize, z, majorSize, minorSize, 2 + normalFlipInc, material - 1, 15);
//                            v4.asNum = makeVertex(x + minorSize, y, z, 0, minorSize, 2 + normalFlipInc, material - 1, 15);
                            makeVertexArray(x, y, z, 0, 0, 2 + normalFlipInc, material - 1, 15, v1);
                            makeVertexArray(x, y + majorSize, z, majorSize, 0, 2 + normalFlipInc, material - 1, 15, v2);
                            makeVertexArray(x + minorSize, y + majorSize, z, majorSize, minorSize, 2 + normalFlipInc, material - 1, 15, v3);
                            makeVertexArray(x + minorSize, y, z, 0, minorSize, 2 + normalFlipInc, material - 1, 15, v4);
                            break;
                        default:
                            break;
                    }

                    // add quad
                    // if flipped, add clockwise instead of counter clockwise
//                    if (flipped) {
//                        // top left tri
//                        addVertexToMesh(mesh, v1, NUM_BYTES_PER_VERTEX);
//                        addVertexToMesh(mesh, v4, NUM_BYTES_PER_VERTEX);
//                        addVertexToMesh(mesh, v2, NUM_BYTES_PER_VERTEX);
//                        // bottom right tri
//                        addVertexToMesh(mesh, v2, NUM_BYTES_PER_VERTEX);
//                        addVertexToMesh(mesh, v4, NUM_BYTES_PER_VERTEX);
//                        addVertexToMesh(mesh, v3, NUM_BYTES_PER_VERTEX);
//                    } else {
//                        // top left tri
//                        addVertexToMesh(mesh, v1, NUM_BYTES_PER_VERTEX);
//                        addVertexToMesh(mesh, v2, NUM_BYTES_PER_VERTEX);
//                        addVertexToMesh(mesh, v4, NUM_BYTES_PER_VERTEX);
//                        // bottom right tri
//                        addVertexToMesh(mesh, v2, NUM_BYTES_PER_VERTEX);
//                        addVertexToMesh(mesh, v3, NUM_BYTES_PER_VERTEX);
//                        addVertexToMesh(mesh, v4, NUM_BYTES_PER_VERTEX);
//                    }
                    if (flipped) {
                        // top left tri
                        addVertexArrayToMesh(mesh, v1, NUM_BYTES_PER_VERTEX);
                        addVertexArrayToMesh(mesh, v4, NUM_BYTES_PER_VERTEX);
                        addVertexArrayToMesh(mesh, v2, NUM_BYTES_PER_VERTEX);
                        // bottom right tri
                        addVertexArrayToMesh(mesh, v2, NUM_BYTES_PER_VERTEX);
                        addVertexArrayToMesh(mesh, v4, NUM_BYTES_PER_VERTEX);
                        addVertexArrayToMesh(mesh, v3, NUM_BYTES_PER_VERTEX);
                    } else {
                        // top left tri
                        addVertexArrayToMesh(mesh, v1, NUM_BYTES_PER_VERTEX);
                        addVertexArrayToMesh(mesh, v2, NUM_BYTES_PER_VERTEX);
                        addVertexArrayToMesh(mesh, v4, NUM_BYTES_PER_VERTEX);
                        // bottom right tri
                        addVertexArrayToMesh(mesh, v2, NUM_BYTES_PER_VERTEX);
                        addVertexArrayToMesh(mesh, v3, NUM_BYTES_PER_VERTEX);
                        addVertexArrayToMesh(mesh, v4, NUM_BYTES_PER_VERTEX);
                    }
//                    todo: there is an optimization to be made here.
//                     minorAxis could be minorAxisStart + minorSize possibly
//                    *majorAxis = majorAxisStart;
//                    *minorAxis = minorAxisStart + minorSize;
                }
            }
        }
    }
}


unsigned long ChunkManagement::makeVertex(unsigned char x, unsigned char y, unsigned char z,
                                          unsigned char tx, unsigned char ty, unsigned char n,
                                          unsigned char tex, unsigned char b) {
//    return x | y << 6u | z << 12u | tx << 18u | ty << 24u | n << 30u | tex << 33ul | b << 41ul;
//    return x | y << 8 | z << 16 | tx << 24 | ty << 32 | n << 40 | tex << 48 | b << 56;
        return b | tex << 8 | n << 16 | ty << 24 | tx << 32 | z << 40 | y << 48 | x << 56;
//    return b | tex << 8 | n << 12 | ty << 15 | tx << 21 | z << 27 | y << 33 | x << 39;
//    return x | y << 6u | z << 12u | tx << 18u | ty << 24u;
}

void ChunkManagement::addVertexToMesh(std::vector<unsigned char> &mesh, Vertex vertex, int numBytesToCopy) {
//    for (int i = 0; i < numBytesToCopy; i++) {
//        mesh.push_back(vertex.asArray[i]);
//    }
//    for (unsigned int i = 0; i < numBytesToCopy; i++) {
//        mesh.push_back((vertex.asNum >> (8 * i)) & 0xFF);
//    }
    for (int i = numBytesToCopy; i >= 0; --i) {
        mesh.push_back(vertex.asArray[i]);
    }
}

void ChunkManagement::genVboVaoAndBuffer(entt::registry& registry, entt::entity chunkEntity) {
    registry.emplace<Components::ChunkOpenGL>(chunkEntity);
    auto [gl, mesh] = registry.get<Components::ChunkOpenGL, Components::ChunkMeshData>(chunkEntity);
    // set numTriangles for rendering later
    gl.numTriangles = mesh.mesh.size() / (NUM_BYTES_PER_VERTEX * 3);

    glGenVertexArrays(1, &gl.vao);
    glGenBuffers(1, &gl.vbo);

    glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.mesh.size() * sizeof(unsigned char), mesh.mesh.data(), GL_STATIC_DRAW);

    glBindVertexArray(gl.vao);
    /**
    name | bits | offset
    x      6      0
    y      6      6
    z      6      12
    tx     6      18
    ty     6      24
    n      3      30
    tex    8      33
    b      4      41
    */
//    // xyz
//    glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)0);
//    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)6);
//    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)12);
//    // tx ty n
//    glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)18);
//    glVertexAttribIPointer(4, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)24);
//    glVertexAttribIPointer(5, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)30);
//    // tex b
//    glVertexAttribIPointer(6, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)33);
//    glVertexAttribIPointer(7, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)41);

//    // xyz
//    glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)0);
//    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)8);
//    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)16);
//    // tx ty n
//    glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)24);
//    glVertexAttribIPointer(4, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)32);
//    glVertexAttribIPointer(5, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)40);
//    // tex b
//    glVertexAttribIPointer(6, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)48);
//    glVertexAttribIPointer(7, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)56);


    // xyz
    glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)0);
    // tx ty n
    glVertexAttribIPointer(1, 2, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)24);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)40);
    // tex b
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)48);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_BYTE, NUM_BYTES_PER_VERTEX * sizeof(unsigned char), (void*)56);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
//    glEnableVertexAttribArray(5);
//    glEnableVertexAttribArray(6);
//    glEnableVertexAttribArray(7);

    // unbind buffer i guess
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // unbind vao
    glBindVertexArray(0);

    // delete mesh data as it is no longer needed
    registry.remove<Components::ChunkMeshData>(chunkEntity);
//    std::cout << "Done buffering mesh" << std::endl;
}

void ChunkManagement::render(entt::registry &registry, int screenWidth, int screenHeight) {
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
        auto [playerCam, playerPos, playerChunkPos, playerDir] = registry.get<Components::CameraAttach, Components::Position, Components::ChunkPosition, Components::DirectionPitchYaw>(player);

        glm::mat4 projection = glm::perspective(glm::radians((float) playerCam.fov), screenWidth / (float) screenHeight, 0.5f, 600.0f);
//        glm::mat4 projection = glm::ortho(-500.f, 500.f, -500.f, 500.f, 0.1f, 100.0f);

//        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3((float) -(playerPos.pos.x - playerChunkPos.x * CHUNK_SIZE), (float) -(playerPos.pos.y - playerChunkPos.y * CHUNK_SIZE), (float) -(playerPos.pos.z - playerChunkPos.z * CHUNK_SIZE)));
        glm::mat4 view = glm::rotate(projection, (float) playerDir.pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, (float) playerDir.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3((float) -(playerPos.pos.x), (float) -(playerPos.pos.y), (float) -(playerPos.pos.z)));


        voxelShader.use();
        voxelShader.setMatrix4("viewProjection", view);

        auto renderableChunks = registry.view<Components::ChunkPosition, Components::ChunkOpenGL>();

        // render visible chunks
        for (auto c : renderableChunks) {
            auto [chunkPos, chunkGL] = renderableChunks.get<Components::ChunkPosition, Components::ChunkOpenGL>(c);

            if (chunkGL.numTriangles > 0) {
//                std::cout << "Rendering " << chunkGL.numTriangles << " triangles." << std::endl;
//                voxelShader.setVec3i("chunkPos", chunkPos.x - playerChunkPos.x, chunkPos.y - playerChunkPos.y, chunkPos.z - playerChunkPos.z);
                voxelShader.setVec3i("chunkPos", chunkPos.x, chunkPos.y, chunkPos.z);
//                std::cout << "cx: " << chunkPos.x << " cy: " << chunkPos.y << " cz: " << chunkPos.z << std::endl;
                glBindVertexArray(chunkGL.vao);
                glDrawArrays(GL_TRIANGLES, 0, chunkGL.numTriangles);
                glBindVertexArray(0);
            }
        }
    }
}

Shader &ChunkManagement::getShader() {
    return voxelShader;
}

void ChunkManagement::makeVertexArray(unsigned char x, unsigned char y, unsigned char z,
                                      unsigned char tx, unsigned char ty, unsigned char n,
                                      unsigned char tex, unsigned char b, unsigned char vertex[]) {
    vertex[0] = x;
    vertex[1] = y;
    vertex[2] = z;
    vertex[3] = tx;
    vertex[4] = ty;
    vertex[5] = n;
    vertex[6] = tex;
    vertex[7] = b;
//    std::cout << (int)tx << " " << (int)ty << std::endl;
}

void ChunkManagement::addVertexArrayToMesh(std::vector<unsigned char> &mesh, unsigned char vertex[],
                                           int numBytesToCopy) {
    for (int i = 0; i < numBytesToCopy; i++) {
        mesh.push_back(vertex[i]);
    }
}
