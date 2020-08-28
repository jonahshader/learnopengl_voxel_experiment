//
// Created by Jonah on 8/10/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_CHUNKMANAGEMENT_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_CHUNKMANAGEMENT_H


#include <unordered_map>
#include <string>
#include <vector>
#include <thread>
#include <entt/entt.hpp>
#include <external/fastnoise/FastNoise.h>
#include <ecs/Components.h>
#include <other/Constants.h>
#include <other/DataTypes.h>
#include <graphics/Shader.h>
#include <glm/glm.hpp>
#include <external/ThreadPool.h>



class ChunkManagement {
public:
    ChunkManagement(const char* vertexPath, const char* fragmentPath);

    entt::entity* getChunk(int xChunk, int yChunk, int zChunk); // returns nullptr if the chunk was not found
    void run(entt::registry& registry);
    void render(entt::registry& registry, int screenWidth, int screenHeight, const glm::vec3 &skyColor);
    static std::string chunkPositionToKey(int xChunk, int yChunk, int zChunk);
    static double worldPosChunkPosDist(Components::ChunkPosition &chunkPos, Components::Position &worldPos);
    static double worldPosChunkPosDist(int xc, int yc, int zc, double x, double y, double z);
    Shader &getShader();

private:
    const static int CHUNK_LOAD_RADIUS = 500; // this is in voxels, not chunks
    const static int CHUNK_UNLOAD_RADIUS = 550; // this is in voxels, not chunks
    const static int MAX_MESH_BUFFERS_PER_FRAME = 1;
    const static int MAX_CONCURRENT_MESH_GENS = 8;
    const static int MAX_GENERATES_PER_FRAME = 1;
    const static int MAX_CONCURRENT_GENERATES = 14;
    const static int NUM_BYTES_PER_VERTEX = 8; // was 6

    std::unordered_map<std::string, entt::entity> chunkKeyToChunkEntity;
    std::vector<entt::entity> chunks;

//    std::vector<std::thread> chunkGenThreadPool;
//    volatile std::vector<bool> chunkGenThreadsRunning;

    ThreadPool pool;

    Shader voxelShader;

    FastNoise mainNoise;
    FastNoise terraceOffset;
    FastNoise terraceSelect;

    float fogDistance;

    volatile int chunksCurrentlyGenerating;
    volatile int chunksCurrentlyMeshing;


    unsigned int cubeVbo;

    const float cubeData[6 * 6 * (3 + 2 + 1)] = {
            // pos, tex, norm
            0.f, 0.f, 1.f,  0.f, 1.f,  0.f,
            1.f, 0.f, 1.f,  1.f, 1.f,  0.f,
            1.f, 1.f, 1.f,  1.f, 0.f,  0.f,
            0.f, 0.f, 1.f,  0.f, 1.f,  0.f,
            1.f, 1.f, 1.f,  1.f, 0.f,  0.f,
            0.f, 1.f, 1.f,  0.f, 0.f,  0.f,

            1.f, 0.f, 1.f,  0.f, 1.f,  1.f,
            1.f, 0.f, 0.f,  1.f, 1.f,  1.f,
            1.f, 1.f, 0.f,  1.f, 0.f,  1.f,
            1.f, 0.f, 1.f,  0.f, 1.f,  1.f,
            1.f, 1.f, 0.f,  1.f, 0.f,  1.f,
            1.f, 1.f, 1.f,  0.f, 0.f,  1.f,

            1.f, 0.f, 0.f,  0.f, 1.f,  2.f,
            0.f, 0.f, 0.f,  1.f, 1.f,  2.f,
            0.f, 1.f, 0.f,  1.f, 0.f,  2.f,
            1.f, 0.f, 0.f,  0.f, 1.f,  2.f,
            0.f, 1.f, 0.f,  1.f, 0.f,  2.f,
            1.f, 1.f, 0.f,  0.f, 0.f,  2.f,

            0.f, 0.f, 0.f,  0.f, 1.f,  3.f,
            0.f, 0.f, 1.f,  1.f, 1.f,  3.f,
            0.f, 1.f, 1.f,  1.f, 0.f,  3.f,
            0.f, 0.f, 0.f,  0.f, 1.f,  3.f,
            0.f, 1.f, 1.f,  1.f, 0.f,  3.f,
            0.f, 1.f, 0.f,  0.f, 0.f,  3.f,

            0.f, 1.f, 1.f,  0.f, 1.f,  4.f,
            1.f, 1.f, 1.f,  1.f, 1.f,  4.f,
            1.f, 1.f, 0.f,  1.f, 0.f,  4.f,
            0.f, 1.f, 1.f,  0.f, 1.f,  4.f,
            1.f, 1.f, 0.f,  1.f, 0.f,  4.f,
            0.f, 1.f, 0.f,  0.f, 0.f,  4.f,

            0.f, 0.f, 0.f,  0.f, 1.f,  5.f,
            1.f, 0.f, 0.f,  1.f, 1.f,  5.f,
            1.f, 0.f, 1.f,  1.f, 0.f,  5.f,
            0.f, 0.f, 0.f,  0.f, 1.f,  5.f,
            1.f, 0.f, 1.f,  1.f, 0.f,  5.f,
            0.f, 0.f, 1.f,  0.f, 0.f,  5.f,
    };


    // these are not pointers because an entt::entity is just a uint32 that shouldn't change after it is created.
    void tryCreateChunk(entt::registry& registry, int xChunk, int yChunk, int zChunk);
    void tryRemoveChunk(entt::registry& registry, entt::entity chunkEntity);

    static Components::Position chunkComparePos;
    static entt::registry* chunkCompareRegistry;
    static bool chunkCompareFun(entt::entity chunk1, entt::entity chunk2);
    // assumes chunk entity already has ChunkData, ChunkStatus,
    void generateChunk(volatile Components::ChunkStatusEnum* chunkStatus, int xChunk, int yChunk, int zChunk,
                       unsigned char* chunkData);

    // assumes chunk entity already has ChunkData, ChunkStatus, ChunkMesh
    void generateMesh(volatile Components::ChunkStatusEnum* chunkStatus,
                      unsigned char* chunkData, std::vector<unsigned char>* offsets,
                      std::vector<unsigned char>* dims, std::vector<unsigned char>* textures,
                      std::vector<unsigned char>* brightnesses, std::vector<unsigned char*>* neighborChunks);

    bool chunkHasAllNeighborData(entt::registry &registry, Components::ChunkPosition &chunkPosition);

    bool chunkHasData(entt::registry &registry, int xChunk, int yChunk, int zChunk);

    Components::ChunkData& getChunkData(entt::registry &registry, int xChunk, int yChunk, int zChunk);

    // adds ChunkOpenGL and generates vbo & vao
    // also configures vao
    void genVboVaoAndBuffer(entt::registry& registry, entt::entity chunkEntity);
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_CHUNKMANAGEMENT_H
