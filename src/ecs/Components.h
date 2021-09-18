//
// Created by Jonah on 8/10/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_COMPONENTS_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_COMPONENTS_H

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <entt/entt.hpp>

#include <other/Constants.h>
#include <other/DataTypes.h>


class Components {
public:
    Components() = delete;

    struct ChunkData {
        // if byte is 0xFF, go to lookup table to find actual value.
        // hashtable with voxel xyz as key will yield the actual value
        // of the voxel. this is only for when there is greater than
        // 256 types of blocks.
//        blockid data[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
        std::vector<blockid> data;
    };

    static blockid chunkDataGet(blockid* data, int x, int y, int z);
    static blockid chunkDataGet(std::vector<blockid*>* chunks, int x, int y, int z);
//    static blockid chunkDataGetOptimizeChoose(ChunkData &data, ChunkData* chunks[], bool optimize, int x, int y, int z);
    static blockid chunkDataGetAirBounds(blockid* data, int x, int y, int z);
    static void chunkDataSet(blockid* data, blockid value, int x, int y, int z);
    static bool voxelIsTouchingAirAirBounds(blockid* data, int x, int y, int z);
    static bool voxelIsTouchingAirWithNeighbors(std::vector<blockid*>* chunks, int x, int y, int z);
//    static bool voxelIsTouchingAirOptimizeChoose(ChunkData &data, ChunkData* chunks[], bool optimize, int x, int y, int z);


    enum ChunkStatusEnum {
        NEW,
        GENERATING_OR_LOADING,
        GENERATED_OR_LOADED,
        MESH_GENERATING,
        MESH_GENERATED,
        MESH_BUFFERED
    };

    struct ChunkStatus {
        volatile ChunkStatusEnum status;
        volatile bool markedForRemoval;
    };

    struct ChunkFog {
        bool xMin; // false = clear, true = fog
        bool xMax;
        bool yMin;
        bool yMax;
        bool zMin;
        bool zMax;
    };

    struct ChunkMeshData {
        std::vector<unsigned char>* offsets;
        std::vector<unsigned char>* dims;
        std::vector<unsigned char>* textures;
        std::vector<unsigned char>* brightnesses;
    };

    struct ChunkMeshDataTris {
        std::vector<unsigned char>* tris;
    };

    struct ChunkPosition {
        int x;
        int y;
        int z;
    };

    struct ChunkOpenGLInstanceVer {
        unsigned int offsetsVbo;
        unsigned int dimsVbo;
        unsigned int texturesVbo;
        unsigned int brightnessesVbo;
        unsigned int vao;
        unsigned int numInstances;
    };

    struct ChunkOpenGLTriVer {
        unsigned int vbo;
        unsigned int vao;
        unsigned int numTriangles;
    };

    struct ChunkEntities {
        std::vector<entt::entity> entities;
    };

    struct VoxelCollision {
        bool grounded;
    };

    struct YTeleport {
        unsigned char maxSteps;
        double stepSize;
        bool requireGrounded;
    };

    struct BoxCollider{
        double height;
        double radius;
        glm::dvec3 lastValidPos;
    };

    struct PointCollider {
        glm::dvec3 lastValidPos;
    };

    struct Position {
        glm::dvec3 pos;
    };

    struct Velocity {
        glm::dvec3 vel;
    };

    struct Acceleration {
        glm::dvec3 acc;
    };

    struct DirectionPitchYaw {
        double pitch;
        double yaw;
    };

    struct PlayerControl {};

    struct Dig {
        double radius;
    };

    struct Score {
        long score;
    };

    struct Collectible {};

    struct CameraAttach {
        double fov;
        glm::dvec3 posOffset;
    };

    struct TravelMaxSpeed {
        double maxSpeed;
    };

    struct JumpVelocity {
        double jumpUpVelocity;
    };

    struct TargetVelocity {
        glm::dvec3 targetVel;
    };

    struct Gravity {
        double gravity;
    };

    struct GraphicBillboard {
        float sx; // from 0 to 1
        float sy;
        float sw; // from 0 to 1
        float sh;
        float width; // world size
        float height;
        bool yLocked;
    };

    struct AIRandomWalking {
        double newDirectionPeriod;
        double newDirectionTimer;
        double newDirectionVariance;
    };

    static std::string dvecToString(glm::dvec3 &vec);

};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_COMPONENTS_H
