#pragma once

#include <iostream>
#include <string>

#include "../../thirdparty/glew/include/GL/glew.h" // OpenGL types / functions
#include "../../wolf/stb_image.h"

// 2D texture RAII wrapper
class Texture2D
{
    // Interface
    public:
        Texture2D(const std::string& texPath);
        ~Texture2D();

        // Delete copy constructor/assignment
        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        // Delete move constructor/assignment
        Texture2D(Texture2D&& other) = delete;
        void operator=(Texture2D&& other) = delete;

        // Binding operations
        void Bind(int texUnit = 0);
    
    // Data / implementation
    private:
        GLuint textureID;
};