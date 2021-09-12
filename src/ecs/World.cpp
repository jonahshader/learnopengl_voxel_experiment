//
// Created by Jonah on 8/19/2020.
//

#include "World.h"

#include <iostream>
//#include <ctime>

World::World(const char* vertexPathInst, const char* fragmentPathInst,
             const char* vertexPathTri, const char* fragmentPathTri) :
seeder(std::random_device{}()), // 0
registry(),
chunkManagement(vertexPathInst, fragmentPathInst, vertexPathTri, fragmentPathTri, seeder),
ai(seeder),
graphics(registry),
textureManager(),
pMouseX(0.0),
pMouseY(0.0),
firstMouse(true),
screenWidth(800),
screenHeight(600),
skyColor(0.3f, 0.45f, 0.95f)
{
    // load textures
    textureManager.addSpriteArray("textures/spritesheet.png", "textures/spritesheet.atlas");
    textureManager.addSpriteSheet("textures/padded_textures.png", "textures/padded_textures.atlas");
    textureManager.addSpriteSheet("textures/items.png", "textures/items.pack");
    // create player
    entt::entity player = registry.create();
    registry.emplace<Components::Position>(player, glm::dvec3(0.5, 0.5, 0.5));
    registry.emplace<Components::Velocity>(player, glm::dvec3(0.0));
    registry.emplace<Components::Acceleration>(player, glm::dvec3(0.0));
    registry.emplace<Components::BoxCollider>(player, 1.9, 0.4, glm::dvec3(0.5, 0.5, 0.5));
    registry.emplace<Components::VoxelCollision>(player, false);
    registry.emplace<Components::ChunkPosition>(player, 0, 0, 0);
    registry.emplace<Components::DirectionPitchYaw>(player, M_PI / 2, 0.0);
    registry.emplace<Components::PlayerControl>(player);
    registry.emplace<Components::TravelMaxSpeed>(player, 5.612);
    registry.emplace<Components::JumpVelocity>(player, 7.0);
    registry.emplace<Components::CameraAttach>(player, 90.0, glm::dvec3(0.0, 1.75, 0.0));
    registry.emplace<Components::Gravity>(player, -20.0);
    registry.emplace<Components::TargetVelocity>(player, glm::dvec3(0.0));
//    registry.emplace<Components::YTeleport>(player, CHUNK_SIZE, 1.0, false);


    // make 100 billboards
    // make random num gen distribution
    std::uniform_real_distribution<double> bbDist(-32, 32);
    std::uniform_real_distribution<double> zeroOneDist(0.0, 1.0);
    std::mt19937_64 randomEngine(seeder());
    auto spriteInfo = textureManager.getTextureInfoSS("person");
    auto textureDim = textureManager.getTextureDimensions(spriteInfo.textureUnit);

    float sx = spriteInfo.sx / textureDim.textureWidth;
    float sy = (spriteInfo.sy / textureDim.textureHeight) + (spriteInfo.sh / textureDim.textureHeight);
    float sw = spriteInfo.sw / textureDim.textureWidth;
    float sh = -(spriteInfo.sh / textureDim.textureHeight);

    std::cout << spriteInfo.name << " " << spriteInfo.sw << " " << spriteInfo.sh << " " << spriteInfo.sx << " " << spriteInfo.sy << std::endl;
    for (int i = 0; i < 0; ++i) {
        entt::entity bb = registry.create();
        float scale = zeroOneDist(randomEngine) * 0.9f + 0.1f;
        registry.emplace<Components::GraphicBillboard>(bb, sx, sy, sw, sh, scale, scale * 2, true);

        double x = bbDist(randomEngine), y = bbDist(randomEngine) * 0.0625 + 128.0, z = bbDist(randomEngine);
        registry.emplace<Components::Position>(bb, glm::dvec3(x, y, z));
        registry.emplace<Components::Velocity>(bb, glm::dvec3(0.0, 0.0, 0.0));
        registry.emplace<Components::Acceleration>(bb, glm::dvec3(0.0, 0.0, 0.0));
        registry.emplace<Components::Gravity>(bb, -20.0);
        registry.emplace<Components::TravelMaxSpeed>(bb, 5.612 * scale);
        registry.emplace<Components::VoxelCollision>(bb, false);
        registry.emplace<Components::ChunkPosition>(bb, 0, 0, 0);
        registry.emplace<Components::JumpVelocity>(bb, 7.0);
        registry.emplace<Components::TargetVelocity>(bb, glm::dvec3(0.0));
//        registry.emplace<Components::BoxCollider>(bb, 0.98, 0.4, glm::dvec3(x, y, z));
        registry.emplace<Components::PointCollider>(bb, glm::dvec3(x, y, z));
        registry.emplace<Components::AIRandomWalking>(bb, 1.0f, 1.0f, 1.0f);

//        struct GraphicBillboard {
//            float sx; // from 0 to 1
//            float sy;
//            float sw; // from 0 to 1
//            float sh;
//            float width; // world size
//            float height;
//            bool yLocked;
//        };

    }
}

void World::run(double dt, GLFWwindow *window) {
    chunkManagement.run(registry);
    PlayerControl::updateMovement(registry, dt, window);
    Physics::update(registry, chunkManagement, dt);
    ai.update(registry, dt);
    graphics.update(registry);

//    registry.view<Components::Position, Components::ChunkPosition, Components::DirectionPitchYaw>().each([](auto &pos, auto &chunkPos, auto &dir) {
//        std::cout << "Position: " << Components::dvecToString(pos.pos)
//        << "\nChunk Position: x: " << chunkPos.x << " y: " << chunkPos.y << " z: " << chunkPos.z
//        << "\nDirection: pitch: " << dir.pitch << " yaw: " << dir.yaw << std::endl;
//    });

}

void World::draw(GLFWwindow *window) {
    chunkManagement.render(registry, textureManager, screenWidth, screenHeight, skyColor);
    graphics.render(registry, textureManager, screenWidth, screenHeight, chunkManagement.getFogDistance(), skyColor); // TODO: take fogDistance from chunkManager
}

void World::updateWindowSize(int width, int height) {
    if (!(width == 0 || height == 0)) {
        screenWidth = width;
        screenHeight = height;
    }
}

void World::updateMouse(double xpos, double ypos) {
    if (firstMouse) {
        firstMouse = false;
    } else {
        PlayerControl::updateLook(registry, -ypos + pMouseY, xpos - pMouseX);
    }
    pMouseX = xpos;
    pMouseY = ypos;
}

void World::updateScroll(double xoffset, double yoffset) {

}

const glm::vec3 &World::getSkyColor() const {
    return skyColor;
}
