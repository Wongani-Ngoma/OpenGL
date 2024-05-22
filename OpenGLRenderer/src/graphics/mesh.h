#pragma once
#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <vector>
#include <glm/glm.hpp>
#include "shader.h"
#include "texture.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;

    static std::vector<struct Vertex> genList(float* vertices, int novertices);
};
typedef struct Vertex Vertex;

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indicies;
    unsigned int VAO; //Sord of like a container, containing pointers to data buffers in VBOs
    
    aiColor4D diffuse;
    aiColor4D specular;

    std::vector<Texture> textures;

    Mesh();
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indicies, std::vector<Texture> textures = {}, bool noTex = true);
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indicies, aiColor4D diffuse, aiColor4D spec);

    void render(Shader shader);

    void cleanup();

private:
    bool noTex;
    unsigned int VBO; //Hold actual vertex data
    unsigned int EBO; //Contains indicies, each of which references to a group of vertices, eg index 1 might reference to the top left position on the screen (-0.5, 0.5, 0.0)

    void setup();

};
#endif // !MESH_H



