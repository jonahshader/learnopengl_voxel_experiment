//
// Created by Jonah on 8/11/2020.
//

#include "Physics.h"
#include <ecs/Components.h>


void Physics::update(entt::registry &registry, ChunkManagement &chunkManagement, double dt) {
    updateAccelFromVelocityTarget(registry, dt);
    updateVelocity(registry, dt);
    updatePosition(registry, chunkManagement, dt);
}

void Physics::updatePosition(entt::registry &registry, ChunkManagement &chunkManagement, double dt) {
    // update position from velocity
    registry.view<Components::Position, Components::Velocity>().each([dt](auto &pos, auto &vel) {
        pos.pos += vel.vel * dt;
    });

    runYTeleport(registry, chunkManagement);
    runBoxCollision(registry, chunkManagement);
    runPointCollision(registry, chunkManagement, dt);

    // update chunk positions
    registry.view<Components::Position, Components::ChunkPosition>().each([](auto &pos, auto &cPos) {
        int xCNew = floor(pos.pos.x / CHUNK_SIZE);
        int yCNew = floor(pos.pos.y / CHUNK_SIZE);
        int zCNew = floor(pos.pos.z / CHUNK_SIZE);

        // TODO: on chunk pos change, notify chunk entity lists.
        cPos.x = xCNew;
        cPos.y = yCNew;
        cPos.z = zCNew;
    });
}

void Physics::updateVelocity(entt::registry &registry, double dt) {
    registry.view<Components::Velocity, Components::Acceleration>().each([dt](auto &vel, auto &acc) {
        vel.vel += acc.acc * dt;
        acc.acc.x = 0;
        acc.acc.y = 0;
        acc.acc.z = 0;
    });

    registry.view<Components::Velocity, Components::Gravity>().each([dt](auto &vel, auto &grav) {
        vel.vel.y += grav.gravity * dt;
    });
}

void Physics::updateAccelFromVelocityTarget(entt::registry &registry, double dt) {
    auto view = registry.view<Components::Velocity, Components::Acceleration, Components::TargetVelocity>();

    for (auto entity: view){
        auto [vel, acc, targetVel] = view.get<Components::Velocity, Components::Acceleration, Components::TargetVelocity>(entity);

        // horizontal magnitudes
        double currentMagnitude = sqrt(pow(vel.vel.x, 2) + pow(vel.vel.z, 2));
        double targetMagnitude = sqrt(pow(targetVel.targetVel.x, 2) + pow(targetVel.targetVel.z, 2));
        double currentDirection = atan2(vel.vel.z, vel.vel.x);
        double targetDirection = atan2(targetVel.targetVel.z, targetVel.targetVel.x);
        double accel = 60;

        if (registry.has<Components::VoxelCollision>(entity)) {
            auto &chunkCollision = registry.get<Components::VoxelCollision>(entity);
            if (chunkCollision.grounded) {
                if (targetMagnitude > currentMagnitude) {
                    acc.acc.x += accel * cos(targetDirection);
                    acc.acc.z += accel * sin(targetDirection);
                } else {
                    double cosCDir = cos(currentDirection);
                    double sinCDir = sin(currentDirection);
                    acc.acc.x -= accel * cosCDir;
                    acc.acc.z -= accel * sinCDir;
                    double futureVelX = vel.vel.x + acc.acc.x * dt;
                    double futureVelZ = vel.vel.z + acc.acc.z * dt;
                    double futureMagnitude = sqrt(pow(futureVelX, 2) + pow(futureVelZ, 2));
                    if (futureMagnitude > currentMagnitude) {
//                        vel.vel.x = 0;
//                        vel.vel.z = 0;
                        acc.acc.x = -vel.vel.x / dt;
                        acc.acc.z = -vel.vel.z / dt;
                    }

                }
//                if (targetVel.targetVel.y > vel.vel.y) {
                    vel.vel.y = targetVel.targetVel.y * std::max(1.0, currentMagnitude / 10.0);
//                }
            } else {
                if (targetMagnitude > currentMagnitude) {
                    acc.acc.x += accel * cos(targetDirection) * 0.33;
                    acc.acc.z += accel * sin(targetDirection) * 0.33;
                } else {
                    if (targetMagnitude > 0) {
                        acc.acc.x -= accel * cos(currentDirection) * 0.33;
                        acc.acc.z -= accel * sin(currentDirection) * 0.33;
                        acc.acc.x += accel * cos(targetDirection) * 0.33;
                        acc.acc.z += accel * sin(targetDirection) * 0.33;
                    }
//                    acc.acc.x -= accel * cos(currentDirection) * 0.125;
//                    acc.acc.z -= accel * sin(currentDirection) * 0.125;
                }
            }
        } else {
            if (targetMagnitude > currentMagnitude) {
                acc.acc.x += accel * cos(targetDirection);
                acc.acc.z += accel * sin(targetDirection);
            } else {
                acc.acc.x -= accel * cos(targetDirection);
                acc.acc.z -= accel * sin(targetDirection);
            }
        }
    }
}

