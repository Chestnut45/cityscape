#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "../../thirdparty/glew/include/GL/glew.h" // OpenGL types / functions

// Cubemap data
class Cubemap
{
    // Interface
    public:
        Cubemap(const std::vector<std::string>& faces);
        ~Cubemap();

        // Delete copy constructor/assignment
        Cubemap(const Cubemap&) = delete;
        Cubemap& operator=(const Cubemap&) = delete;

        // Delete move constructor/assignment
        Cubemap(Cubemap&& other) = delete;
        void operator=(Cubemap&& other) = delete;

        // Bind this cubemap's texture to GL_TEXTURE_CUBE_MAP on a given texture unit
        void Bind(int texUnit = 0) const;
    
    private:
        // OpenGL objects
        GLuint textureID;
};