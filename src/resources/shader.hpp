#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "../../thirdparty/glew/include/GL/glew.h" // OpenGL types / functions

// Shader management class
class Shader
{
    public:
        Shader();
        ~Shader();

        // Loading / compiling
        bool LoadShaderSource(GLenum stage, const std::string& sourcePath);
        bool Link();

        // Delete copy constructor/assignment
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        // Delete move constructor/assignment
        Shader(Shader&& other) = delete;
        void operator=(Shader&& other) = delete;

        // Set as the active program
        void Use() const;

        // Uniform / binding manipulation
        // NOTE: All calls to SetUniform() are only valid following a call to Use()!
        void BindUniformBlock(const std::string& blockName, GLuint bindingPoint);
        void SetUniform(const std::string& name, int value);
        void SetUniform(const std::string& name, float value);

        // Accessors
        inline GLuint GetProgramID() const { return programID; };
    
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