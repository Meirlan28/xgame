#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/Components.h"
#include "core/EntityManager.h"

struct Collider {
    glm::vec3 center;
    glm::vec3 halfExtents;
    float maxExtent;

    Collider(const glm::vec3& position, float size) : center(position), halfExtents(size), maxExtent(size) {}
    Collider(const glm::vec3& position, const glm::vec3& scale) : center(position) {
        halfExtents = scale * 0.5f;
        maxExtent = glm::length(halfExtents);
    }
    glm::vec3 getClosestPoint(const glm::vec3& point) const {
        glm::vec3 closest;
        closest.x = glm::clamp(point.x, center.x - halfExtents.x, center.x + halfExtents.x);
        closest.y = glm::clamp(point.y, center.y - halfExtents.y, center.y + halfExtents.y);
        closest.z = glm::clamp(point.z, center.z - halfExtents.z, center.z + halfExtents.z);
        return closest;
    }
};



class CollisionSystem {
public:
    void addStaticCollider(const Collider& collider) {
        staticColliders.push_back(collider);
    }

    void update(EntityManager& manager, float deltaTime) {
        for (auto entity : manager.getEntitiesWith<TransformComponent, ColliderComponent, PhysicsComponent>()) {
            auto& transform = manager.getComponent<TransformComponent>(entity);
            auto& collider = manager.getComponent<ColliderComponent>(entity);
            auto& physics = manager.getComponent<PhysicsComponent>(entity);

            glm::vec3 oldPosition = transform.position;
            glm::vec3 proposedPosition = transform.position;

            // Учитываем горизонтальное движение из MovementComponent, если есть
            if (manager.hasComponent<MovementComponent>(entity)) {
                auto& movement = manager.getComponent<MovementComponent>(entity);
                proposedPosition += movement.groundVelocity * deltaTime;
            }

            bool collisionDetected = false;
            auto nearbyColliders = getNearbyColliders(proposedPosition, collider);
            Logger::log("Entity " + std::to_string(entity) + " nearby colliders: " + std::to_string(nearbyColliders.size()));

            for (const auto& otherCollider : nearbyColliders) {
                if (checkCollision(proposedPosition, collider.halfExtents, otherCollider)) {
                    collisionDetected = true;

                    // Вычисляем пересечение
                    glm::vec3 cameraMin = proposedPosition - collider.halfExtents;
                    glm::vec3 cameraMax = proposedPosition + collider.halfExtents;
                    glm::vec3 colliderMin = otherCollider.center - otherCollider.halfExtents;
                    glm::vec3 colliderMax = otherCollider.center + otherCollider.halfExtents;

                    // Находим минимальное выталкивание
                    float overlapX = std::min(cameraMax.x - colliderMin.x, colliderMax.x - cameraMin.x);
                    float overlapY = std::min(cameraMax.y - colliderMin.y, colliderMax.y - cameraMin.y);
                    float overlapZ = std::min(cameraMax.z - colliderMin.z, colliderMax.z - cameraMin.z);

                    // Определяем направление выталкивания
                    glm::vec3 normal(0.0f);
                    float minOverlap = std::min({ overlapX, overlapY, overlapZ });

                    if (minOverlap == overlapX) {
                        normal.x = (proposedPosition.x > otherCollider.center.x) ? 1.0f : -1.0f;
                    }
                    else if (minOverlap == overlapY) {
                        normal.y = (proposedPosition.y > otherCollider.center.y) ? 1.0f : -1.0f;
                    }
                    else {
                        normal.z = (proposedPosition.z > otherCollider.center.z) ? 1.0f : -1.0f;
                    }

                    // Выталкивание
                    proposedPosition += -normal * minOverlap;

                    // Проверка, стоит ли на поверхности
                    if (normal.y > 0.1f && proposedPosition.y > otherCollider.center.y + otherCollider.halfExtents.y) {
                        physics.onGround = true;
                        physics.velocity.y = 0.0f;
                        proposedPosition.y = otherCollider.center.y + otherCollider.halfExtents.y + collider.halfExtents.y + 0.001f;
                        Logger::log("Entity " + std::to_string(entity) + " landed on ground: normal.y = " + std::to_string(normal.y) +
                            ", y = " + std::to_string(proposedPosition.y));
                        Logger::log("Collision with collider at (" + std::to_string(otherCollider.center.x) + ", " +
                            std::to_string(otherCollider.center.y) + "), normal=(" + std::to_string(normal.x) + ", " +
                            std::to_string(normal.y) + ", " + std::to_string(normal.z) + ")");
                    }

                    if (abs(normal.x) > 0.7f) {
                        proposedPosition.x = transform.position.x; // Отменить движение по X
                    }
                    if (abs(normal.z) > 0.7f) {
                        proposedPosition.z = transform.position.z; // Отменить движение по Z
                    }

                }
            }

            if (!collisionDetected) {
                physics.onGround = false;
                Logger::log("Entity " + std::to_string(entity) + " no collision detected, onGround = false");
            }

            transform.position = proposedPosition;
        }
    }

private:
    std::vector<Collider> staticColliders;

    std::vector<Collider> getNearbyColliders(const glm::vec3& position, const ColliderComponent& collider) const {
        std::vector<Collider> nearby;
        float cameraMaxExtent = collider.maxExtent;
        for (const auto& otherCollider : staticColliders) {
            float distance = glm::distance(position, otherCollider.center);
            float threshold = cameraMaxExtent + otherCollider.maxExtent + 2.0f;
            if (distance < threshold) {
                nearby.push_back(otherCollider);
            }
            Logger::log("Collider check: distance = " + std::to_string(distance) + ", threshold = " + std::to_string(threshold) +
                ", collider center = (" + std::to_string(otherCollider.center.x) + ", " +
                std::to_string(otherCollider.center.y) + ", " + std::to_string(otherCollider.center.z) + ")");
        }
        return nearby;
    }

    bool checkCollision(const glm::vec3& point, const glm::vec3& cameraHalfExtents, const Collider& collider) const {
        glm::vec3 cameraMin = point - cameraHalfExtents;
        glm::vec3 cameraMax = point + cameraHalfExtents;
        glm::vec3 colliderMin = collider.center - collider.halfExtents;
        glm::vec3 colliderMax = collider.center + collider.halfExtents;

        bool collision = (cameraMin.x <= colliderMax.x && cameraMax.x >= colliderMin.x) &&
            (cameraMin.y <= colliderMax.y && cameraMax.y >= colliderMin.y) &&
            (cameraMin.z <= colliderMax.z && cameraMax.z >= colliderMin.z);

        if (collision) {
            Logger::log("Collision detected: entity at (" + std::to_string(point.x) + ", " + std::to_string(point.y) + ", " +
                std::to_string(point.z) + "), collider center = (" + std::to_string(collider.center.x) + ", " +
                std::to_string(collider.center.y) + ", " + std::to_string(collider.center.z) + ")");
        }

        return collision;
    }
};
