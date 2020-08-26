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
    const static int CHUNK_LOAD_RADIUS = 350; // this is in voxels, not chunks
    const static int CHUNK_UNLOAD_RADIUS = 360; // this is in voxels, not chunks
    const static int MAX_BUFFERS_PER_FRAME = 1;
    const static int MAX_GENERATES_PER_FRAME = 1;
    const static int NUM_BYTES_PER_VERTEX = 8; // was 6

    std::unordered_map<std::string, entt::entity> chunkKeyToChunkEntity;
    std::vector<entt::entity> chunks;

    Shader voxelShader;

    FastNoise noise;

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

/*            0.f, 0.f, 1.f,  0.f, 0.f,  0.f,
            1.f, 0.f, 1.f,  1.f, 0.f,  0.f,
            1.f, 1.f, 1.f,  1.f, 1.f,  0.f,
            0.f, 0.f, 1.f,  0.f, 0.f,  0.f,
            1.f, 1.f, 1.f,  1.f, 1.f,  0.f,
            0.f, 1.f, 1.f,  0.f, 1.f,  0.f,

            1.f, 0.f, 1.f,  0.f, 0.f,  1.f,
            1.f, 0.f, 0.f,  1.f, 0.f,  1.f,
            1.f, 1.f, 0.f,  1.f, 1.f,  1.f,
            1.f, 0.f, 1.f,  0.f, 0.f,  1.f,
            1.f, 1.f, 0.f,  1.f, 1.f,  1.f,
            1.f, 1.f, 1.f,  0.f, 1.f,  1.f,

            1.f, 0.f, 0.f,  0.f, 0.f,  2.f,
            0.f, 0.f, 0.f,  1.f, 0.f,  2.f,
            0.f, 1.f, 0.f,  1.f, 1.f,  2.f,
            1.f, 0.f, 0.f,  0.f, 0.f,  2.f,
            0.f, 1.f, 0.f,  1.f, 1.f,  2.f,
            1.f, 1.f, 0.f,  0.f, 1.f,  2.f,

            0.f, 0.f, 0.f,  0.f, 0.f,  3.f,
            0.f, 0.f, 1.f,  1.f, 0.f,  3.f,
            0.f, 1.f, 1.f,  1.f, 1.f,  3.f,
            0.f, 0.f, 0.f,  0.f, 0.f,  3.f,
            0.f, 1.f, 1.f,  1.f, 1.f,  3.f,
            0.f, 1.f, 0.f,  0.f, 1.f,  3.f,

            0.f, 1.f, 1.f,  0.f, 0.f,  4.f,
            1.f, 1.f, 1.f,  1.f, 0.f,  4.f,
            1.f, 1.f, 0.f,  1.f, 1.f,  4.f,
            0.f, 1.f, 1.f,  0.f, 0.f,  4.f,
            1.f, 1.f, 0.f,  1.f, 1.f,  4.f,
            0.f, 1.f, 0.f,  0.f, 1.f,  4.f,

            0.f, 0.f, 0.f,  0.f, 0.f,  5.f,
            1.f, 0.f, 0.f,  1.f, 0.f,  5.f,
            1.f, 0.f, 1.f,  1.f, 1.f,  5.f,
            0.f, 0.f, 0.f,  0.f, 0.f,  5.f,
            1.f, 0.f, 1.f,  1.f, 1.f,  5.f,
            0.f, 0.f, 1.f,  0.f, 1.f,  5.f,*/

