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

            // Updates the Uniform Buffer Object associated with this camera
            void UpdateUBO();
            
            // Accessors
            inline const glm::vec3& GetDirection() const { return direction; };
            inline const glm::vec3& GetPosition() const { return position; };
            inline const glm::vec3& GetRight() const { return right; };
            inline int GetWidth() const { return width; };
            inline int GetHeight() const { return height; };
            inline GPUBuffer& GetUBO() { return ubo; };

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

            // OpenGL resources
            GPUBuffer ubo;

            // Helper methods for updating matrices
            void UpdateView();
            void UpdateProjection();
    };
}