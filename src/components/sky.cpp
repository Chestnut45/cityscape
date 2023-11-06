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

    // Bind UBO to default light binding point
    lightUBO.BindBase(GL_UNIFORM_BUFFER, 2);

    // Initialize default lighting values
    sun = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.9f, 0.5f, 1.0f}};
    moon = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.2f, 0.2f, 0.3f, 1.0f}};

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

// Advances the time by delta, updating a global light UBO
void Sky::Update(float delta)
{
    // Advance time and clamp to [0, dayCycle]
    currentTime += delta;
    if (currentTime > dayCycle) currentTime = 0;
    offsetTime = currentTime + (dayCycle / 2);

    // Calculate normalized TOD (0 = noon, 1 = midnight)
    // This is used for interpolating between the 2 skybox images
    float ss = std::sin(TAU * currentTime / dayCycle);
    float cs = std::cos(TAU * currentTime / dayCycle);
    float sm = std::sin(TAU * offsetTime / dayCycle);
    float cm = std::cos(TAU * offsetTime / dayCycle);
    float normalizedTOD = 1 - ((ss + 1) / 2);

    // Update sky shader time uniform
    skyShader.Use();
    skyShader.SetUniform("time", normalizedTOD);

    // Calculate global light positions + directions
    sun.position = {0.0f, std::abs(ss), 1 - cs - 1, 1};
    moon.position = {0.0f, std::abs(sm), 1 - cm - 1, 1};
    sun.direction = glm::normalize(-sun.position);
    moon.direction = glm::normalize(-moon.position);

    // Update ambient lighting
    ambient = ((ss + 1) / 2) * 0.4 + 0.04;

    // Update UBO
    lightUBO.Write(sun.position);
    lightUBO.Write(sun.direction);
    lightUBO.Write(sun.color);
    lightUBO.Write(moon.position);
    lightUBO.Write(moon.direction);
    lightUBO.Write(moon.color);
    lightUBO.Write(ambient);
    lightUBO.Flush();
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