/*
            0.f, 0.f, 1.f,  0.f, 0.f,  0.f, 0.f, 1.f,
            1.f, 0.f, 1.f,  1.f, 0.f,  0.f, 0.f, 1.f,
            1.f, 1.f, 1.f,  1.f, 1.f,  0.f, 0.f, 1.f,
            0.f, 0.f, 1.f,  0.f, 0.f,  0.f, 0.f, 1.f,
            1.f, 1.f, 1.f,  1.f, 1.f,  0.f, 0.f, 1.f,
            0.f, 1.f, 1.f,  0.f, 1.f,  0.f, 0.f, 1.f,

            1.f, 0.f, 1.f,  0.f, 0.f,  1.f, 0.f, 0.f,
            1.f, 0.f, 0.f,  1.f, 0.f,  1.f, 0.f, 0.f,
            1.f, 1.f, 0.f,  1.f, 1.f,  1.f, 0.f, 0.f,
            1.f, 0.f, 1.f,  0.f, 0.f,  1.f, 0.f, 0.f,
            1.f, 1.f, 0.f,  1.f, 1.f,  1.f, 0.f, 0.f,
            1.f, 1.f, 1.f,  0.f, 1.f,  1.f, 0.f, 0.f,

            1.f, 0.f, 0.f,  0.f, 0.f,  0.f, 0.f,-1.f,
            0.f, 0.f, 0.f,  1.f, 0.f,  0.f, 0.f,-1.f,
            0.f, 1.f, 0.f,  1.f, 1.f,  0.f, 0.f,-1.f,
            1.f, 0.f, 0.f,  0.f, 0.f,  0.f, 0.f,-1.f,
            0.f, 1.f, 0.f,  1.f, 1.f,  0.f, 0.f,-1.f,
            1.f, 1.f, 0.f,  0.f, 1.f,  0.f, 0.f,-1.f,

            0.f, 0.f, 0.f,  0.f, 0.f, -1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,  1.f, 0.f, -1.f, 0.f, 0.f,
            0.f, 1.f, 1.f,  1.f, 1.f, -1.f, 0.f, 0.f,
            0.f, 0.f, 0.f,  0.f, 0.f, -1.f, 0.f, 0.f,
            0.f, 1.f, 1.f,  1.f, 1.f, -1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,  0.f, 1.f, -1.f, 0.f, 0.f,

            0.f, 1.f, 1.f,  0.f, 0.f,  0.f, 1.0f, 0.0f,
            1.f, 1.f, 1.f,  1.f, 0.f,  0.f, 1.0f, 0.0f,
            1.f, 1.f, 0.f,  1.f, 1.f,  0.f, 1.0f, 0.0f,
            0.f, 1.f, 1.f,  0.f, 0.f,  0.f, 1.0f, 0.0f,
            1.f, 1.f, 0.f,  1.f, 1.f,  0.f, 1.0f, 0.0f,
            0.f, 1.f, 0.f,  0.f, 1.f,  0.f, 1.0f, 0.0f,

            0.f, 0.f, 0.f,  0.f, 0.f,  0.f,-1.0f, 0.0f,
            1.f, 0.f, 0.f,  1.f, 0.f,  0.f,-1.0f, 0.0f,
            1.f, 0.f, 1.f,  1.f, 1.f,  0.f,-1.0f, 0.0f,
            0.f, 0.f, 0.f,  0.f, 0.f,  0.f,-1.0f, 0.0f,
            1.f, 0.f, 1.f,  1.f, 1.f,  0.f,-1.0f, 0.0f,
            0.f, 0.f, 1.f,  0.f, 1.f,  0.f,-1.0f, 0.0f*/
    };


    // these are not pointers because an entt::entity is just a uint32 that shouldn't change after it is created.
    void tryCreateChunk(entt::registry& registry, int xChunk, int yChunk, int zChunk);
    void tryRemoveChunk(entt::registry& registry, entt::entity chunkEntity);

    static Components::Position chunkComparePos;
    static entt::registry* chunkCompareRegistry;
    static bool chunkCompareFun(entt::entity chunk1, entt::entity chunk2);
    // assumes chunk entity already has ChunkData, ChunkStatus,
    std::thread* threadLaunchPointer;
    static void generateChunk(Components::ChunkStatus &chunkStatus, Components::ChunkPosition &chunkPosition, Components::ChunkData &chunkData, FastNoise &n);

    // assumes chunk entity already has ChunkData, ChunkStatus, ChunkMesh
    static void generateMesh(Components::ChunkStatus &chunkStatus, Components::ChunkPosition &chunkPosition,
                             Components::ChunkData &chunkData, Components::ChunkMeshData &chunkMeshData);

    // outer loop is major, inner loop is minor
    // slice is the progress in the 3rd direction away from 0, 0, 0
    static void addSlice(std::vector<unsigned char> &mesh, Components::ChunkData &voxelData,
                         DataTypes::Axis axis, bool flipped, int slice);

    static unsigned long makeVertex(unsigned char x, unsigned char y, unsigned char z,
                                    unsigned char tx, unsigned char ty, unsigned char n,
                                    unsigned char tex, unsigned char b);

    static void makeVertexArray(unsigned char x, unsigned char y, unsigned char z,
                                unsigned char tx, unsigned char ty, unsigned char n,
                                unsigned char tex, unsigned char b, unsigned char vertex[]);

    static void addVertexArrayToMesh(std::vector<unsigned char> &mesh, unsigned char vertex[], int numBytesToCopy);

    // adds ChunkOpenGL and generates vbo & vao
    // also configures vao
    void genVboVaoAndBuffer(entt::registry& registry, entt::entity chunkEntity);

//    union Vertex {
//        unsigned long asNum;
//        unsigned char asArray[8];
//    };

    union Vertex {
        unsigned long asNum;
        unsigned char asArray[8];
    };

    static void addVertexToMesh(std::vector<unsigned char> &mesh, Vertex vertex, int numBytesToCopy);
};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_CHUNKMANAGEMENT_H
