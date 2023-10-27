#pragma once

// GLM math includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include "../thirdparty/glew/include/GL/glew.h"

// Provides an interface to manipulate and update a camera used for rendering
class Camera
{
    // Public interface
    public:
        Camera();
        ~Camera();

        // Temporal manipulation
        void SetPosition(const glm::vec3& position);
        void Translate(const glm::vec3& offset);

        // View manipulation
        void Rotate(float yawOffset, float pitchOffset);
        void Zoom(float amount);

        // Needs to be public so caller can tell the camera when the window size changes
        void UpdateViewport(int width, int height);
        
        // Accessors
        const glm::mat4& GetView() { return view; };
        const glm::mat4& GetProj() { return proj; };
        int GetWidth() { return width; };
        int GetHeight() { return height; };

    private:
        // Camera properties
        glm::vec3 position;

        // Directional info
        glm::vec3 direction;
        glm::vec3 up;
        glm::vec3 right;

        // Matrices
        glm::mat4 view;
        glm::mat4 proj;

        // View properties (Sensible defaults)
        int width = 0;
        int height = 0;
        float fov = 60;
        float aspect = 1;
        float near = 0.1;
        float far = 1000;

        // View angles
        float yaw = -90;
        float pitch = 0;

        // OpenGL objects / state
        GLuint ubo;

        // Helper methods for updating matrices and buffer objects
        void UpdateView();
        void UpdateProjection();
        void UpdateUBO();
};