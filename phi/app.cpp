#include "app.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

    void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        App* pApp = (App*)glfwGetWindowUserPointer(window);
        pApp->mouseScroll = {(float)xoffset, (float)yoffset};
    }

    void WindowResizeCallback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
        App* pApp = (App*)glfwGetWindowUserPointer(window);
        pApp->wWidth = width;
        pApp->wHeight = height;
        pApp->windowResized = true;
    }

    App::App(const std::string& name, int glMajVer, int glMinVer) : name(name), wWidth(defaultWidth), wHeight(defaultHeight)
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
        pWindow = glfwCreateWindow(defaultWidth, defaultHeight, name.c_str(), NULL, NULL);
        if (!pWindow) FatalError("Failed to create window");

        // Set other callbacks
        glfwSetScrollCallback(pWindow, MouseScrollCallback);
        glfwSetWindowSizeCallback(pWindow, WindowResizeCallback);

        // Set user pointer and make context current
        glfwSetWindowUserPointer(pWindow, this);
        glfwMakeContextCurrent(pWindow);

        // Vsync
        glfwSwapInterval(0);

        // Initialize GLEW
        GLenum err = glewInit();
        if (GLEW_OK != err) FatalError((const char*)glewGetErrorString(err));
        std::cout << "GLEW initialized successfully" << std::endl;
        
        // Output current OpenGL context version
        std::cout << "OpenGL Context: " << glGetString(GL_VERSION) << std::endl;

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Setup Dear ImGui Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(GetWindow(), true);
        ImGui_ImplOpenGL3_Init();
    }

    App::~App()
    {
        // Shutdown ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        std::cout << "ImGui shutdown successfully" << std::endl;

        // De-init GLFW
        glfwDestroyWindow(pWindow);
        glfwTerminate();
        std::cout << "GLFW terminated successfully" << std::endl;
    }

    void App::Run()
    {
        double lastTime = glfwGetTime();
        while (!glfwWindowShouldClose(pWindow) && !IsKeyDown(GLFW_KEY_END))
        {
            // Update timing
            double currentTime = glfwGetTime();
            float elapsedTime = (float)(currentTime - lastTime);
            lastTime = currentTime;

            // Calculate FPS
            static const float updateRate = 0.2f;
            static float timeAccum = 0;
            static float frameCount = 0;
            timeAccum += elapsedTime;
            frameCount++;
            while (timeAccum >= updateRate)
            {
                // 5 updates / second
                averageFPS = frameCount / timeAccum;
                frameCount = 0;
                timeAccum -= updateRate;
            }
            
            // Update program lifetime
            programLifetime += elapsedTime;
            
            // Poll for inputs
            glfwPollEvents();

            // Ensure framebuffer has non-zero size
            glfwGetFramebufferSize(pWindow, &wWidth, &wHeight);
            if (wWidth != 0 && wHeight != 0)
            {
                // Update and measure time
                InternalUpdate(elapsedTime);
                lastUpdate = (glfwGetTime() - currentTime);
                
                // Render and measure time
                Render();
                lastRender = (glfwGetTime() - lastUpdate - currentTime);
            }

            // Update samples
            static const float sampleRate = 1.0f / numSamples;
            static float sampleAccum = 0;
            sampleAccum += elapsedTime;
            while (sampleAccum >= sampleRate)
            {
                updateSamples.push_back(lastUpdate * 1000);
                renderSamples.push_back(lastRender * 1000);
                while (updateSamples.size() > numSamples) updateSamples.erase(updateSamples.begin());
                while (renderSamples.size() > numSamples) renderSamples.erase(renderSamples.begin());
                sampleAccum -= sampleRate;
            }

            // Reset mouse scroll
            mouseScroll = glm::vec2(0.0f, 0.0f);
            glfwSwapBuffers(pWindow);
        }
    }

    bool App::IsKeyDown(int key) const { return glfwGetKey(pWindow,key) == GLFW_PRESS; };
    bool App::IsKeyJustDown(int key) const { return glfwGetKey(pWindow,key) == GLFW_PRESS && !keysDownLastFrame[key - GLFW_KEY_SPACE]; };

    bool App::IsLMBDown() const { return glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS; };
    bool App::IsRMBDown() const { return glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS; };
    bool App::IsMMBDown() const { return glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS; };

    glm::vec2 App::GetMousePos() const
    {
        double xpos, ypos;
        glfwGetCursorPos(pWindow, &xpos, &ypos);
        return glm::vec2((float)xpos, (float)ypos);
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