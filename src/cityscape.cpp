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

    // Initialize camera pos
    camera.SetPosition(glm::vec3(0, 2, 4));

    // Set the sky's shader to use our camera uniforms
    sky.GetShader().BindUniformBlock("CameraBlock", 0);

    // Generate a 10x10 grid of city blocks
    for (int x = -5; x < 5; x++)
    {
        for (int z = -5; z < 5; z++)
        {
            GenerateBlock({x, z});
        }
    }

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
    // Initialize static rng resources with default seed
    static std::default_random_engine rng(seed);
    static std::uniform_int_distribution<int> storyDist(1, Building::MAX_STORIES);
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

            temp = registry.create();
            registry.emplace<Building>(temp, blockPos + smallBuildingOffsets[3 * i + 1], storyDist(rng), 2,
                                    variantDist(rng), Building::Feature::None, smallBuildingOrientations[3 * i + 1]);

            temp = registry.create();
            registry.emplace<Building>(temp, blockPos + smallBuildingOffsets[3 * i + 2], storyDist(rng), 2,
                                    variantDist(rng), Building::Feature::None, smallBuildingOrientations[3 * i + 2]);
        }
        else
        {
            temp = registry.create();
            registry.emplace<Building>(temp, blockPos + largeBuildingOffsets[i], storyDist(rng), 4,
                                    variantDist(rng), Building::Feature::None, largeBuildingOrientations[i]);
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
    // GLenum errorCode;
    // while ((errorCode = glGetError()) != GL_NO_ERROR)
    // {
    //     std::string error;
    //     switch (errorCode)
    //     {
    //         case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
    //         case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
    //         case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
    //         case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
    //         case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
    //         case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
    //         case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
    //     }
    //     std::cout << error << std::endl;
    // }

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