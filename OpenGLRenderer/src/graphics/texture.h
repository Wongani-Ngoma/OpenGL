#pragma once
#ifndef TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/scene.h>

#include <stb/stb_image.h>

class Texture {
public:
    Texture();
    Texture(std::string dir, std::string path, aiTextureType type);

    void generate();
    void load(bool flip = false);

    void bind(); // Creates an object of the texture in memory

    std::string path;
    unsigned int id;
    std::string dir;
    aiTextureType type;
private:
    
};

#endif // !TEXTURE_H



