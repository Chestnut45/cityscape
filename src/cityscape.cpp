#include "cityscape.hpp"

// Constructor
Cityscape::Cityscape() : App("Cityscape", 4, 4), mainCamera(), sky("data/skyboxDay", "data/skyboxNight")
{
    // Enable programs
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    // Back face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Initialize blend functions (only used for point light pass)
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // Create the gBuffer
    RecreateFBO();

    // Load lighting pass shader
    globalLightShader.LoadShaderSource(GL_VERTEX_SHADER, "data/globalLightPass.vs");
    globalLightShader.LoadShaderSource(GL_FRAGMENT_SHADER, "data/globalLightPass.fs");
    globalLightShader.Link();

    // Load streetLight shader
    streetLightShader.LoadShaderSource(GL_VERTEX_SHADER, "data/streetLight.vs");
    streetLightShader.LoadShaderSource(GL_FRAGMENT_SHADER, "data/streetLight.fs");
    streetLightShader.Link();

    // Load light source shader
    // NOTE: Shares a VS with the streetLight shader
    lightSourceShader.LoadShaderSource(GL_VERTEX_SHADER, "data/streetLight.vs");
    lightSourceShader.LoadShaderSource(GL_FRAGMENT_SHADER, "data/lightSource.fs");
    lightSourceShader.Link();

    // Load snow shader
    snowShader.LoadShaderSource(GL_VERTEX_SHADER, "data/snow.vs");
    snowShader.LoadShaderSource(GL_FRAGMENT_SHADER, "data/snow.fs");
    snowShader.Link();

    // Load snowbank shader
    snowbankShader.LoadShaderSource(GL_VERTEX_SHADER, "data/snowbank.vs");
    snowbankShader.LoadShaderSource(GL_FRAGMENT_SHADER, "data/snowbank.fs");
    snowbankShader.Link();

    // Generate placeholder empty VAO for attributeless rendering
    // This is really only used for drawing a fullscreen triangle generated
    // by a vertex shader for some post-processing effects since it saves
    // the lighting calculations done on extra helper fragments that would
    // be generated by the inner seam of a quad.
    glGenVertexArrays(1, &dummyVAO);

    // Initialize camera pos
    mainCamera.SetPosition(glm::vec3(0, 2, 4));

    // Load models
    streetLightModel = new Phi::Model("data/models/streetlight.obj");
    snowbankModel = new Phi::Model("data/models/snow.obj");

    // Initial generation
    Regenerate();

    // Generate initial snow positions
    std::array<glm::vec4, MAX_SNOW> snowPositions;
    for (int i = 0; i < MAX_SNOW; ++i)
    {
        std::uniform_real_distribution<float> posDist{-2.0f, 2.0f};
        std::uniform_real_distribution<float> sizeDist{1.0f, 6.0f,};
        snowPositions[i] = {posDist(rng), posDist(rng), posDist(rng), sizeDist(rng)};
    }
    snowBuffer = new Phi::GPUBuffer(Phi::BufferType::Dynamic, sizeof(glm::vec4) * MAX_SNOW, snowPositions.data());
    snowBuffer->Bind(GL_ARRAY_BUFFER);
    snowVAO.Bind();
    snowVAO.SetStride(sizeof(glm::vec4));
    snowVAO.Add(4, GL_FLOAT);
    snowVAO.Unbind();

    // Initialize mouse input
    glfwSetInputMode(GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    prevMousePos = GetMousePos();
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(GetWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        std::cout << "Raw mouse motion enabled" << std::endl;
    }
	
    // Success msg
    std::cout << "Cityscape initialized successfully" << std::endl;
}

// Cleanup
Cityscape::~Cityscape()
{
    // Delete gBuffer + textures
    delete gBuffer;
    delete gPositionTex;
    delete gNormalTex;
    delete gColorSpecTex;
    delete gDepthStencilTex;

    delete snowBuffer;

    // Delete models
    delete streetLightModel;

    std::cout << "Cityscape shutdown successfully" << std::endl;
}

void Cityscape::Update(float delta)
{
    lastFrameTime = delta;

    // Recreate the Geometry Buffer if the app's window was resized
    if (windowResized)
    {
        RecreateFBO();
        windowResized = false;
    }

    // Update the camera's viewport if the window size has changed
    if (wWidth != mainCamera.GetWidth() || wHeight != mainCamera.GetHeight())
        mainCamera.UpdateViewport(wWidth, wHeight);

    // Process all input for this frame
    ProcessInput(delta);

    // Update the simulated city blocks
    UpdateBlocks();

    // Update the sky's time if selected
    if (timeAdvance)
    {
        // Advance time and wrap at dayCycle
        sky.currentTime += delta;
        sky.Update();

        // Adjust snow accumulation if time is being simulated
        if (snow)
            snowAccumulation = snowAccumulation >= maxAccumulation ? maxAccumulation : snowAccumulation + lastFrameTime * snowIntensity * baseAccumulationLevel;
        else
            snowAccumulation = snowAccumulation <= 0.0f ? 0.0f : snowAccumulation - lastFrameTime * baseAccumulationLevel;
    }

    // Update light colors for special modes
    if (partyMode)
    {
        static float lightTimeAccum = 0;
        lightTimeAccum += delta;
        if (lightTimeAccum >= lightTimer)
        {
            for (auto &&[entity, pointLight] : registry.view<PointLight>().each())
            {
                pointLight.SetColor(RandomColor());
            }
            lightTimeAccum = 0.0f;
        }
    }
    
    // Update lights
    if (sky.IsNight())
    {
        // Turn on lights at night
        for (auto &&[entity, pointLight] : registry.view<PointLight>().each())
        {
            if (!pointLight.IsOn()) pointLight.TurnOn();
        }
    }
    else
    {
        // Turn off lights during the day
        for (auto &&[entity, pointLight] : registry.view<PointLight>().each())
        {
            if (pointLight.IsOn()) pointLight.TurnOff();
        }
    }

    // Render ImGui window
    if (paused || keepGUIOpen)
    {
        ImGui::Begin("Cityscape");

        ImGui::Checkbox("Keep GUI Open", &keepGUIOpen);
        ImGui::Separator();

        // Simulation statistics
        ImGui::Text("Simulation:");
        ImGui::Text("Buildings: %d", buildingDrawCount);
        ImGui::Text("Lights: %d", lightDrawCount);
        ImGui::Separator();
        
        // Performance monitoring
        ImGui::Text("Performance:");
        ImGui::Text("FPS: %.0f", averageFPS);
        ImGui::PlotLines("Update:", updateSamples.data(), updateSamples.size(), 0, (const char*)nullptr, 0.0f, 16.6f, {128.0f, 32.0f});
        ImGui::SameLine();
        ImGui::Text("%.2fms", lastUpdate * 1000);
        ImGui::PlotLines("Render:", renderSamples.data(), renderSamples.size(), 0, (const char*)nullptr, 0.0f, 16.6f, {128.0f, 32.0f});
        ImGui::SameLine();
        ImGui::Text("%.2fms", lastRender * 1000);
        ImGui::Separator();

        // Graphics settings
        ImGui::Text("Graphics Settings:");
        if (ImGui::Checkbox("Fullscreen", &fullscreen))
        {
            GLFWwindow* window = GetWindow();
            if (fullscreen)
            {
                // Get primary monitor and enable fullscreen
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
            else
            {
                // Get window monitor and revert to windowed mode
                GLFWmonitor* monitor = glfwGetWindowMonitor(window);
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, NULL, (mode->width - defaultWidth) / 2, (mode->height - defaultHeight) / 2, defaultWidth, defaultHeight, 0);
            }
        }
        if (ImGui::Checkbox("Vsync", &vsync)) glfwSwapInterval(vsync);

        ImGui::End();

        // Control window
        ImGui::Begin("Controls");

        ImGui::SliderFloat("Camera Speed", &cameraSpeed, 1.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Separator();

        ImGui::Checkbox("Infinite Mode", &infinite);
        ImGui::Checkbox("Party Mode", &partyMode);
        if (ImGui::Checkbox("Festive Colors", &festiveMode))
        {
            // Regenerate all light colors if mode is toggled
            for (auto &&[entity, pointLight] : registry.view<PointLight>().each())
            {
                pointLight.SetColor(RandomColor());
            }
        };
        ImGui::Separator();

        // Timing and weather controls
        ImGui::Checkbox("Time Advance", &timeAdvance);
        if (ImGui::SliderFloat("Day Length", &sky.dayCycle, 1.0f, 120.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) sky.Update();
        if (ImGui::SliderFloat("Time of Day", &sky.currentTime, 0.0f, sky.dayCycle, "%.2f", ImGuiSliderFlags_AlwaysClamp)) sky.Update();
        ImGui::Separator();

        ImGui::Checkbox("Snow", &snow);
        ImGui::SliderFloat("Intensity", &snowIntensity, 1.0f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Accumulation", &snowAccumulation, 0.0f, maxAccumulation, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Separator();

        // Generation button
        if (ImGui::Button("Regenerate")) Regenerate();

        ImGui::End();
    }
}

void Cityscape::Render()
{
    // Update the camera's UBO so all shaders have access to the new values
    mainCamera.UpdateUBO();
    
    // Geometry pass

    // Bind the geometry fbo and clear it
    gBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Hold a vector of all currently loaded blocks and their offsets
    static std::vector<glm::vec4> blockInstancePositions;
    blockInstancePositions.clear();

    // Draw all ground tiles to the gBuffer
    for (auto &&[entity, ground]: registry.view<GroundTile>().each())
    {
        ground.Draw();
        blockInstancePositions.push_back(ground.GetPosition());
    }
    GroundTile::FlushDrawCalls();

    // Then draw all buildings to the gBuffer
    buildingDrawCount = 0;
    for (auto &&[entity, building]: registry.view<Building>().each())
    {
        building.Draw();
        buildingDrawCount++;
    }
    Building::FlushDrawCalls();
    
    // Draw all street lights
    if (sky.IsNight())
    {
        // Draw the bulbs with the light source shader during nighttime
        streetLightModel->GetMesh(0).DrawInstances(streetLightShader, blockInstancePositions);
        streetLightModel->GetMesh(1).DrawInstances(lightSourceShader, blockInstancePositions);
    }
    else
    {
        // Draw full model using textures during the day
        streetLightModel->DrawInstances(streetLightShader, blockInstancePositions);
    }

    // Draw snow effect
    if (snow)
    {
        // Snow particles
        snowShader.Use();
        snowShader.SetUniform("deltaTimeWind", glm::vec3(lastFrameTime, programLifetime, snowIntensity));
        snowVAO.Bind();
        snowBuffer->BindBase(GL_SHADER_STORAGE_BUFFER, 1);
        glDrawArrays(GL_POINTS, 0, MAX_SNOW);
        snowVAO.Unbind();
    }

    // Draw the snow accumulation (regardless of festive mode)
    snowbankShader.Use();
    snowbankShader.SetUniform("accumulationHeight", snowAccumulation);
    snowbankModel->DrawInstances(snowbankShader, blockInstancePositions);

    // Lighting passes

    // First bind all gBuffer textures appropriately
    gPositionTex->Bind(0);
    gNormalTex->Bind(1);
    gColorSpecTex->Bind(2);

    // Then blit the gBuffer's depth buffer texture to the default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, wWidth, wHeight, 0, 0, wWidth, wHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disable writing to the depth buffer here so lights don't affect it
    glDepthMask(GL_FALSE);

    // Global light pass

    // Draw a fullscreen triangle to calculate global lighting on every pixel in the scene
    globalLightShader.Use();
    glBindVertexArray(dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    // Point light pass
    
    glEnable(GL_BLEND);
    glDepthFunc(GL_ALWAYS);

    // Draw each point light
    lightDrawCount = 0;
    for (auto &&[entity, pointLight] : registry.view<PointLight>().each())
    {
        if (pointLight.IsOn())
        {
            pointLight.Draw();
            lightDrawCount++;
        }
    }
    PointLight::FlushDrawCalls();

    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);

    // Draw sky
    sky.Draw();

    // Re-enable writing into the depth buffer after all lights have been drawn
    glDepthMask(GL_TRUE);

    // Lock the camera buffer
    mainCamera.GetUBO().Lock();
}

// Handles all input for this demo
void Cityscape::ProcessInput(float delta)
{
    // Inputs allowed while unpaused
    if (!paused)
    {
        // Calculate mouse movement
        glm::vec2 mousePos = GetMousePos();
        glm::vec2 mouseOffset = (mousePos - prevMousePos) * delta * mouseSensitivity;

        // Rotate the camera according to mouse movement
        mainCamera.Rotate(mouseOffset.x, -mouseOffset.y);

        // Boost if shift is held
        float boost = IsKeyDown(GLFW_KEY_LEFT_SHIFT) ? 4 : 1;

        // Move the camera according to WASD
        if (IsKeyDown(GLFW_KEY_W)) mainCamera.Translate(mainCamera.GetDirection() * delta * cameraSpeed * boost);
        if (IsKeyDown(GLFW_KEY_S)) mainCamera.Translate(-mainCamera.GetDirection() * delta * cameraSpeed * boost);
        if (IsKeyDown(GLFW_KEY_A)) mainCamera.Translate(-mainCamera.GetRight() * delta * cameraSpeed * boost);
        if (IsKeyDown(GLFW_KEY_D)) mainCamera.Translate(mainCamera.GetRight() * delta * cameraSpeed * boost);

        // Unload blocks and regenerate if we press R
        if (IsKeyJustDown(GLFW_KEY_R)) Regenerate();

        // Toggle infinite generation mode with I
        if (IsKeyJustDown(GLFW_KEY_I)) infinite = !infinite;

        // Zoom the camera according to scroll
        mainCamera.Zoom(GetMouseScroll().y);

        // Keep track of previous mouse position
        prevMousePos = mousePos;
    }

    // Update pause state regardless of whether we are paused or not
    if (IsKeyJustDown(GLFW_KEY_ESCAPE))
    {
        paused = !paused;

        // Capture / uncapture mouse
        if (paused) glfwSetInputMode(GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else 
        {
            glfwSetInputMode(GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            prevMousePos = GetMousePos();
        }
    }
}

// Unloads all city blocks, destroys their entities, and regenerates
// a 10x10 grid of city blocks around the camera
void Cityscape::Regenerate()
{
    // Delete all loaded blocks
    for (const auto&[id, entityList] : cityBlocks)
    {
        DeleteBlock(id);
    }
    cityBlocks.clear();

    // Generate a 10x10 grid of city blocks around the camera
    glm::ivec3 pos = mainCamera.GetPosition() / 16.0f;
    for (int x = pos.x - 5; x < pos.x + 5; x++)
    {
        for (int z = pos.z - 5; z < pos.z + 5; z++)
        {
            GenerateBlock({x, z});
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
        static std::vector<glm::ivec2> shouldBeLoaded;
        shouldBeLoaded.clear();

        // Calculate which chunks should be loaded given the camera's position
        glm::ivec3 pos = mainCamera.GetPosition() / 16.0f;
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

    // Delete all blocks from deletion queue once per frame
    for (const auto& id : deletionQueue)
    {
        DeleteBlock(id);
        cityBlocks.erase(id);
    }
    deletionQueue.clear();

    // Generate a max of one chunk per frame from the generation queue
    // This helps reduce the impact of any generation stutter on lower end systems when moving around
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
    // Delete if already generated
    if (cityBlocks.count(id) > 0) DeleteBlock(id);

    // Create a ground tile component
    entt::entity temp = registry.create();
    registry.emplace<GroundTile>(temp, id);

    // Retrieve the block position
    glm::vec3 blockPos = registry.get<GroundTile>(temp).GetPosition();

    // Register the entity with the block
    cityBlocks[id].push_back(temp);

    // Create point lights for each street lamp
    temp = registry.create();
    registry.emplace<PointLight>(temp, glm::vec4{blockPos + glm::vec3(8.0f, 1.7f, 1.65f), 8.0f}, RandomColor());
    cityBlocks[id].push_back(temp);

    temp = registry.create();
    registry.emplace<PointLight>(temp, glm::vec4{blockPos + glm::vec3(1.65f, 1.7f, 8.0f), 8.0f}, RandomColor());
    cityBlocks[id].push_back(temp);

    temp = registry.create();
    registry.emplace<PointLight>(temp, glm::vec4{blockPos + glm::vec3(14.35f, 1.7f, 8.0f), 8.0f}, RandomColor());
    cityBlocks[id].push_back(temp);

    temp = registry.create();
    registry.emplace<PointLight>(temp, glm::vec4{blockPos + glm::vec3(8.0f, 1.7f, 14.35f), 8.0f}, RandomColor());
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

// Generates the next random color to be used for a street light
glm::vec4 Cityscape::RandomColor() const
{
    static std::uniform_real_distribution<float> colorDist{0.2f, 1.0f};

    if (festiveMode)
    {
        return boolDist(rng) ? glm::vec4{1.0f, 0.0f, 0.0f, 1.0f} : glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
    }
    else
    {
        return std::move(glm::vec4{glm::normalize(glm::vec3(colorDist(rng), colorDist(rng), colorDist(rng))), 1.0f});
    }
}

// Regenerates the Geometry Buffer FBO with current width and height
void Cityscape::RecreateFBO()
{
    // Delete gBuffer + textures
    if (gBuffer)
    {
        delete gBuffer;
        delete gPositionTex;
        delete gNormalTex;
        delete gColorSpecTex;
        delete gDepthStencilTex;
    }

    // Generate geometry buffer textures
    gPositionTex = new Phi::Texture2D(wWidth, wHeight, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    gNormalTex = new Phi::Texture2D(wWidth, wHeight, GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    gColorSpecTex = new Phi::Texture2D(wWidth, wHeight, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    gDepthStencilTex = new Phi::Texture2D(wWidth, wHeight, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

    // Attach textures to geometry buffer
    gBuffer = new Phi::FrameBuffer();
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