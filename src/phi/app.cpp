#include "app.hpp"

namespace Phi
{
    void FatalError(const char* const msg) 
    {
        std::cout << "Fatal Error: " << msg << std::endl;
        exit(1);
    }

    void Error(const char* const msg)
    {
        std::cout << "Error: " << msg << std::endl;
    }

    void ErrorCallback(int error, const char* const description)
    {
        FatalError(description);
    }

    void MouseScrollCallback(GLFWwindow* pWindow, double xoffset, double yoffset)
    {
        App* pApp = (App*)glfwGetWindowUserPointer(pWindow);
        pApp->mouseScroll = {(float)xoffset, (float)yoffset};
    }

    App::App(const std::string& name, int width, int height, int glMajVer, int glMinVer) : name(name)
    {
        // Initialize GLFW
        if (!glfwInit()) FatalError("Failed to initialize GLFW");
        std::cout << "GLFW initialized successfully" << std::endl;

        // Set callbacks
        glfwSetErrorCallback(ErrorCallback);

        // Create window
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajVer);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinVer);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        pWindow = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
        if (!pWindow) FatalError("Failed to create window");

        // Set other callbacks
        glfwSetScrollCallback(pWindow, MouseScrollCallback);

        wWidth = width;
        wHeight = height;

        glfwSetWindowUserPointer(pWindow, this);
        glfwMakeContextCurrent(pWindow);

        // Vsync
        glfwSwapInterval(1);

        // Initialize GLEW
        GLenum err = glewInit();
        if (GLEW_OK != err) FatalError((const char*)glewGetErrorString(err));
        std::cout << "GLEW initialized successfully" << std::endl;
        
        // Output current OpenGL context version
        std::cout << "OpenGL Context: " << glGetString(GL_VERSION) << std::endl;
    }

    App::~App()
    {
        
    }

    void App::Run()
    {
        double lastTime = glfwGetTime();
        while (!glfwWindowShouldClose(pWindow) && !IsKeyDown(GLFW_KEY_END))
        {
            double currentTime = glfwGetTime();
            float elapsedTime = (float)(currentTime - lastTime);
            lastTime = currentTime;

            glfwGetFramebufferSize(pWindow, &wWidth, &wHeight);
            if(wWidth != 0 && wHeight != 0)
            {
                InternalUpdate(elapsedTime);
                Render();
            }

            mouseScroll = glm::vec2(0.0f,0.0f);
            glfwSwapBuffers(pWindow);
            glfwPollEvents();
        }
    }

    bool App::IsKeyDown(int key) const
    {
        return glfwGetKey(pWindow,key) == GLFW_PRESS;
    }

    bool App::IsKeyJustDown(int key) const
    {
        return glfwGetKey(pWindow,key) == GLFW_PRESS && !keysDownLastFrame[key - GLFW_KEY_SPACE];
    }

    bool App::IsLMBDown() const { return glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS; };
    bool App::IsRMBDown() const { return glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS; };
    bool App::IsMMBDown() const { return glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS; };

    glm::vec2 App::GetMousePos() const
    {
        double xpos, ypos;
        glfwGetCursorPos(pWindow, &xpos, &ypos);
        return glm::vec2((float)xpos,(float)ypos);
    }

    void App::InternalUpdate(float delta)
    {
        Update(delta);

        // Update keys from last frame
        for (int i = 0; i < NUM_KEYS; ++i)
        {
            keysDownLastFrame[i] = IsKeyDown(GLFW_KEY_SPACE + i);
        }
    }
}