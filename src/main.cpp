#include <stdio.h>
#include <iostream>
#include <glm/glm.hpp>

#include "../wolf/wolf.h"
#include "../samplefw/SampleRunner.h"

#include "cityscape.hpp"

Cityscape* cityscape = nullptr;

// Handle window resizing properly
void WindowResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    cityscape->WindowResizeCallback(window, width, height);
}

// Application entrypoint
int main(int, char**)
{
    // Create the cityscape app
    cityscape = new Cityscape();

    // Setup window resize callback
    glfwSetWindowSizeCallback(cityscape->getWindow(), WindowResizeCallback);

    // Run the app
    cityscape->run();

    // Delete and exit
    delete cityscape;
    return 0;
}