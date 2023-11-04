#include "cityscape.hpp"

// Offset locations relative to a city block origin for buildings
static const glm::ivec3 smallBuildingOffsets[] =
{
    {4, 0, 7},
    {4, 0, 4},
    {7, 0, 4},
    {9, 0, 4},
    {12, 0, 4},
    {12, 0, 7},
    {12, 0, 9},
    {12, 0, 12},
    {9, 0, 12},
    {7, 0, 12},
    {4, 0, 12},
    {4, 0, 9},
};

static const Building::Orientation smallBuildingOrientations[] =
{
    Building::Orientation::West,
    Building::Orientation::North,
    Building::Orientation::North,
    Building::Orientation::North,
    Building::Orientation::East,
    Building::Orientation::East,
    Building::Orientation::East,
    Building::Orientation::South,
    Building::Orientation::South,
    Building::Orientation::South,
    Building::Orientation::West,
    Building::Orientation::West
};

static const glm::ivec3 largeBuildingOffsets[] =
{
    {5, 0, 5},
    {11, 0, 5},
    {11, 0, 11},
    {5, 0, 11}
};

static const Building::Orientation largeBuildingOrientations[] =
{
    Building::Orientation::West,
    Building::Orientation::North,
    Building::Orientation::East,
    Building::Orientation::South
};

// Constructor
Cityscape::Cityscape() : App("Cityscape"), camera(), sky("data/skyboxDay", "data/skyboxNight", "data/sky.vs", "data/sky.fs")
{
    // Enable programs
    glEnable(GL_DEPTH_TEST);

    // Back face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Generate geometry buffer textures
    gPositionTex = new Texture2D(m_width, m_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST);
    gNormalTex = new Texture2D(m_width, m_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST);
    gColorSpecTex = new Texture2D(m_width, m_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST);
    gDepthStencilTex = new Texture2D(m_width, m_height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, GL_NEAREST);

    // Attach textures to geometry buffer
    gBuffer = new FrameBuffer();
    gBuffer->Bind();
    gBuffer->AttachTexture(gPositionTex, GL_COLOR_ATTACHMENT0);
    gBuffer->AttachTexture(gNormalTex, GL_COLOR_ATTACHMENT1);
    gBuffer->AttachTexture(gColorSpecTex, GL_COLOR_ATTACHMENT2);
    gBuffer->AttachTexture(gDepthStencilTex, GL_DEPTH_STENCIL_ATTACHMENT);

    // Set the draw buffers for the currently bound FBO
    GLenum drawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, drawBuffers);

    // Check for completeness :)
    gBuffer->CheckCompleteness();

    // Load lighting pass shader
    lightingShader.LoadShaderSource(GL_VERTEX_SHADER, "data/lightingPass.vs");
    lightingShader.LoadShaderSource(GL_FRAGMENT_SHADER, "data/lightingPass.fs");
    lightingShader.Link();
    lightingShader.Use();
    lightingShader.SetUniform("gPos", 0);
    lightingShader.SetUniform("gNorm", 1);
    lightingShader.SetUniform("gColorSpec", 2);
    lightingShader.BindUniformBlock("CameraBlock", 0);
    lightingShader.BindUniformBlock("LightBlock", 2);

    // Generate placeholder empty VAO
    glGenVertexArrays(1, &dummyVAO);

    // Initialize camera pos
    camera.SetPosition(glm::vec3(0, 2, 4));

    // Initial generation
    Regenerate();

    // Initialize mouse input
    glfwSetInputMode(getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        std::cout << "Raw mouse motion enabled" << std::endl;
    }
    prevMousePos = getMousePos();
	
    // Success msg
    std::cout << "Successfully initialized Cityscape" << std::endl;
}

// Cleanup
Cityscape::~Cityscape()
{
	std::cout << "Destroying Cityscape..." << std::endl;

    // Delete gBuffer + textures
    delete gBuffer;
    delete gPositionTex;
    delete gNormalTex;
    delete gColorSpecTex;
    delete gDepthStencilTex;
}

