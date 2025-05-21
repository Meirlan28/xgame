//#pragma once
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include "ShaderProgram.h"
//#include "../Texture.h"
//#include "../physics/EntityManager.h"
//#include "../camera.h"
//
//class RenderSystem {
//public:
//    RenderSystem(Shader& shader, unsigned int vao, Texture& diffuse, Texture& specular, Texture& emission)
//        : shader(shader), VAO(vao), diffuse(diffuse), specular(specular), emission(emission) {}
//
//    void update(EntityManager& manager, Camera& camera, float aspectRatio) {
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        shader.use();
//        shader.setVec3("light.position", camera.Position);
//        shader.setVec3("light.direction", camera.Front);
//        shader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
//        shader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
//        shader.setVec3("viewPos", camera.Position);
//        shader.setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
//        shader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
//        shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
//        shader.setFloat("light.constant", 1.0f);
//        shader.setFloat("light.linear", 0.09f);
//        shader.setFloat("light.quadratic", 0.032f);
//        shader.setFloat("material.shininess", 32.0f);
//
//        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 100.0f);
//        glm::mat4 view = camera.GetViewMatrix();
//        shader.setMat4("projection", projection);
//        shader.setMat4("view", view);
//
//        diffuse.Bind(GL_TEXTURE0);
//        shader.setInt("material.diffuse", 0);
//        specular.Bind(GL_TEXTURE1);
//        shader.setInt("material.specular", 1);
//        emission.Bind(GL_TEXTURE2);
//        shader.setInt("material.emission", 2);
//
//        glBindVertexArray(VAO);
//        for (auto entity : manager.getEntitiesWith<TransformComponent, RenderComponent>()) {
//            auto& transform = manager.getComponent<TransformComponent>(entity);
//            auto& render = manager.getComponent<RenderComponent>(entity);
//
//            glm::mat4 model = glm::mat4(1.0f);
//            model = glm::translate(model, transform.position);
//            if (render.rotationAngle != 0.0f) {
//                model = glm::rotate(model, glm::radians(render.rotationAngle), render.rotationAxis);
//            }
//            model = glm::scale(model, render.scale);
//            shader.setMat4("model", model);
//            glDrawArrays(GL_TRIANGLES, 0, 36);
//        }
//    }
//
//private:
//    Shader& shader;
//    unsigned int VAO;
//    Texture& diffuse;
//    Texture& specular;
//    Texture& emission;
//};