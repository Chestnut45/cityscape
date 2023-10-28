#include "cubemap.hpp"

// Include stb image header
// NOTE: Do not define STB_IMAGE_IMPLEMENTATION here, wolf already defines it in W_Texture.cpp
#include "../../wolf/stb_image.h"

// Constructor
Cubemap::Cubemap(const std::vector<std::string>& faces)
{
    // Generate resources
    glGenTextures(1, &textureID);

    // Bind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // Load the face files from the list of filenames
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        // Load the face image data
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

        if (data)
        {
            // Send image data to texture target
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            printf("ERROR: Couldn't load file %s\n", faces[i].c_str());
            stbi_image_free(data);
        }
    }

    // Set the texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Unbind before returning
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// Destructor
Cubemap::~Cubemap()
{
    glDeleteTextures(1, &textureID);
}

// Bind this cubemap's texture to GL_TEXTURE_CUBE_MAP on a given texture unit
// NOTE: This leaves texUnit as the currently active texture unit on return
void Cubemap::Bind(int texUnit)
{
    // Ensure valid texture unit is supplied
    // OpenGL 3.x requires a minimum of 16 units per stage
    if (texUnit < 0 || texUnit > 15)
    {
        std::cout << "ERROR: Invalid texture unit passed to Cubemap::Bind" << std::endl;
        return;
    }

    // Set active unit and bind texture
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}