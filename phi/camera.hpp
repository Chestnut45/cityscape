#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <GL/glew.h> // OpenGL types / functions

#include "gpubuffer.hpp"

namespace Phi
{
    // Provides an interface to manipulate and update a camera used for rendering
    class Camera
    {
        // Public interface
        public:

            Camera();
            ~Camera();

            // Delete copy constructor/assignment
            Camera(const Camera&) = delete;
            Camera& operator=(const Camera&) = delete;

            // Delete move constructor/assignment
            Camera(Camera&& other) = delete;
            void operator=(Camera&& other) = delete;

            // Movement
            void SetPosition(const glm::vec3& position);
            void Translate(const glm::vec3& offset);

            // View manipulation
            void Rotate(float yawOffset, float pitchOffset);
            void Zoom(float amount);

            // Needs to be public so caller can tell the camera when the window size changes
            void UpdateViewport(int width, int height);
            void UpdateView();
            void UpdateProjection();

            // Updates the Uniform Buffer Object associated with this camera
            void UpdateUBO();
            
            // Accessors
            inline const glm::vec3& GetDirection() const { return direction; };
            inline const glm::vec3& GetPosition() const { return position; };
            inline const glm::vec3& GetRight() const { return right; };
            inline int GetWidth() const { return width; };
            inline int GetHeight() const { return height; };
            inline GPUBuffer& GetUBO() { return ubo; };

            // Public so ImGUI may directly control camera properties
            float fov = 60.0f;

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
            float aspect = 1.0f;
            float near = 0.1f;
            float far = 1000.0f;

            // View angles
            float yaw = -90.0f;
            float pitch = 0.0f;

            // OpenGL resources
            GPUBuffer ubo;

            // Constants
            static const int UBO_SIZE = sizeof(glm::mat4) * 3 + sizeof(glm::vec4) * 2;
    };
}