bool Physics::checkRectangularCollision(entt::registry &registry, ChunkManagement &chunkManagement, glm::dvec3 &pos, Components::BoxCollider &collider) {
    glm::dvec3 moddedPos;

    int testHeight = std::ceil(collider.height);
    float testHeightInverse = 1.0f / testHeight;
    for (int y = 0; y <= testHeight; ++y)
    for (int x = -1; x <= 1; x += 2) {
        for (int z = -1; z <= 1; z += 2) {
            moddedPos = pos;
            moddedPos.x += x * collider.radius;
            moddedPos.z += z * collider.radius;
            moddedPos.y += y * collider.height * testHeightInverse;

            if (chunkManagement.inSolidBlock(registry, moddedPos)) return true;
        }
    }
    return false;
}

bool Physics::checkPointCollision(entt::registry &registry, ChunkManagement &chunkManagement, glm::dvec3 &pos) {
    return chunkManagement.inSolidBlock(registry, pos);
}

void Physics::runBoxCollision(entt::registry &registry, ChunkManagement &chunkManagement) {
    auto collisionTestView = registry.view<Components::Position, Components::ChunkPosition, Components::Velocity, Components::VoxelCollision, Components::BoxCollider>();

    // perform collision detection and handling
    for (auto entity: collisionTestView) {
        auto &pos = collisionTestView.get<Components::Position>(entity);
        auto &chunkPos = collisionTestView.get<Components::ChunkPosition>(entity);
        auto &vel = collisionTestView.get<Components::Velocity>(entity);
        auto &collider = collisionTestView.get<Components::BoxCollider>(entity);
        auto &voxelCollision = collisionTestView.get<Components::VoxelCollision>(entity);

        if (chunkManagement.isChunkDataLoaded(registry, chunkPos.x, chunkPos.y, chunkPos.z)) {
            if (checkRectangularCollision(registry, chunkManagement, pos.pos, collider)) {
                int xc;
                int yc = floor(collider.lastValidPos.y);
                int zc;
                int xcNew;
                int ycNew = floor(pos.pos.y);
                int zcNew;


                voxelCollision.grounded = (ycNew < yc);

                glm::dvec3 posPreMod = pos.pos;
                glm::dvec3 velPreMod = vel.vel;

                // try fixing just x
                pos.pos.x = collider.lastValidPos.x;
                vel.vel.x = 0;
                int correctionLevel = 0;
                if (checkRectangularCollision(registry, chunkManagement, pos.pos, collider)) {
                    pos.pos.x = posPreMod.x;
                    vel.vel.x = velPreMod.x;
                    pos.pos.y = collider.lastValidPos.y;
                    vel.vel.y = 0;
                    correctionLevel = 1;
                    if (checkRectangularCollision(registry, chunkManagement, pos.pos, collider)) {
                        pos.pos.y = posPreMod.y;
                        vel.vel.y = velPreMod.y;
                        pos.pos.z = collider.lastValidPos.z;
                        vel.vel.z = 0;
                        correctionLevel = 2;
                        if (checkRectangularCollision(registry, chunkManagement, pos.pos, collider)) {
                            pos.pos.x = collider.lastValidPos.x;
                            vel.vel.x = 0;
                            correctionLevel = 3;
                            if (checkRectangularCollision(registry, chunkManagement, pos.pos, collider)) {
                                pos.pos.z = posPreMod.z;
                                vel.vel.z = velPreMod.z;
                                pos.pos.y = collider.lastValidPos.y;
                                vel.vel.y = 0;
                                correctionLevel = 4;
                                if (checkRectangularCollision(registry, chunkManagement, pos.pos, collider)) {
                                    pos.pos.x = posPreMod.x;
                                    vel.vel.x = velPreMod.x;
                                    pos.pos.z = collider.lastValidPos.z;
                                    vel.vel.z = 0;
                                    correctionLevel = 5;
                                    if (checkRectangularCollision(registry, chunkManagement, pos.pos, collider)) {
                                        pos.pos = collider.lastValidPos;
                                        vel.vel.x = 0;
                                        vel.vel.y = 0;
                                        vel.vel.z = 0;
                                        correctionLevel = 6;
                                        if (checkRectangularCollision(registry, chunkManagement, pos.pos, collider)) {
                                            vel.vel = velPreMod;
                                            pos.pos = posPreMod;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                bool yFixed = correctionLevel == 1 || correctionLevel == 4 || correctionLevel == 5 || correctionLevel == 6;
                voxelCollision.grounded = voxelCollision.grounded && yFixed;
            } else {
                voxelCollision.grounded = false;
            }
        } else {
            vel.vel = glm::dvec3(0);
            pos.pos = collider.lastValidPos;
        }

        collider.lastValidPos = pos.pos;
    }
}

void Physics::runPointCollision(entt::registry &registry, ChunkManagement &chunkManagement, double dt) {
    auto collisionTestView = registry.view<Components::Position, Components::ChunkPosition, Components::Velocity, Components::VoxelCollision, Components::PointCollider>();

    // perform collision detection and handling
    for (auto entity: collisionTestView) {
        auto &pos = collisionTestView.get<Components::Position>(entity);
        auto &chunkPos = collisionTestView.get<Components::ChunkPosition>(entity);
        auto &vel = collisionTestView.get<Components::Velocity>(entity);
        auto &collider = collisionTestView.get<Components::PointCollider>(entity);
        auto &chunkCollision = collisionTestView.get<Components::VoxelCollision>(entity);

        if (chunkManagement.isChunkDataLoaded(registry, chunkPos.x, chunkPos.y, chunkPos.z)) {
            if (checkPointCollision(registry, chunkManagement, pos.pos)) {
                int xc = floor(collider.lastValidPos.x);
                int yc = floor(collider.lastValidPos.y);
                int zc = floor(collider.lastValidPos.z);
                int xcNew = floor(pos.pos.x);
                int ycNew = floor(pos.pos.y);
                int zcNew = floor(pos.pos.z);


                chunkCollision.grounded = (ycNew < yc);

                glm::dvec3 posPreMod = pos.pos;
                glm::dvec3 velPreMod = vel.vel;

                // try fixing just x
//                pos.pos.x = collider.lastValidPos.x;
//                vel.vel.x = 0;
//                int correctionLevel = 0;
//                if (checkPointCollision(registry, chunkManagement, pos.pos)) {
//                    pos.pos.x = posPreMod.x;
//                    vel.vel.x = velPreMod.x;
//                    pos.pos.y = collider.lastValidPos.y;
//                    vel.vel.y = 0;
//                    correctionLevel = 1;
//                    if (checkPointCollision(registry, chunkManagement, pos.pos)) {
//                        pos.pos.y = posPreMod.y;
//                        vel.vel.y = velPreMod.y;
//                        pos.pos.z = collider.lastValidPos.z;
//                        vel.vel.z = 0;
//                        correctionLevel = 2;
//                        if (checkPointCollision(registry, chunkManagement, pos.pos)) {
//                            pos.pos.x = collider.lastValidPos.x;
//                            vel.vel.x = 0;
//                            correctionLevel = 3;
//                            if (checkPointCollision(registry, chunkManagement, pos.pos)) {
//                                vel.vel = velPreMod;
//                                pos.pos = posPreMod;
//                            }
//                        }
//                    }
//                }

                bool yFixed = false;
                if (checkPointCollision(registry, chunkManagement, pos.pos)) {
                    if (xcNew > xc) {
                        pos.pos.x = collider.lastValidPos.x;
                        vel.vel.x = 0;
                    } else if (xcNew < xc) {
                        pos.pos.x = collider.lastValidPos.x;
                        vel.vel.x = 0;
                    }
                    if (ycNew > yc) {
                        pos.pos.y = collider.lastValidPos.y;
                        vel.vel.y = 0;
                        yFixed = true;
                    } else if (ycNew < yc) {
                        pos.pos.y = collider.lastValidPos.y;
                        vel.vel.y = 0;
                        yFixed = true;
                    }
                    if (zcNew > zc) {
                        pos.pos.z = collider.lastValidPos.z;
                        vel.vel.z = 0;
                    } else if (zcNew < zc) {
                        pos.pos.z = collider.lastValidPos.z;
                        vel.vel.z = 0;
                    }
                }
//                bool yFixed = correctionLevel == 1 || correctionLevel == 4 || correctionLevel == 5 || correctionLevel == 6;
                chunkCollision.grounded = chunkCollision.grounded && yFixed;
            } else {
                chunkCollision.grounded = false;
            }
        } else {
            vel.vel = glm::dvec3(0);
            pos.pos = collider.lastValidPos;
        }

        collider.lastValidPos = pos.pos;
    }
}

void Physics::runYTeleport(entt::registry &registry, ChunkManagement &chunkManagement) {
    auto yTeleportBoxView = registry.view<Components::Position, Components::ChunkPosition, Components::VoxelCollision, Components::YTeleport, Components::BoxCollider>();
    for (auto entity: yTeleportBoxView) {
        auto &pos = yTeleportBoxView.get<Components::Position>(entity);
        auto &chunkPos = yTeleportBoxView.get<Components::ChunkPosition>(entity);
        auto &voxelCollision = yTeleportBoxView.get<Components::VoxelCollision>(entity);
        auto &yTeleport = yTeleportBoxView.get<Components::YTeleport>(entity);
        auto &boxCollider = yTeleportBoxView.get<Components::BoxCollider>(entity);

        if (((yTeleport.requireGrounded && voxelCollision.grounded) || !yTeleport.requireGrounded) && chunkManagement.isChunkDataLoaded(registry, chunkPos.x, chunkPos.y, chunkPos.z)) {
            if (checkRectangularCollision(registry, chunkManagement, pos.pos, boxCollider)) {
                glm::dvec3 posPreMod = pos.pos;
                pos.pos.y = std::ceil(pos.pos.y);
                if (checkRectangularCollision(registry, chunkManagement, pos.pos, boxCollider)) {
                    bool success = false;
                    for (int i = 1; i < yTeleport.maxSteps; ++i) {
                        pos.pos = posPreMod;
                        pos.pos.y = std::ceil(pos.pos.y);
                        pos.pos.y += i * yTeleport.stepSize;
                        if (!checkRectangularCollision(registry, chunkManagement, pos.pos, boxCollider)) {
                            success = true;
                            goto exitLoop;
                        }
                    }
                    exitLoop:
                    if (!success) {
                        pos.pos = posPreMod;
                    } else {
                        voxelCollision.grounded = true;
                    }
                } else {
                    pos.pos = posPreMod;
                }
            }
        }
    }

    auto yTeleportPointView = registry.view<Components::Position, Components::ChunkPosition, Components::VoxelCollision, Components::YTeleport, Components::PointCollider>();
    for (auto entity: yTeleportPointView) {
        auto &pos = yTeleportPointView.get<Components::Position>(entity);
        auto &chunkPos = yTeleportPointView.get<Components::ChunkPosition>(entity);
        auto &voxelCollision = yTeleportPointView.get<Components::VoxelCollision>(entity);
        auto &yTeleport = yTeleportBoxView.get<Components::YTeleport>(entity);
        auto &pointCollider = yTeleportPointView.get<Components::PointCollider>(entity);

        if (((yTeleport.requireGrounded && voxelCollision.grounded) || !yTeleport.requireGrounded) && chunkManagement.isChunkDataLoaded(registry, chunkPos.x, chunkPos.y, chunkPos.z)) {
            if (checkPointCollision(registry, chunkManagement, pos.pos)) {
                glm::dvec3 posPreMod = pos.pos;
                bool success = false;
                for (int i = 1; i < yTeleport.maxSteps; ++i) {
                    pos.pos = posPreMod;
                    pos.pos.y += i * yTeleport.stepSize;
                    if (!checkPointCollision(registry, chunkManagement, pos.pos)) {
                        success = true;
                        goto exitLoop2;
                    }
                }
                exitLoop2:
                if (!success) {
                    pos.pos = posPreMod;
                }
            }
        }
    }
}

