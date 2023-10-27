#include "cityscape.hpp"

// Constructor
Cityscape::Cityscape() : App("Cityscape")
{
    // Enable programs
    glEnable(GL_DEPTH_TEST);

    // Create the camera and initialize position
    camera = new Camera();
    camera->SetPosition(glm::vec3(0, 2, 4));

    // Initialize mouse input
    glfwSetInputMode(getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        std::cout << "Raw mouse motion enabled" << std::endl;
    }
    prevMousePos = getMousePos();
	
    std::cout << "Successfully initialized Cityscape" << std::endl;
}

// Cleanup
Cityscape::~Cityscape()
{
	printf("Destroying Cityscape\n");
    
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

    // Update the buffer containing camera data
    camera->UpdateUBO();
}

void Cityscape::render()
{
    // Clear the framebuffer
	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Handles all input for this demo
void Cityscape::ProcessInput(float dt)
{
    // Mouse input
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