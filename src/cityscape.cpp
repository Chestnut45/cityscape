#include "cityscape.hpp"

// Constructor
Cityscape::Cityscape() : App("Cityscape"), camera(), sky("data/skyboxDay", "data/skyboxNight", "data/sky.vs", "data/sky.fs")
{
    // Enable programs
    glEnable(GL_DEPTH_TEST);

    // Back face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Create the gBuffer FBO
    RecreateFBO();

    // Load lighting pass shader
    globalLightShader.LoadShaderSource(GL_VERTEX_SHADER, "data/globalLightPass.vs");
    globalLightShader.LoadShaderSource(GL_FRAGMENT_SHADER, "data/globalLightPass.fs");
    globalLightShader.Link();
    globalLightShader.Use();
    globalLightShader.SetUniform("gPos", 0);
    globalLightShader.SetUniform("gNorm", 1);
    globalLightShader.SetUniform("gColorSpec", 2);
    globalLightShader.BindUniformBlock("CameraBlock", 0);
    globalLightShader.BindUniformBlock("GlobalLightBlock", 2);

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

    if (!paused)
    {
        // Update sky
        sky.Update(delta);

        // Update the simulated city blocks
        UpdateBlocks();
    }
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
    Building::FlushDrawCalls();

    // Blit the gBuffer's depth buffer texture to the default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glClear(GL_COLOR_BUFFER_BIT);

    // Lighting pass
    // First bind all gBuffer textures appropriately
    gPositionTex->Bind(0);
    gNormalTex->Bind(1);
    gColorSpecTex->Bind(2);

    // Use the lighting pass shader
    globalLightShader.Use();

    // Draw a fullscreen triangle to calculate global lighting on every pixel in the scene
    // We want to disable writing to the depth buffer here so we don't prevent the skybox from drawing later
    glDepthMask(GL_FALSE);
    glBindVertexArray(dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);

    // TODO: Point light pass, render instanced spheres scaled by light size

    // Draw the sky
    sky.Draw();
}

// Handles all input for this demo
void Cityscape::ProcessInput(float delta)
{
    if (!paused)
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

    // Update pause state regardless of whether we are paused or not
    if (isKeyJustDown(GLFW_KEY_ESCAPE))
    {
        paused = !paused;

        if (paused) glfwSetInputMode(getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else glfwSetInputMode(getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
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

// Regenerates the Geometry Buffer FBO with current width and height
void Cityscape::RecreateFBO()
{
    // Delete gBuffer + textures
    // This is safe the first time since calling delete on nullptr is allowed
    delete gBuffer;
    delete gPositionTex;
    delete gNormalTex;
    delete gColorSpecTex;
    delete gDepthStencilTex;

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
}

// Recreate the geometry buffer and other FBOs when the window is resized
void Cityscape::WindowResizeCallback(GLFWwindow* window, int width, int height)
{
    m_width = width;
    m_height = height;
    RecreateFBO();
}