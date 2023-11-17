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
// daySkyboxPath, nightSkyboxPath: path to a folder containing skybox face images in the below format
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
    // Bind UBO to default light binding point
    lightUBO.BindBase(GL_UNIFORM_BUFFER, 2);

    // If first instance, initialize static resources
    if (refCount == 0)
    {
        // Create VAO / VBO / Shader for skybox
        skyboxVBO = new GPUBuffer(BufferType::StaticVertex, sizeof(SKYBOX_VERTICES), SKYBOX_VERTICES);
        skyboxVAO = new VertexAttributes(VertexFormat::POS, skyboxVBO);
        skyboxShader = new Shader();
        skyboxShader->LoadShaderSource(GL_VERTEX_SHADER, "data/skybox.vs");
        skyboxShader->LoadShaderSource(GL_FRAGMENT_SHADER, "data/skybox.fs");
        skyboxShader->Link();
        skyboxShader->Use();
        skyboxShader->SetUniform("dayCube", (int)SkyTextureUnit::Day);
        skyboxShader->SetUniform("nightCube", (int)SkyTextureUnit::Night);
        skyboxShader->BindUniformBlock("CameraBlock", 0);
        skyboxShader->BindUniformBlock("GlobalLightBlock", 2);

        // Create resources for rendering sun and moon
        sphereVBO = new GPUBuffer(BufferType::StaticVertex, sizeof(Icosphere::ICOSPHERE_VERTICES), Icosphere::ICOSPHERE_VERTICES);
        sphereEBO = new GPUBuffer(BufferType::StaticIndex, sizeof(Icosphere::ICOSPHERE_INDICES), Icosphere::ICOSPHERE_INDICES);
        sphereVAO = new VertexAttributes(VertexFormat::POS, sphereVBO, sphereEBO);
        celestialBodyShader = new Shader();
        celestialBodyShader->LoadShaderSource(GL_VERTEX_SHADER, "data/celestialBody.vs");
        celestialBodyShader->LoadShaderSource(GL_FRAGMENT_SHADER, "data/celestialBody.fs");
        celestialBodyShader->Link();
        celestialBodyShader->Use();
        celestialBodyShader->BindUniformBlock("CameraBlock", 0);
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
        delete skyboxShader;
        delete sphereVBO;
        delete sphereEBO;
        delete sphereVAO;
        delete celestialBodyShader;
    }
}

// Updates all internal lighting values for rendering
void Sky::Update()
{
    // Ensure currentTime is valid
    while (currentTime > dayCycle) currentTime -= dayCycle;

    // Calculate offset time
    offsetTime = currentTime + (dayCycle / 2);

    // Perform expensive trig calculations once
    float st = std::sin(TAU * currentTime / dayCycle);
    float ct = std::cos(TAU * currentTime / dayCycle);
    float so = std::sin(TAU * offsetTime / dayCycle);
    float co = std::cos(TAU * offsetTime / dayCycle);
    
    // Calculate normalized TOD (t for lerping between day / night skyboxes)
    skyboxShader->Use();
    skyboxShader->SetUniform("time", 1 - ((st + 1) / 2));

    // Update sun
    sun.SetPosition({0.0f, st * sunDistance, (1 - ct - 1) * sunDistance, 1.0f});
    sun.SetDirection(-glm::normalize(sun.GetPosition()));
    sun.SetColor({1.0f, st * 0.8f, std::max(0.32f, ct * 0.32f), std::min(std::max(0.0f, st * sunlightInfluence), 1.0f)});

    // Update moon
    moon.SetPosition({0.0f, so * moonDistance, (1 - co - 1) * moonDistance, 1.0f});
    moon.SetDirection(-glm::normalize(moon.GetPosition()));
    moon.SetColor({0.8f, 0.8f, 1.0f, std::min(std::max(0.0f, so * moonlightInfluence), 1.0f)});

    // Update ambient lighting
    ambient = std::max(0.05f, ((st + 1) / 2) * 0.45f);

    // Update UBO
    lightUBO.Write(sun.GetPosition());
    lightUBO.Write(sun.GetDirection());
    lightUBO.Write(sun.GetColor());
    lightUBO.Write(moon.GetPosition());
    lightUBO.Write(moon.GetDirection());
    lightUBO.Write(moon.GetColor());
    lightUBO.Write(ambient);
    lightUBO.Flush(true);
}

// Renders the sky
void Sky::Draw()
{
    // First pass: Draw skybox

    // Bind resources
    skyboxShader->Use();
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

    // Get references
    const glm::vec4& sunPos = sun.GetPosition();
    const glm::vec4& moonPos = moon.GetPosition();
    const glm::vec4& sunCol = sun.GetColor();
    const glm::vec4& moonCol = moon.GetColor();

    // Draw outer faces of icosphere
    glFrontFace(GL_CW);
    sphereVAO->Bind();

    celestialBodyShader->Use();

    // Draw sun
    celestialBodyShader->SetUniform("position", {sunPos.x, sunPos.y, sunPos.z, sunRadius});
    celestialBodyShader->SetUniform("color", {sunCol.r, sunCol.g, sunCol.b, 1.0f});
    glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);

    // Draw moon
    celestialBodyShader->SetUniform("position", {moonPos.x, moonPos.y, moonPos.z, moonRadius});
    celestialBodyShader->SetUniform("color", {moonCol.r, moonCol.g, moonCol.b, 1.0f});
    glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);

    // Unbind and reset to default winding order
    glBindVertexArray(0);
    glFrontFace(GL_CCW);
}