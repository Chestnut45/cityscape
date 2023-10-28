#pragma once

#include <iostream>

#include "camera.hpp"
#include "shader.hpp"

// EnTT: https://github.com/skypjack/entt
#include "../thirdparty/entt.hpp"

class Cityscape: public wolf::App
{
    public:
        // Initialization / cleanup
        Cityscape();
        ~Cityscape();

        // Simulates all city blocks and handles generation
        void update(float dt) override;

        // Renders all currently active components
        void render() override;

        // Input handling
        void ProcessInput(float dt);

        // Generation
        void GenerateBlock(glm::ivec2 id);
        void DeleteBlock(glm::ivec2 id);

    private:
        // Main camera
        Camera* camera = nullptr;

        // Simulation list
        // Each entity in this registry represents one city block
        // Valid component types are:
        // - Ground
        // - Building
        // - ...
        entt::registry cityBlocks;

        // Input
        glm::vec2 prevMousePos;

        // Settings
        float mouseSensitivity = 8.0f;
        float cameraSpeed = 4.0f;

        // Timing
        float elapsedTime = 0;
};