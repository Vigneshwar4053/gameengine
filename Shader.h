#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // Use GLAD instead of GLEW
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void use() const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& value) const; // ✅ Add setVec3()

private:
    GLuint ID;
    void checkCompileErrors(GLuint shader, const std::string& type);
};

#endif
