#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h> // GLFW get time function
#include "../thirdparty/glew/include/GL/glew.h" // OpenGL functions

// Shader management class
class Shader
{
    public:
        // Initialization
        Shader();
        ~Shader();

        // Delete copy constructor/assignment
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        // Delete move constructor/assignment
        Shader(Shader&& other) = delete;
        void operator=(Shader&& other) = delete;

        // Methods
        void Use() const;
        bool LoadShaderSource(GLenum stage, const std::string& sourcePath);
        bool Link();

        // Uniform / binding manipulation
        void BindUniformBlock(const std::string& blockName, GLuint bindingPoint);

        // Accessors
        inline GLuint GetProgramID() { return programID; };
    
    // Implementation
    private:
        // Identifiers
        GLuint programID;

        // Verification vars
        GLint success;
        GLchar infoLog[512];

        // IDs of individual shaders
        std::vector<GLuint> shaders;
};