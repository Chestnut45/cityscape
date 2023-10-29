#include "texture2d.hpp"

// Constructor
Texture2D::Texture2D(const std::string& texPath)
{
    // Generate the texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Load the image data
    int width, height, channelCount;
    unsigned char* data = stbi_load(texPath.c_str(), &width, &height, &channelCount, STBI_rgb_alpha);
    if (data)
    {
        // Send image data to texture target
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "ERROR: Couldn't load file: " << texPath.c_str() << std::endl;
        stbi_image_free(data);
    }

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
void Texture2D::Bind(int texUnit)
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