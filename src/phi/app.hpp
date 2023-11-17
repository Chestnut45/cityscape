#pragma once

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Phi
{
    // Main app class, handles OpenGL context creation and input
    class App
    {
        public:

            App(const std::string& name, int glMajVer = 4, int glMinVer = 5);
            virtual ~App();

            virtual void Run() = delete;
            virtual void Update(float delta) = delete;
            virtual void Render() = delete;

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

            // Input helpers
            bool IsKeyDown(int key) const;
            bool IsKeyJustDown(int key) const;
            bool IsLMBDown() const;
            bool IsRMBDown() const;
            bool IsMMBDown() const;
            glm::vec2 GetMousePos() const;
            glm::vec2 GetMouseScroll() const;

        private:

            // Key inputs
            static const int NUM_KEYS = GLFW_KEY_LAST - GLFW_KEY_SPACE;
            bool m_lastKeysDown[NUM_KEYS];
    };
}