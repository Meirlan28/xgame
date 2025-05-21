#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/Components.h"
#include "core/EntityManager.h"
#include "core/Logger.h"

class MovementSystem {
public:
    MovementSystem(float jumpStrength = 5.0f) : jumpStrength(jumpStrength) {}

    void setMovementDirection(EntityID entity, const glm::vec3& direction) {
        if (manager && manager->hasComponent<MovementComponent>(entity)) {
            manager->getComponent<MovementComponent>(entity).movementDirection = direction;
        }
    }

    void jump(EntityID entity) {
        if (manager && manager->hasComponent<PhysicsComponent>(entity) && manager->hasComponent<MovementComponent>(entity)) {
            auto& physics = manager->getComponent<PhysicsComponent>(entity);
            if (physics.onGround) {
                physics.velocity.y = jumpStrength;
                physics.onGround = false;
                Logger::log("Entity " + std::to_string(entity) + " jump triggered: velocityY = " + std::to_string(physics.velocity.y));
            }
            else {
                Logger::log("Entity " + std::to_string(entity) + " jump failed: not on ground");
            }
        }
    }

    void setManager(EntityManager& mgr) {
        manager = &mgr;
    }

    void update(EntityManager& manager, float deltaTime) {
        for (auto entity : manager.getEntitiesWith<MovementComponent, TransformComponent>()) {
            auto& movement = manager.getComponent<MovementComponent>(entity);
            auto& transform = manager.getComponent<TransformComponent>(entity);

            glm::vec3 targetVelocity = movement.movementDirection * movement.movementSpeed;
            movement.groundVelocity += (targetVelocity - movement.groundVelocity) * movement.acceleration * deltaTime;
            if (glm::length(movement.movementDirection) < 0.001f) {
                movement.groundVelocity -= movement.groundVelocity * movement.friction * deltaTime;
                if (glm::length(movement.groundVelocity) < 0.01f) movement.groundVelocity = glm::vec3(0.0f);
            }

            // √оризонтальное движение будет обработано в CollisionSystem
        }
    }

private:
    float jumpStrength;
    EntityManager* manager = nullptr;
};
