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

            App(const std::string& name, int glMajVer, int glMinVer);
            virtual ~App();

            virtual void Run() = delete;
            virtual void Update(float delta) = delete;
            virtual void Render() = delete;

            // Accessors
            glm::vec2 GetScreenSize() const;
            GLFWwindow* GetWindow() const { return nullptr; };

            // Used internally but needs to be public
            void _setMouseScroll(const glm::vec2 &scroll);

        protected:

            // App details
            std::string name;
            int glMajorVersion;
            int glMinorVersion;

            // Window size
            int wWidth = 0;
            int wHeight = 0;

            // Input helpers
            bool IsKeyDown(int key) const;
            bool IsKeyJustDown(int key) const;
            bool IsLMBDown() const;
            bool IsRMBDown() const;
            bool IsMMBDown() const;
            glm::vec2 GetMousePos() const;
            glm::vec2 GetMouseScroll() const { return {0, 0}; };

        private:

            // Helpers
            void Init();
    };
}