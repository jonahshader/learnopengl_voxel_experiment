//
// Created by Jonah on 8/10/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_CHUNKMANAGEMENT_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_CHUNKMANAGEMENT_H


#include <unordered_map>
#include <string>
#include <vector>
#include <thread>
#include <random>
#include <entt/entt.hpp>
#include <external/fastnoise/FastNoise.h>
#include <ecs/Components.h>
#include <other/Constants.h>
#include <other/DataTypes.h>
#include <graphics/Shader.h>
#include <graphics/TextureManager.h>
#include <glm/glm.hpp>
#include <external/ThreadPool.h>



class ChunkManagement {
public:
    ChunkManagement(const char* vertexPathInstVer, const char* fragmentPathInstVer,
                    const char* vertexPathTriVer, const char* fragmentPathTriVer,
                    std::random_device &rd);

    entt::entity* getChunk(int xChunk, int yChunk, int zChunk); // returns nullptr if the chunk was not found
    bool isChunkDataLoaded(entt::registry &registry, int xChunk, int yChunk, int zChunk);
    bool getVoxel(entt::registry &registry, int x, int y, int z, unsigned char &voxel); // returns true if success
    bool inSolidBlock(entt::registry &registry, glm::dvec3 &pos); // unloaded chunk is not solid
    void run(entt::registry& registry);
    void render(entt::registry& registry, TextureManager &tm, int screenWidth, int screenHeight, const glm::vec3 &skyColor);
    static std::string chunkPositionToKey(int xChunk, int yChunk, int zChunk);
    static double worldPosChunkPosDist(Components::ChunkPosition &chunkPos, Components::Position &worldPos);
    static double worldPosChunkPosDist(int xc, int yc, int zc, double x, double y, double z);
    Shader &getShader();

private:
    const static int CHUNK_LOAD_RADIUS = 550 + CHUNK_SIZE; // this is in voxels, not chunks
    const static int CHUNK_UNLOAD_RADIUS = 550 + CHUNK_SIZE + CHUNK_SIZE; // this is in voxels, not chunks
    const static int MAX_MESH_BUFFERS_PER_FRAME = 1;
    const static int MAX_CONCURRENT_MESH_GENS = 5;
    const static int MAX_GENERATES_PER_FRAME = 1;
    const static int MAX_CONCURRENT_GENERATES = 8;
    const static int NUM_BYTES_PER_VERTEX = 8; // was 6

    constexpr static float SIDE_BRIGHTNESS_MULT = 0.66f;
    constexpr static float BOTTOM_BRIGHTNESS_MULT = 0.25f;

    std::unordered_map<std::string, entt::entity> chunkKeyToChunkEntity;
    std::vector<entt::entity> chunks;

    std::random_device &rd;

    ThreadPool pool;

    Shader voxelShader, voxelShaderTris;

    FastNoise mainNoise;
    FastNoise terraceOffset;
    FastNoise terraceSelect;
    FastNoise smoothHillSelect;
    FastNoise smoothHill;
    FastNoise overallTerrainHeightOffset;
    FastNoise caveNoiseA, caveNoiseB, caveNoiseC;
    float smoothHillMagnitude;

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

    void tryCreateChunk(entt::registry& registry, int xChunk, int yChunk, int zChunk);
    // entt::entity is just a uint32 so there is no need to reference, copying is fine.
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

    void generateMeshTris(volatile Components::ChunkStatusEnum* chunkStatus,
                          unsigned char* chunkData, std::vector<unsigned char>* tris,
                          std::vector<unsigned char*>* neighborChunks);

    void generateMeshGreedy(volatile Components::ChunkStatusEnum* chunkStatus,
                            unsigned char* chunkData, std::vector<unsigned char>* tris,
                            std::vector<unsigned char*>* neighborChunks);

    void generateMeshDumb(volatile Components::ChunkStatusEnum* chunkStatus,
                          unsigned char* chunkData, std::vector<unsigned char>* tris,
                          std::vector<unsigned char*>* neighborChunks);

    bool chunkHasAllNeighborData(entt::registry &registry, Components::ChunkPosition &chunkPosition);

    bool chunkHasData(entt::registry &registry, int xChunk, int yChunk, int zChunk);

    Components::ChunkData& getChunkData(entt::registry &registry, int xChunk, int yChunk, int zChunk);

    // adds ChunkOpenGL and generates vbo & vao
    // also configures vao
    void genVboVaoAndBuffer(entt::registry& registry, entt::entity chunkEntity);
    void genVboVaoAndBufferTris(entt::registry& registry, entt::entity chunkEntity);

    void makeVertex(std::vector<unsigned char> &mesh, unsigned char x, unsigned char y, unsigned char z,
                    unsigned char texture, unsigned char xTex, unsigned char yTex,
                    unsigned char brightness, unsigned char normal);

    void fixGrass(unsigned char* chunkData, std::vector<unsigned char*>* neighborChunks);
    void calculateBrightness(unsigned char* bVals, unsigned char* chunkData, std::vector<unsigned char*>* neighborChunks);
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_CHUNKMANAGEMENT_H
