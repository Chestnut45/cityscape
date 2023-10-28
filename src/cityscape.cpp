#include "cityscape.hpp"

// Constructor
Cityscape::Cityscape() : App("Cityscape")
{
    // Enable programs
    glEnable(GL_DEPTH_TEST);

    // Create the camera and initialize position
    camera = new Camera();
    camera->SetPosition(glm::vec3(0, 2, 4));

    // Generate a 10x10 grid of city blocks
    

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
    
    delete camera;
}

void Cityscape::update(float dt)
{
    // Keep track of total elapsed time
    elapsedTime += dt;

    // Update the camera's viewport if the window size has changed
    if (m_width != camera->GetWidth() || m_height != camera->GetHeight())
        camera->UpdateViewport(m_width, m_height);

    // Process all input for this frame
    ProcessInput(dt);

    // Potential TODO: Infinitely generate / unload city blocks as the camera moves around
    // Requisite Guarantee: 400 MINIMUM buildings must be loaded at any given time
}

void Cityscape::render()
{
    // Clear the framebuffer
	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update the camera's UBO so all shaders have access to the new values
    camera->UpdateUBO();

    // TODO: Iterate registry and render all components
    // NOTE: Should be simple; component.Draw() followed by Component::FlushDrawCommands() after iteration (batching)
}

// Handles all input for this demo
void Cityscape::ProcessInput(float dt)
{
    // Calculate mouse movement
    glm::vec2 mousePos = getMousePos();
    glm::vec2 mouseOffset = (mousePos - prevMousePos) * dt * mouseSensitivity;

    // Rotate the camera according to mouse movement
    camera->Rotate(mouseOffset.x, -mouseOffset.y);

    // Move the camera according to WASD
    if (isKeyDown(GLFW_KEY_W)) camera->Translate(camera->GetDirection() * dt * cameraSpeed);
    if (isKeyDown(GLFW_KEY_S)) camera->Translate(-camera->GetDirection() * dt * cameraSpeed);
    if (isKeyDown(GLFW_KEY_A)) camera->Translate(-camera->GetRight() * dt * cameraSpeed);
    if (isKeyDown(GLFW_KEY_D)) camera->Translate(camera->GetRight() * dt * cameraSpeed);

    // Keep track of previous mouse position
    prevMousePos = mousePos;
}