#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/EntityManager.h"
#include "core/EntityManager.h"
#include "core/camera.h"
#include "core/Logger.h"

#include "systems/CollisionSystem.h"
#include "systems/MovementSystem.h"
#include "systems/PhysicsSystem.h"

#include "utils/ShaderProgram.h"
#include "utils/TextureProgram.h"
#include "stb_image.h"


// Объявления функций
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, MovementSystem& movement, EntityID player, Camera& camera);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Константы
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Камера
Camera camera(glm::vec3(0.0f, 20.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Тайминги
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Позиции объектов
glm::vec3 objectPositions[] = {
    glm::vec3(0.0f,  0.0f,  -1.0f),
    glm::vec3(1.3f,  0.0f,  -2.0f),
    glm::vec3(-1.3f,  0.0f, -3.0f),
    glm::vec3(1.3f,  0.0f,  -4.0f),
    glm::vec3(-1.3f,  0.0f, -5.0f),
    glm::vec3(1.3f,  0.0f, -6.0f),
    glm::vec3(-1.3f,  0.0f, -7.0f),
    glm::vec3(1.3f,  0.0f, -8.0f),
    glm::vec3(-1.3f,  0.0f, -9.0f),
    glm::vec3(1.3f,  0.0f, -10.0f),
    glm::vec3(0.0f, -5.0f,  5.0f) // Пол
};

// Компонент рендеринга
struct RenderComponent {
    glm::vec3 scale;
    float rotationAngle = 0.0f; // Для вращения кубов
    glm::vec3 rotationAxis = glm::vec3(1.0f, 0.3f, 0.5f);
};

// Система рендеринга
class RenderSystem {
public:
    RenderSystem(Shader& shader, unsigned int vao, Texture& diffuse, Texture& specular, Texture& emission)
        : shader(shader), VAO(vao), diffuse(diffuse), specular(specular), emission(emission) {}

    void update(EntityManager& manager, Camera& camera, float aspectRatio) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setVec3("light.position", camera.Position);
        shader.setVec3("light.direction", camera.Front);
        shader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
        shader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
        shader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
        shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        shader.setFloat("light.constant", 1.0f);
        shader.setFloat("light.linear", 0.09f);
        shader.setFloat("light.quadratic", 0.032f);
        shader.setFloat("material.shininess", 32.0f);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        diffuse.Bind(GL_TEXTURE0);
        shader.setInt("material.diffuse", 0);
        specular.Bind(GL_TEXTURE1);
        shader.setInt("material.specular", 1);
        emission.Bind(GL_TEXTURE2);
        shader.setInt("material.emission", 2);

        glBindVertexArray(VAO);
        for (auto entity : manager.getEntitiesWith<TransformComponent, RenderComponent>()) {
            auto& transform = manager.getComponent<TransformComponent>(entity);
            auto& render = manager.getComponent<RenderComponent>(entity);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, transform.position);
            if (render.rotationAngle != 0.0f) {
                model = glm::rotate(model, glm::radians(render.rotationAngle), render.rotationAxis);
            }
            model = glm::scale(model, render.scale);
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

private:
    Shader& shader;
    unsigned int VAO;
    Texture& diffuse;
    Texture& specular;
    Texture& emission;
};

int main() {
    // Инициализация GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Physics ECS", nullptr, nullptr);
    if (!window) {
        Logger::log("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Инициализация GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Logger::log("Failed to initialize GLAD");
        glfwTerminate();
        return -1;
    }

    // Настройки OpenGL
    glEnable(GL_DEPTH_TEST);

    // Шейдеры
    Shader cube("assets/shaders/cube.vs", "assets/shaders/cube.fs");

    // Вершины куба
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    };

    // Настройка VAO и VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Атрибуты вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Текстуры
    Texture diffuse("assets/textures/diffuse.png");
    Texture specular("assets/textures/specular.png");
    Texture emission("assets/textures/emission.png");

    // Инициализация ECS
    EntityManager manager;
    PhysicsSystem physics(-20.0f, -30.0f, -100.0f, 1.0f);
    CollisionSystem collisions;
    MovementSystem movement(8.0f);
    RenderSystem render(cube, VAO, diffuse, specular, emission);

    // Создание игрока
    EntityID player = manager.createEntity();
    manager.addComponent(player, TransformComponent{ glm::vec3(0.0f, 5.0f, -1.0f) });
    manager.addComponent(player, PhysicsComponent{});
    manager.addComponent(player, ColliderComponent{ glm::vec3(0.3f, 0.5f, 0.2f), 0.5f });
    manager.addComponent(player, MovementComponent{ glm::vec3(0), camera.MovementSpeed, 3.0f, 3.0f, glm::vec3(0) });

    // Создание игровых объектов
    for (size_t i = 0; i < 10; ++i) {
        EntityID obj = manager.createEntity();
        manager.addComponent(obj, TransformComponent{ objectPositions[i] });
        manager.addComponent(obj, ColliderComponent{ glm::vec3(0.5f), 0.5f });
        manager.addComponent(obj, RenderComponent{ glm::vec3(1.0f)});
        collisions.addStaticCollider(Collider(objectPositions[i], glm::vec3(0.5f)));
    }
    // Пол
    EntityID floor = manager.createEntity();
    manager.addComponent(floor, TransformComponent{ objectPositions[10] });
    manager.addComponent(floor, ColliderComponent{ glm::vec3(5.0f), 5.0f });
    manager.addComponent(floor, RenderComponent{ glm::vec3(10.0f), 0.0f });
    collisions.addStaticCollider(Collider(objectPositions[10], glm::vec3(10.0f)));


    // Установка точки возрождения
    physics.setSpawnPoint(glm::vec3(0.0f, 2.0f, 5.0f));

    // Связка MovementSystem с EntityManager
    movement.setManager(manager);

    // Проверка ошибок OpenGL
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        Logger::log("OpenGL error after initialization: " + std::to_string(err));
    }

    // Цикл рендеринга
    while (!glfwWindowShouldClose(window)) {
        // Время
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        deltaTime = glm::min(deltaTime, 0.016f);
        lastFrame = currentFrame;

        // Ввод
        processInput(window, movement, player, camera);

        // Обновление систем
        physics.update(manager, deltaTime);
        movement.update(manager, deltaTime);
        collisions.update(manager, deltaTime);

        // Синхронизация камеры
        camera.Position = manager.getComponent<TransformComponent>(player).position;

        // Рендеринг
        render.update(manager, camera, (float)SCR_WIDTH / (float)SCR_HEIGHT);

        // Проверка ошибок OpenGL
        while ((err = glGetError()) != GL_NO_ERROR) {
            Logger::log("OpenGL error during rendering: " + std::to_string(err));
        }

        // Обмен буферов
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, MovementSystem& movement, EntityID player, Camera& camera) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Сбор направления движения
    glm::vec3 direction(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) direction += camera.Front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) direction -= camera.Front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) direction -= camera.Right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) direction += camera.Right;
    if (glm::length(direction) > 0.001f) direction = glm::normalize(direction);
    movement.setMovementDirection(player, direction);


    bool isSpacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (isSpacePressed) movement.jump(player);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}