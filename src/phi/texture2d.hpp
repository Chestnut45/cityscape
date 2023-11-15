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
        Texture2D(int width, int height, GLint internalFormat, GLint format, GLenum type, GLenum minFilter, GLenum magFilter, bool mipmap = false);
        Texture2D(const std::string& texPath, GLenum minFilter, GLenum magFilter, bool mipmap = false);
        ~Texture2D();

        // Delete copy constructor/assignment
        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        // Delete move constructor/assignment
        Texture2D(Texture2D&& other) = delete;
        void operator=(Texture2D&& other) = delete;

        // Binding operations
        void Bind(int texUnit = 0) const;

        // Accessors
        inline GLuint GetID() const { return textureID; };
        inline int GetWidth() const { return width; };
        inline int GetHeight() const { return height; };
    
    // Data / implementation
    private:
        GLuint textureID;
        int width = 0;
        int height = 0;
};