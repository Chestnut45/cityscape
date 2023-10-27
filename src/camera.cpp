#include "camera.hpp"

// Constructor
Camera::Camera() : position(0), direction(0, 0, -1), up(0, 1, 0), right(1, 0, 0)
{
    // Generate and init UBO
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) + sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Bind UBO to binding point 0
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    // Ensure our matrices are in a valid state
    UpdateView();
    UpdateProjection();
}

// Cleanup
Camera::~Camera()
{
    glDeleteBuffers(1, &ubo);
}

// Sets the camera's position and updates the view matrix
void Camera::SetPosition(const glm::vec3& position)
{
    this->position = position;
    UpdateView();
}

// Translates the camera by offset and updates the view matrix
void Camera::Translate(const glm::vec3& offset)
{
    this->position += offset;
    UpdateView();
}

// Rotates the camera according to yawOffset and pitchOffset in degrees
void Camera::Rotate(float yawOffset, float pitchOffset)
{
    yaw += yawOffset;
    pitch += pitchOffset;

    // Clamp pitch for flip safety
    if (pitch > 89) pitch =  89;
    if (pitch < -89) pitch = -89;

    UpdateView();
}

// Zoom (fov adjust)
void Camera::Zoom(float amount)
{
    fov -= amount;
    fov = glm::clamp(fov, 1.0f, 120.0f);

    UpdateProjection();
}

// Updates the camera's width and height in pixels, then recalculates the projection matrix
void Camera::UpdateViewport(int width, int height)
{
    this->width = width;
    this->height = height;

    aspect = (float)width / (float)height;
    UpdateProjection();
}

// Update the camera's view matrix
void Camera::UpdateView()
{
    // Calculate direction from yaw and pitch
    glm::vec3 dir;
    float cosP = cos(glm::radians(pitch));
    float sinP = sin(glm::radians(pitch));
    float cosY = cos(glm::radians(yaw));
    float sinY = sin(glm::radians(yaw));
    dir.x = cosY * cosP;
    dir.y = sinP;
    dir.z = sinY * cosP;
    direction = glm::normalize(dir);

    // Calculate right axis
    right = glm::normalize(glm::cross(direction, up));

    // Update view matrix
    view = glm::lookAt(position, position + direction, up);
}

// Update the camera's projection matrix
void Camera::UpdateProjection()
{
    proj = glm::perspective(glm::radians(fov), aspect, near, far);
}

// Updates the buffer to contain the view data
void Camera::UpdateUBO()
{
    // Combine view and projection into one matrix
    glm::mat4 viewProj = proj * view;
    
    // Build up local buffer with camera data
    unsigned char data[20 * sizeof(GLfloat)];

    memcpy(data, glm::value_ptr(viewProj), sizeof(glm::mat4));
    memcpy(data + sizeof(glm::mat4), glm::value_ptr(glm::vec4(position, 1)), sizeof(glm::vec4));

    // Copy over to the GPU and unbind
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}