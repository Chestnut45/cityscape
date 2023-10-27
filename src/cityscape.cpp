#include "cityscape.hpp"

// Constructor
Cityscape::Cityscape() : App("Cityscape")
{
    glEnable(GL_DEPTH_TEST);

    // Load the plane shader
    planeShader = wolf::LoadShaders("data/plane.vs", "data/plane.fs");

    // Bind the camera uniform block to the camera's binding point
    GLuint index = glGetUniformBlockIndex(planeShader, "CameraBlock");
    glUniformBlockBinding(planeShader, index, 0);
    glUseProgram(planeShader);
    timeLoc = glGetUniformLocation(planeShader, "time");

    // Create the camera and initialize position
    camera = new Camera();
    camera->SetPosition(glm::vec3(0, 2, 4));

    // Create a plane with many subdivisions
    // 31 division lines means 32x32 quads
    plane = new Plane(31, planeShader);
    plane->SetScale(4, 1, 4);
    plane->SetColor(0.2f, 0.75f, 0.8f, 1.0f);

    // Input initialization
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
    delete plane;
    glDeleteShader(planeShader);
}

void Cityscape::update(float dt)
{
    // Keep track of total elapsed time
    elapsedTime += dt;

    // Update the camera's viewport if the window size has changed
    if (m_width != camera->GetWidth() || m_height != camera->GetHeight())
    {
        camera->UpdateViewport(m_width, m_height);
    }

    // Process all input for this frame
    ProcessInput(dt);

    // Update the buffer containing camera data
    camera->UpdateUBO();
}

void Cityscape::render()
{
    // Clear the framebuffer
	glClearColor(0.3f, 0.3f, 0.3f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Redundant but safe
    glUseProgram(planeShader);

    // Update uniforms
    glUniform1fv(timeLoc, 1, &elapsedTime);

    // Draw plane
    plane->Draw(planeShader);
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