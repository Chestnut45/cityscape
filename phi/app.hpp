#pragma once

#include <iostream>
#include <string>
#include <vector>

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

// Dear ImGui: https://github.com/ocornut/imgui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glm/glm.hpp>

namespace Phi
{
    // Main app class, handles OpenGL context creation and input
    class App
    {
        public:

            App(const std::string& name, int glMajVer = 4, int glMinVer = 5);
            virtual ~App();

            virtual void Run();
            virtual void Update(float delta) = 0;
            virtual void Render() = 0;

            // Accessors
            GLFWwindow* GetWindow() const { return pWindow; };
            glm::vec2 GetWindowSize() const { return {wWidth, wHeight}; };

        protected:

            // App details
            std::string name;
            int glMajorVersion;
            int glMinorVersion;

            // Window details
            GLFWwindow* pWindow;
            int wWidth = 0;
            int wHeight = 0;
            bool windowResized = false;
            static const int defaultWidth = 1280;
            static const int defaultHeight = 720;

            // Timing
            float programLifetime = 0;
            float lastUpdate = 0;
            float lastRender = 0;
            float averageFPS = 0;
            std::vector<float> updateSamples;
            std::vector<float> renderSamples;
            static const int numSamples = 240;

            // Input helpers
            bool IsKeyDown(int key) const;
            bool IsKeyJustDown(int key) const;
            bool IsLMBDown() const;
            bool IsRMBDown() const;
            bool IsMMBDown() const;
            glm::vec2 GetMousePos() const;
            glm::vec2 GetMouseScroll() const { return mouseScroll; };

        private:
            
            // Internal methods
            void InternalUpdate(float delta);

            // Internal input handling data
            glm::vec2 mouseScroll;
            static const int NUM_KEYS = GLFW_KEY_LAST - GLFW_KEY_SPACE;
            bool keysDownLastFrame[NUM_KEYS] = { false };

            // Friends who should have access to private data
            friend void WindowResizeCallback(GLFWwindow* window, int width, int height);
            friend void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    };
}