#include "texture2d.hpp"

namespace Phi
{
    // Generate texture constructor
    Texture2D::Texture2D(int width, int height, GLint internalFormat, GLint format, GLenum type, GLenum minFilter, GLenum magFilter, bool mipmap)
    {
        // Generate the texture object
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Apply default texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

        // Specify the texture details
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
        if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        this->width = width;
        this->height = height;
    }

    // Load from file constructor
    Texture2D::Texture2D(const std::string& texPath, GLenum minFilter, GLenum magFilter, bool mipmap)
    {
        // Generate the texture object
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Default default texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

        // Load the image data
        int width, height, channelCount;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(texPath.c_str(), &width, &height, &channelCount, STBI_rgb_alpha);
        if (data)
        {
            // Send image data to texture target
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "ERROR: Couldn't load file: " << texPath.c_str() << std::endl;
            stbi_image_free(data);
        }

        this->width = width;
        this->height = height;

        // Unbind
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Destructor
    Texture2D::~Texture2D()
    {
        glDeleteTextures(1, &textureID);
    }

    // Bind this texture to GL_TEXTURE_2D on a given texture unit
    // NOTE: This leaves texUnit as the currently active texture unit on return
    void Texture2D::Bind(int texUnit) const
    {
        // Ensure valid texture unit is supplied
        // OpenGL 3.x requires a minimum of 16 units per stage
        if (texUnit < 0 || texUnit > 15)
        {
            std::cout << "ERROR: Invalid texture unit passed to Texture2D::Bind" << std::endl;
            return;
        }

        // Set active unit and bind texture
        glActiveTexture(GL_TEXTURE0 + texUnit);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
}