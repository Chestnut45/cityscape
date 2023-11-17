#include "app.hpp"

namespace Phi
{
    App::App(const std::string& name, int glMajVer, int glMinVer) : name(name)
    {
        
    }

    App::~App()
    {
        
    }

    bool App::IsKeyDown(int key) const
    {
        key = toupper(key);
        return glfwGetKey(pWindow,key) == GLFW_PRESS;
    }

    bool App::IsKeyJustDown(int key) const
    {
        key = toupper(key);
        return glfwGetKey(pWindow,key) == GLFW_PRESS && !m_lastKeysDown[key-GLFW_KEY_SPACE];
    }

    bool App::IsLMBDown() const
    {
        int state = glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_LEFT);
        return state == GLFW_PRESS;
    }

    bool App::IsRMBDown() const
    {
        int state = glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT);
        return state == GLFW_PRESS;
    }

    bool App::IsMMBDown() const
    {
        int state = glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_MIDDLE);
        return state == GLFW_PRESS;
    }

    glm::vec2 App::GetMousePos() const
    {
        double xpos, ypos;
        glfwGetCursorPos(pWindow, &xpos, &ypos);
        return glm::vec2((float)xpos,(float)ypos);
    }
}