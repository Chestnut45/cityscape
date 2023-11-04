#include "sky.hpp"

// Vertex layout: (x, y, z) position only
static const VertexPos SKYBOX_VERTICES[] =
{
    {-1.0f,  1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f},
    {1.0f, -1.0f, -1.0f},
    {1.0f, -1.0f, -1.0f},
    {1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},

    {-1.0f, -1.0f,  1.0f},
    {-1.0f, -1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f,  1.0f},
    {-1.0f, -1.0f,  1.0f},

    {1.0f, -1.0f, -1.0f},
    {1.0f, -1.0f,  1.0f},
    {1.0f,  1.0f,  1.0f},
    {1.0f,  1.0f,  1.0f},
    {1.0f,  1.0f, -1.0f},
    {1.0f, -1.0f, -1.0f},

    {-1.0f, -1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f},
    {1.0f,  1.0f,  1.0f},
    {1.0f,  1.0f,  1.0f},
    {1.0f, -1.0f,  1.0f},
    {-1.0f, -1.0f,  1.0f},

    {-1.0f,  1.0f, -1.0f},
    {1.0f,  1.0f, -1.0f},
    {1.0f,  1.0f,  1.0f},
    {1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f, -1.0f},

    {-1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
    {1.0f, -1.0f, -1.0f},
    {1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
    {1.0f, -1.0f,  1.0f}
};

// Contruct a sky component
// daySkyboxPath, nightSkyboxPath: path to a folder containing skybox face images
// skyVS, skyFS: paths to sky vertex / fragment shader sources
// NOTE: If you want access to the sky's day/night cubemaps, then your shader source
// should contain 2 samplerCubes "dayCube", "nightCube" and 1 uniform float "time"
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
        skyboxVBO = new GPUBuffer(BufferType::StaticVertex, sizeof(SKYBOX_VERTICES), SKYBOX_VERTICES);
        skyboxVAO = new VertexAttributes(VertexFormat::POS, skyboxVBO);
    }

    refCount++;
}

// Cleanup
Sky::~Sky()
{
    refCount--;

    // Non-static resources manage themselves

    // If last instance, cleanup static resources
    if (refCount == 0)
    {
        delete skyboxVBO;
        delete skyboxVAO;
    }
}

// Advances the time by delta, setting appropriate variables
void Sky::AdvanceTime(float delta)
{
    // Advance time and clamp to 0 -> dayCycle
    currentTime += delta;
    if (currentTime > dayCycle) currentTime = 0;

    // Calculate normalized TOD (0 = noon, 1 = midnight)
    float s = std::sin(TAU * currentTime / dayCycle);
    float c = std::cos(TAU * currentTime / dayCycle);
    float normalizedTOD = 1 - ((s + 1) / 2);

    // Calculate global light position
    glm::vec3 globalLightPos = {1 - c - 1, std::abs(s), 1 - c - 1};

    // Update global directional light
    globalLight.direction = glm::normalize(-globalLightPos);
    globalLight.color = glm::vec3(glm::mix(dayColor, nightColor, normalizedTOD));
    ambient = ((s + 1) / 2) * 0.4 + 0.04;

    // Update skyshader time uniform
    skyShader.Use();
    skyShader.SetUniform("time", normalizedTOD);
}

// Renders the sky
void Sky::Draw()
{
    // Bind resources
    skyShader.Use();
    skyboxVAO->Bind();

    // Bind day / night cubemaps to texture units
    dayBox.Bind((int)SkyTextureUnit::Day);
    nightBox.Bind((int)SkyTextureUnit::Night);

    // Change depth function so max distance still renders
    glDepthFunc(GL_LEQUAL);
    
    // Draw, unbind, and reset depth function
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}