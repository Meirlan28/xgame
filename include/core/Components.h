#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



struct TransformComponent {
    glm::vec3 position;
    glm::vec3 rotation; // Для будущей поддержки
    glm::vec3 scale = glm::vec3(1.0f);
};

struct PhysicsComponent {
    glm::vec3 velocity;
    bool onGround = true;
    float mass = 1.0f;
    float gravityMultiplier = 1.0f;
};

struct ColliderComponent {
    glm::vec3 halfExtents;
    float maxExtent;
};

struct MovementComponent {
    glm::vec3 groundVelocity;
    float movementSpeed;
    float acceleration;
    float friction;
    glm::vec3 movementDirection;
};