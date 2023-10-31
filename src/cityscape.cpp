#include "cityscape.hpp"

// Constructor
Cityscape::Cityscape() : App("Cityscape"), camera(), sky("data/skyboxDay", "data/skyboxNight", "data/sky.vs", "data/sky.fs")
{
    // Enable programs
    glEnable(GL_DEPTH_TEST);

    // Create the camera and initialize position
    camera.SetPosition(glm::vec3(0, 2, 4));

    // Set the sky's shader to use our camera uniforms
    sky.GetShader().BindUniformBlock("CameraBlock", 0);

    // TODO: Generate a 10x10 grid of city blocks
    for (int x = -5; x < 5; x++)
    {
        for (int z = -5; z < 5; z++)
        {
            GenerateBlock({x, z});
        }
    }

    // TEST add single building
    entt::entity bldg = registry.create();
    registry.emplace<Building>(bldg, glm::ivec3(0), 1, 2, 0);

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
}

// Generates a city block by id
// Deletes and regenerates if one already exists with the given ID
void Cityscape::GenerateBlock(const glm::ivec2& id)
{
    // Delete if already generated
    if (cityBlocks.count(id) > 0) DeleteBlock(id);

    // Create a ground entity
    entt::entity ground = registry.create();

    // Give it a ground tile component
    registry.emplace<GroundTile>(ground, id);

    // Register it with the block
    cityBlocks[id].push_back(ground);

    // TODO: Generate buildings for each quadrant
    for (int i = 0; i < 4; i++)
    {

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

    // Clear the block ID from the map
    cityBlocks.erase(id);
}

void Cityscape::update(float dt)
{
    // Keep track of total elapsed time
    elapsedTime += dt;

    // Update the camera's viewport if the window size has changed
    if (m_width != camera.GetWidth() || m_height != camera.GetHeight())
        camera.UpdateViewport(m_width, m_height);

    // Process all input for this frame
    ProcessInput(dt);

    // Update sky
    sky.SetTOD((sin(elapsedTime) + 1) / 2);

    // Potential TODO: Infinitely generate / unload city blocks as the camera moves around
    // Requisite Guarantee: 400 MINIMUM buildings must be loaded at any given time
}

void Cityscape::render()
{
    // Clear the framebuffer
	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update the camera's UBO so all shaders have access to the new values
    camera.UpdateUBO();

    // Draw all ground tiles first
    for(auto &&[entity, ground]: registry.view<GroundTile>().each())
    {
        ground.Draw();
    }
    GroundTile::FlushDrawCalls();

    // Then draw all buildings next
    for(auto &&[entity, building]: registry.view<Building>().each())
    {
        building.Draw();
    }
    Building::Flush();

    // Debug: Just spit out errors every frame
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << std::endl;
    }

    // Draw the sky
    sky.Draw();
}

// Handles all input for this demo
void Cityscape::ProcessInput(float dt)
{
    // Calculate mouse movement
    glm::vec2 mousePos = getMousePos();
    glm::vec2 mouseOffset = (mousePos - prevMousePos) * dt * mouseSensitivity;

    // Rotate the camera according to mouse movement
    camera.Rotate(mouseOffset.x, -mouseOffset.y);

    // Boost if shift is held
    float boost = isKeyDown(GLFW_KEY_LEFT_SHIFT) ? 2 : 1;

    // Move the camera according to WASD
    if (isKeyDown(GLFW_KEY_W)) camera.Translate(camera.GetDirection() * dt * cameraSpeed * boost);
    if (isKeyDown(GLFW_KEY_S)) camera.Translate(-camera.GetDirection() * dt * cameraSpeed * boost);
    if (isKeyDown(GLFW_KEY_A)) camera.Translate(-camera.GetRight() * dt * cameraSpeed * boost);
    if (isKeyDown(GLFW_KEY_D)) camera.Translate(camera.GetRight() * dt * cameraSpeed * boost);

    // Zoom the camera according to scroll
    glm::vec2 scroll = getMouseScroll();
    camera.Zoom(scroll.y);

    // Keep track of previous mouse position
    prevMousePos = mousePos;
}