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
Sky::Sky(const std::string& daySkyboxPath, const std::string& nightSkyboxPath)
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
    skyboxShader.LoadShaderSource(GL_VERTEX_SHADER, "data/skybox.vs");
    skyboxShader.LoadShaderSource(GL_FRAGMENT_SHADER, "data/skybox.fs");
    skyboxShader.Link();

    // Bind samplers to proper texture units
    skyboxShader.Use();
    skyboxShader.SetUniform("dayCube", (int)SkyTextureUnit::Day);
    skyboxShader.SetUniform("nightCube", (int)SkyTextureUnit::Night);
    skyboxShader.BindUniformBlock("CameraBlock", 0);

    // Bind UBO to default light binding point
    lightUBO.BindBase(GL_UNIFORM_BUFFER, 2);

    // Initialize global light colors
    sun.SetColor({1.0f, 0.9f, 0.4f, 1.0f});
    moon.SetColor({0.2f, 0.2f, 0.4f, 1.0f});

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

// Updates all internal lighting values for rendering
void Sky::Update()
{
    // Calculate offset time
    offsetTime = currentTime + (dayCycle / 2);

    // Perform expensive trig calculations once
    float st = std::sin(TAU * currentTime / dayCycle);
    float ct = std::cos(TAU * currentTime / dayCycle);
    float so = std::sin(TAU * offsetTime / dayCycle);
    float co = std::cos(TAU * offsetTime / dayCycle);
    
    // Calculate normalized TOD (t for lerping between day / night skyboxes)
    skyboxShader.Use();
    skyboxShader.SetUniform("time", 1 - ((st + 1) / 2));

    // Calculate global light positions + directions
    sun.SetPosition({0.0f, so * sunDistance, (1 - co - 1) * sunDistance, std::min(std::max(-0.2f, st) + 0.2f, 1.0f)});
    moon.SetPosition({0.0f, st * moonDistance, (1 - ct - 1) * moonDistance, std::min(std::max(-0.2f, so) + 0.2f, 1.0f)});
    sun.SetDirection(glm::normalize(-sun.GetPosition()));
    moon.SetDirection(glm::normalize(-moon.GetPosition()));

    // Update ambient lighting
    ambient = ((st + 1) / 2) * 0.45f + 0.01f;

    // Update UBO
    lightUBO.Write(sun.GetPosition());
    lightUBO.Write(sun.GetDirection());
    lightUBO.Write(sun.GetColor());
    lightUBO.Write(moon.GetPosition());
    lightUBO.Write(moon.GetDirection());
    lightUBO.Write(moon.GetColor());
    lightUBO.Write(ambient);
    lightUBO.Flush();
}

// Renders the sky
void Sky::Draw()
{
    // First pass: Draw skybox

    // Bind resources
    skyboxShader.Use();
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

    // Second pass: Draw sun / moon
}