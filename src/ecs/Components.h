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


class Components {
public:
    Components() = delete;

    struct ChunkData {
        // if byte is 0xFF, go to lookup table to find actual value.
        // hashtable with voxel xyz as key will yield the actual value
        // of the voxel. this is only for when there is greater than
        // 256 types of blocks.
//        unsigned char data[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
        std::vector<unsigned char> data;
    };

    static unsigned char chunkDataGet(unsigned char* data, int x, int y, int z);
    static unsigned char chunkDataGet(std::vector<unsigned char*>* chunks, int x, int y, int z);
//    static unsigned char chunkDataGetOptimizeChoose(ChunkData &data, ChunkData* chunks[], bool optimize, int x, int y, int z);
    static unsigned char chunkDataGetAirBounds(unsigned char* data, int x, int y, int z);
    static void chunkDataSet(unsigned char* data, unsigned char value, int x, int y, int z);
    static bool voxelIsTouchingAirAirBounds(unsigned char* data, int x, int y, int z);
    static bool voxelIsTouchingAirWithNeighbors(std::vector<unsigned char*>* chunks, int x, int y, int z);
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
        volatile ChunkStatusEnum* status;
        volatile bool* markedForRemoval;
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

    struct ChunkCollision{
        bool grounded;
    };

    struct CylinderCollider{
        double height;
        double radius;
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
        unsigned char texture;
        double width;
        double height;
        bool yLocked;
    };

    struct AIRandomWalking {
        double newDirectionPeriod;
        double newDirectionTimer;
    };

    static std::string dvecToString(glm::dvec3 &vec);

};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_COMPONENTS_H
