#include "sky.hpp"

// Vertex layout: (x, y, z) position only
static const GLfloat SKYBOX_DATA[] =
{
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
};

// Contruct a sky component
// daySkyboxPath, nightSkyboxPath: path to a folder containing skybox face images
// skyVS, skyFS: paths to sky vertex / fragment shader sources
// NOTE: Shader source should contain 2 samplerCubes named "dayCube" and "nightCube"
Sky::Sky(const std::string& daySkyboxPath, const std::string& nightSkyboxPath, const std::string& skyVS, const std::string& skyFS)
    : dayBox({
        daySkyboxPath + "/right.png",
        daySkyboxPath + "/left.png",
        daySkyboxPath + "/top.png",
        daySkyboxPath + "/bottom.png",
        daySkyboxPath + "/front.png",
        daySkyboxPath + "/back.png"
    }),
    
    nightBox({
        nightSkyboxPath + "/right.png",
        nightSkyboxPath + "/left.png",
        nightSkyboxPath + "/top.png",
        nightSkyboxPath + "/bottom.png",
        nightSkyboxPath + "/front.png",
        nightSkyboxPath + "/back.png"
    })
{
    // Load source for skybox shader and link
    skyShader.LoadShaderSource(GL_VERTEX_SHADER, skyVS);
    skyShader.LoadShaderSource(GL_FRAGMENT_SHADER, skyFS);
    skyShader.Link();

    // Bind samplers to proper texture units
    skyShader.Use();
    skyShader.SetUniform("dayCube", (int)SkyTextureUnit::Day);
    skyShader.SetUniform("nightCube", (int)SkyTextureUnit::Night);
    skyShader.BindUniformBlock("CameraBlock", 0);

    // If first instance, initialize static resources
    if (refCount == 0)
    {
        // Create VBO for skybox data
        skyboxVBO = new GPUBuffer(sizeof(SKYBOX_DATA), BufferType::StaticVertex, SKYBOX_DATA);
        skyboxVBO->Bind(GL_ARRAY_BUFFER);

        // Describe vertex attributes
        skyboxVAO = new VertexAttributes();
        skyboxVAO->Bind();
        skyboxVAO->Add(3, GL_FLOAT);
        skyboxVAO->Unbind();
    }

    refCount++;
}

// Cleanup
Sky::~Sky()
{
    refCount--;

    // Non-static () resources manage themselves

    // If last instance, cleanup static resources
    if (refCount == 0)
    {
        delete skyboxVBO;
        delete skyboxVAO;
    }
}

// Set normalized time of day (0 = day, 1 = night)
// This value is used by the shader to interpolate between
// the day skybox and the night skybox
void Sky::SetTOD(float time)
{
    skyShader.Use();
    skyShader.SetUniform("time", time);
}

// Renders the sky
void Sky::Draw()
{
    skyShader.Use();

    // Change depth function so max distance still renders
    glDepthFunc(GL_LEQUAL);

    // TODO: Bind VertexAttributes
    skyboxVAO->Bind();

    // Bind day / night cubemaps to texture units
    dayBox.Bind((int)SkyTextureUnit::Day);
    nightBox.Bind((int)SkyTextureUnit::Night);

    // Draw, unbind, and reset depth function
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}