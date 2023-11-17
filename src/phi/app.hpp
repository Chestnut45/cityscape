#pragma once

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#define GLEW_NO_GLU
#include <GL/glew.h>

#if defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#include <glm/glm.hpp>

namespace Phi
{
    // Main app class, handles OpenGL context creation and input
    class App
    {
        public:

            App(const std::string& name, int width, int height, int glMajVer = 4, int glMinVer = 5);
            virtual ~App();

            virtual void Run();
            virtual void Update(float delta) = 0;
            virtual void Render() = 0;

            // Accessors
            glm::vec2 GetWindowSize() const;
            glm::vec2 GetFramebufferSize() const;
            GLFWwindow* GetWindow() const { return nullptr; };

        protected:

            // App details
            std::string name;
            int glMajorVersion;
            int glMinorVersion;

            // Window details
            GLFWwindow* pWindow;
            int wWidth = 0;
            int wHeight = 0;

            // Timing
            float elapsedTime = 0;
            float lastUpdate = 0;
            float lastRender = 0;

            // Input helpers
            bool IsKeyDown(int key) const;
            bool IsKeyJustDown(int key) const;
            bool IsLMBDown() const;
            bool IsRMBDown() const;
            bool IsMMBDown() const;
            glm::vec2 GetMousePos() const;
            glm::vec2 GetMouseScroll() const;

        private:
            
            // Internal methods
            void InternalUpdate(float delta);

            // Internal input handling data
            glm::vec2 mouseScroll;
            static const int NUM_KEYS = GLFW_KEY_LAST - GLFW_KEY_SPACE;
            bool keysDownLastFrame[NUM_KEYS] = { false };

            // Friends who should have access to private data
            friend void MouseScrollCallback(GLFWwindow* pWindow, double xoffset, double yoffset);
    };
}