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
        unsigned char data[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    };

    static unsigned char chunkDataGet(ChunkData &data, int x, int y, int z);
    static unsigned char chunkDataGetAirBounds(ChunkData &data, int x, int y, int z);
    static void chunkDataSet(ChunkData &data, unsigned char value, int x, int y, int z);
    static bool voxelIsTouchingAir(ChunkData &data, int x, int y, int z);


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

    struct ChunkMeshData {
        std::vector<unsigned char> offsets;
        std::vector<unsigned char> dims;
        std::vector<unsigned char> textures;
        std::vector<unsigned char> brightnesses;
    };

    struct ChunkPosition {
        int x;
        int y;
        int z;
    };

    struct ChunkOpenGL {
        unsigned int offsetsVbo;
        unsigned int dimsVbo;
        unsigned int texturesVbo;
        unsigned int brightnessesVbo;
        unsigned int vao;
        unsigned int numInstances;
    };

    struct ChunkEntities {
        std::vector<entt::entity> entities;
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
    };

    struct TravelMaxSpeed {
        double maxSpeed;
    };

    static std::string dvecToString(glm::dvec3 &vec);

};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_COMPONENTS_H