void Cityscape::update(float delta)
{
    // Keep track of total elapsed time
    elapsedTime += delta;

    // Update the camera's viewport if the window size has changed
    if (m_width != camera.GetWidth() || m_height != camera.GetHeight())
        camera.UpdateViewport(m_width, m_height);

    // Process all input for this frame
    ProcessInput(delta);

    // Update sky
    sky.AdvanceTime(delta);

    // Update the simulated city blocks
    UpdateBlocks();
}

void Cityscape::render()
{
    // Bind the geometry framebuffer and clear it
    gBuffer->Bind();
	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update the camera's UBO so all shaders have access to the new values
    camera.UpdateUBO();

    // Draw all ground tiles to the gBuffer
    for(auto &&[entity, ground]: registry.view<GroundTile>().each())
    {
        ground.Draw();
    }
    GroundTile::FlushDrawCalls();

    // Then draw all buildings to the gBuffer
    for(auto &&[entity, building]: registry.view<Building>().each())
    {
        building.Draw();
    }
    Building::Flush();

    // Blit the gBuffer's depth buffer texture to the default framebuffer so we can use the depth values
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glClear(GL_COLOR_BUFFER_BIT);

    // Lighting pass
    // First bind all gBuffer textures appropriately
    gPositionTex->Bind(0);
    gNormalTex->Bind(1);
    gColorSpecTex->Bind(2);

    // Use the lighting pass shader and update lights
    lightingShader.Use();
    lightingShader.SetUniform("sun.direction", sky.GetSun().direction);
    lightingShader.SetUniform("sun.color", sky.GetSun().color);
    lightingShader.SetUniform("sun.ambient", sky.GetSun().ambient);

    // Draw a fullscreen triangle to calculate lighting on every pixel in the scene
    // We want to disable writing to the depth buffer here so we don't prevent the skybox from drawing later
    glDepthMask(GL_FALSE);
    glBindVertexArray(dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);

    // Draw the sky last
    sky.Draw();
}

// Handles all input for this demo
void Cityscape::ProcessInput(float delta)
{
    // Calculate mouse movement
    glm::vec2 mousePos = getMousePos();
    glm::vec2 mouseOffset = (mousePos - prevMousePos) * delta * mouseSensitivity;

    // Rotate the camera according to mouse movement
    camera.Rotate(mouseOffset.x, -mouseOffset.y);

    // Boost if shift is held
    float boost = isKeyDown(GLFW_KEY_LEFT_SHIFT) ? 2 : 1;

    // Move the camera according to WASD
    if (isKeyDown(GLFW_KEY_W)) camera.Translate(camera.GetDirection() * delta * cameraSpeed * boost);
    if (isKeyDown(GLFW_KEY_S)) camera.Translate(-camera.GetDirection() * delta * cameraSpeed * boost);
    if (isKeyDown(GLFW_KEY_A)) camera.Translate(-camera.GetRight() * delta * cameraSpeed * boost);
    if (isKeyDown(GLFW_KEY_D)) camera.Translate(camera.GetRight() * delta * cameraSpeed * boost);

    // Unload blocks and regenerate if we press R
    if (isKeyJustDown(GLFW_KEY_R)) Regenerate();

    // Toggle infinite generation mode with I
    if (isKeyJustDown(GLFW_KEY_I))
    {
        infinite = !infinite;
        Regenerate();
    };

    // Zoom the camera according to scroll
    glm::vec2 scroll = getMouseScroll();
    camera.Zoom(scroll.y);

    // Keep track of previous mouse position
    prevMousePos = mousePos;
}

// Unloads all city blocks and destroys their entities
// If not in infinite mode, regenerates 10x10 city blocks
void Cityscape::Regenerate()
{
    if (infinite)
    {
        // Delete all loaded blocks
        for (const auto&[id, entityList] : cityBlocks)
        {
            DeleteBlock(id);
        }
        cityBlocks.clear();
    }
    else
    {
        // Delete all loaded blocks
        for (const auto&[id, entityList] : cityBlocks)
        {
            DeleteBlock(id);
        }
        cityBlocks.clear();

        // Generate a 10x10 grid of city blocks
        for (int x = -5; x < 5; x++)
        {
            for (int z = -5; z < 5; z++)
            {
                GenerateBlock({x, z});
            }
        }
    }

    // Clear queues
    generationQueue.clear();
    deletionQueue.clear();
}

