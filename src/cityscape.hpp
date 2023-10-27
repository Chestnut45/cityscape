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
        Camera* camera = nullptr;

        // Input handling
        glm::vec2 prevMousePos;

        // OpenGL Objects
        GLuint planeShader = 0;
        
        // View properties
        glm::mat4 view;
        GLuint projLoc, viewLoc, timeLoc;

        // Simulation
        Plane* plane = nullptr;
        float elapsedTime = 0;
};