#include "utils/TextureProgram.h"
#include "stb_image.h"
#include <iostream>
#include <fstream>

Texture::Texture(const std::string& path, bool flip) : textureID(0) {
    // Проверка существования файла
    std::ifstream file(path);
    if (!file.good()) {
        std::cerr << "Texture file not found: " << path << std::endl;
        return;
    }
    file.close();

    // Генерация текстуры
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Параметры текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Загрузка изображения
    stbi_set_flip_vertically_on_load(flip);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = GL_RGB;
        GLenum internalFormat = GL_RGB;

        if (nrChannels == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA;
        }
        else if (nrChannels == 1) {
            format = GL_RED;
            internalFormat = GL_R8;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture: " << path << " - " << stbi_failure_reason() << std::endl;
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }

    stbi_image_free(data);
}

Texture::~Texture() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

void Texture::Bind(GLenum textureUnit) const {
    if (textureID == 0) return;
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int Texture::GetID() const {
    return textureID;
}