#include "camera.hpp"

namespace Phi
{
    // Constructor
    Camera::Camera() : position(0), direction(0, 0, -1), up(0, 1, 0), right(1, 0, 0),
                        ubo(BufferType::Uniform, sizeof(glm::mat4) + sizeof(glm::vec4) * 2)
    {
        // Bind UBO to binding point 0
        ubo.BindBase(GL_UNIFORM_BUFFER, 0);

        // Ensure our matrices are in a valid state
        UpdateView();
        UpdateProjection();
    }

    // Cleanup
    Camera::~Camera()
    {
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

    // Rotates the camera according to yawOffset and pitchOffset in degrees,
    // then updates the view matrix
    void Camera::Rotate(float yawOffset, float pitchOffset)
    {
        yaw += yawOffset;
        pitch += pitchOffset;

        // Clamp pitch for flip safety
        if (pitch > 89) pitch =  89;
        if (pitch < -89) pitch = -89;

        UpdateView();
    }

    // Zooms the camera by amount, updating the projection matrix
    void Camera::Zoom(float amount)
    {
        fov -= amount;
        fov = glm::clamp(fov, 1.0f, 120.0f);

        UpdateProjection();
    }

    // Updates the camera's viewport's width and height in pixels, then recalculates the projection matrix
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

        // Calculate forward and right axes
        direction = glm::normalize(dir);
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

        // Write camera matrix data to UBO
        ubo.Write(viewProj);
        ubo.Write(glm::vec4(position, 1));
        ubo.Write(glm::vec4(width, height, 0.0f, 0.0f));
        ubo.SetOffset(0);
    }
}