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
    }),

    lightUBO(BufferType::Uniform, sizeof(DirectionalLight) * 2 + sizeof(GLfloat))
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

    // Initialize global light colors
    sun.color = {1.0f, 0.9f, 0.4f, 1.0f};
    moon.color = {0.2f, 0.2f, 0.4f, 1.0f};

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

// Advances the time by delta, updating the global lights
void Sky::Update(float delta)
{
    // Advance time and wrap at dayCycle
    currentTime += delta;
    while (currentTime > dayCycle) currentTime -= dayCycle;
    offsetTime = currentTime + (dayCycle / 2);

    Update();
}

// Sets the time directly
void Sky::SetTime(float time)
{
    // Set time and wrap at dayCycle
    currentTime = time;
    while (currentTime > dayCycle) currentTime -= dayCycle;
    offsetTime = currentTime + (dayCycle / 2);

    Update();
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

// Internal update method
void Sky::Update()
{
    // Perform expensive trig calculations once
    float st = std::sin(TAU * currentTime / dayCycle);
    float ct = std::cos(TAU * currentTime / dayCycle);
    float so = std::sin(TAU * offsetTime / dayCycle);
    float co = std::cos(TAU * offsetTime / dayCycle);
    
    // Calculate normalized TOD (t for lerping between day / night skyboxes)
    skyShader.Use();
    skyShader.SetUniform("time", 1 - ((st + 1) / 2));

    // Calculate global light positions + directions
    sun.position = {0.0f, so * sunDistance, (1 - co - 1) * sunDistance, std::min(std::max(0.0f, st) + 0.2f, 1.0f)};
    moon.position = {0.0f, st * moonDistance, (1 - ct - 1) * moonDistance, std::min(std::max(0.0f, so) + 0.2f, 1.0f)};
    sun.direction = glm::normalize(-sun.position);
    moon.direction = glm::normalize(-moon.position);

    // Update ambient lighting
    ambient = ((st + 1) / 2) * 0.45f + 0.01f;

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