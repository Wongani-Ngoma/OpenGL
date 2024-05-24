#include "texture.h"
#include <iostream>

Texture::Texture() {
    generate();
}

Texture::Texture(std::string dir, std::string path, aiTextureType type) : dir(dir), path(path), type(type) {
}

void Texture::generate() {
    glGenTextures(1, &id);
}   

void Texture::load(bool flip) {
    stbi_set_flip_vertically_on_load(flip);

    int width, height, nChannels;
    //std::string p = (dir + "//" + path); // use this when the .obj contains relative texture directories
    std::string p = (path); // use path only cause blender stores absolute texture directories in exported .obj files,
    unsigned char* data = stbi_load(p.c_str(), &width, &height, &nChannels, 0);

    GLenum colorMode = GL_RGB; //This is the most common so we dont need to put it in te switch statement
    switch (nChannels) {
    case 1:
        colorMode = GL_RED;
        break;
    case 4:
        colorMode = GL_RGBA;
        break;
    }

    if (data) {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, colorMode, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }
    else {
        std::cout << "Image " << path << " not loaded" << "\n";
    }
    stbi_image_free(data);
}

void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, id);
}
