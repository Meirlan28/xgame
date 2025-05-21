#pragma once
#include <string>
#include <glad/glad.h>

class Texture {
public:
    Texture(const std::string& path, bool flip = false);
    ~Texture();

    void Bind(GLenum textureUnit = GL_TEXTURE0) const;
    void Unbind() const;
    unsigned int GetID() const;

private:
    unsigned int textureID;
};
