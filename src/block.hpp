#pragma once

#include "../thirdparty/glew/include/GL/glew.h"

class Block
{
    public:
        Block();
        ~Block();
    
    private:
        inline static unsigned int refCount = 0;
        inline static GLuint groundTexture = 0;
};