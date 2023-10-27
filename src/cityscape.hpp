#pragma once

#include <iostream>

#include "../wolf/wolf.h"
#include "camera.hpp"
#include "plane.hpp"

class Cityscape: public wolf::App
{
    public:
        Cityscape();
        ~Cityscape();

        void update(float dt) override;
        void render() override;

    private:
        // Main camera
        Camera* camera = nullptr;

        // Input handling
        glm::vec2 prevMousePos;
        float mouseSensitivity = 8.0f;
        float cameraSpeed = 4.0f;

        // Timing
        float elapsedTime = 0;
        GLuint timeLoc;



        // OpenGL Objects
        GLuint planeShader = 0;

        // Simulation
        Plane* plane = nullptr;
};