// Updates the blocks that should be loaded / deleted
void Cityscape::UpdateBlocks()
{
    // Update which chunks should be loaded
    if (infinite)
    {
        // Initialize static list of chunks to load
        static std::vector<glm::ivec2> shouldBeLoaded;
        shouldBeLoaded.clear();

        // Build should be loaded list
        glm::ivec3 pos = camera.GetPosition() / 16.0f;
        for (int x = pos.x - 5; x < pos.x + 5; ++x)
        {
            for (int z = pos.z - 5; z < pos.z + 5; ++z)
            {
                shouldBeLoaded.push_back(glm::ivec2(x, z));
            }
        }

        // Ensure all chunks are added to generation queue if necessary
        for (const auto& id : shouldBeLoaded)
        {
            if (cityBlocks.count(id) == 0)
            {
                if (std::find(generationQueue.begin(), generationQueue.end(), id) == generationQueue.end())
                {
                    generationQueue.push_back(id);
                }
            }
        }

        // Ensure any unnecessary chunks are deleted ASAP
        for (const auto&[id, entityList] : cityBlocks)
        {
            if (std::find(shouldBeLoaded.begin(), shouldBeLoaded.end(), id) == shouldBeLoaded.end())
            {
                deletionQueue.push_back(id);
            }
        }
    }

    // Delete all blocks from queue once per frame
    for (const auto& id : deletionQueue)
    {
        DeleteBlock(id);
        cityBlocks.erase(id);
    }
    deletionQueue.clear();

    // Generate a max of one chunk per frame from the queue
    // This helps reduce the impact of any generation stutter on lower end systems
    if (!generationQueue.empty())
    {
        GenerateBlock(generationQueue.front());
        generationQueue.pop_front();
    }
}

// Generates a city block by id
// Deletes and regenerates if one already exists with the given ID
void Cityscape::GenerateBlock(const glm::ivec2& id)
{
    // Initialize static rng resources with default seed
    static std::default_random_engine rng(seed);
    static std::uniform_int_distribution<int> storyDist(3, Building::MAX_STORIES);
    static std::uniform_int_distribution<int> variantDist(0, Building::NUM_VARIANTS - 1);
    static std::uniform_int_distribution<int> boolDist(0, 1);

    // Delete if already generated (regenerate)
    if (cityBlocks.count(id) > 0) DeleteBlock(id);

    // Create a ground tile component
    entt::entity temp = registry.create();
    registry.emplace<GroundTile>(temp, id);

    // Retrieve the block position
    glm::ivec3 blockPos = registry.get<GroundTile>(temp).GetPosition();

    // Register the entity with the block
    cityBlocks[id].push_back(temp);

    // Generate buildings for each quadrant
    for (int i = 0; i < 4; i++)
    {
        // Decide whether to place 3 small buildings or one large building
        if (boolDist(rng))
        {
            temp = registry.create();
            registry.emplace<Building>(temp, blockPos + smallBuildingOffsets[3 * i], storyDist(rng), 2,
                                       variantDist(rng), Building::Feature::None, smallBuildingOrientations[3 * i]);
            cityBlocks[id].push_back(temp);

            temp = registry.create();
            registry.emplace<Building>(temp, blockPos + smallBuildingOffsets[3 * i + 1], storyDist(rng), 2,
                                       variantDist(rng), Building::Feature::None, smallBuildingOrientations[3 * i + 1]);
            cityBlocks[id].push_back(temp);

            temp = registry.create();
            registry.emplace<Building>(temp, blockPos + smallBuildingOffsets[3 * i + 2], storyDist(rng), 2,
                                       variantDist(rng), Building::Feature::None, smallBuildingOrientations[3 * i + 2]);
            cityBlocks[id].push_back(temp);
        }
        else
        {
            temp = registry.create();
            registry.emplace<Building>(temp, blockPos + largeBuildingOffsets[i], storyDist(rng), 4,
                                       variantDist(rng), Building::Feature::None, largeBuildingOrientations[i]);
            cityBlocks[id].push_back(temp);
        }
    }
}

// Unloads and deletes a city block by id
void Cityscape::DeleteBlock(const glm::ivec2& id)
{
    // Destroy all entites associated with the block
    for (entt::entity entity : cityBlocks[id])
    {
        registry.destroy(entity);
    }
}