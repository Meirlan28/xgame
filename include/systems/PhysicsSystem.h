#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/Components.h"
#include "core/EntityManager.h"
#include "core/Logger.h"

class PhysicsSystem {
public:
    PhysicsSystem(float gravity = -9.81f, float terminalVelocity = -50.0f, float fallThreshold = -100.0f, float fallMultiplier = 2.0f)
        : gravity(gravity), terminalVelocity(terminalVelocity), fallThreshold(fallThreshold), fallMultiplier(fallMultiplier) {}

    void setSpawnPoint(const glm::vec3& spawnPoint) {
        this->spawnPoint = spawnPoint;
    }

    void update(EntityManager& manager, float deltaTime) {
        for (auto entity : manager.getEntitiesWith<PhysicsComponent, TransformComponent>()) {
            auto& physics = manager.getComponent<PhysicsComponent>(entity);
            auto& transform = manager.getComponent<TransformComponent>(entity);

            // Проверка падения
            if (transform.position.y < fallThreshold) {
                transform.position = spawnPoint;
                physics.velocity = glm::vec3(0.0f);
                physics.onGround = false;
                Logger::log("Entity " + std::to_string(entity) + " fell too far! Respawned at (" +
                    std::to_string(spawnPoint.x) + ", " + std::to_string(spawnPoint.y) + ", " +
                    std::to_string(spawnPoint.z) + ")");
                continue;
            }

            // Применение гравитации
            if (!physics.onGround) {
                float gravityForce = gravity * (physics.velocity.y < 0.0f ? fallMultiplier : 1.0f);
                physics.velocity.y += gravityForce * deltaTime;
                if (physics.velocity.y < terminalVelocity) physics.velocity.y = terminalVelocity;
                Logger::log("Entity " + std::to_string(entity) + " applying gravity: velocityY = " + std::to_string(physics.velocity.y));
            }
            else {
                physics.velocity.y = 0.0f;
            }

            // Обновление позиции по вертикали
            transform.position.y += physics.velocity.y * deltaTime;
        }
    }

private:
    float gravity;
    float terminalVelocity;
    float fallThreshold;
    float fallMultiplier;
    glm::vec3 spawnPoint = glm::vec3(0.0f, 2.0f, 0.0f);
};