#ifndef MESH_H
#define MESH_H
#include "Shader.h"

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <assimp/scene.h> // Include Assimp headers
#include <assimp/material.h> // Include Assimp headers

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    GLuint id;
    std::string type;
    aiString path; // Now aiString is recognized
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
    void Draw(Shader& shader);

private:
    GLuint VAO, VBO, EBO;
    void setupMesh();
};

#endif