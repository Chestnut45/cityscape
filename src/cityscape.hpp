#pragma once

#include <iostream>
#include <unordered_map>

// Required for hashing ivec3s into an unordered map
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "block.hpp"
#include "camera.hpp"
#include "shader.hpp"

class Cityscape: public wolf::App
{
    public:
        Cityscape();
        ~Cityscape();

        void update(float dt) override;
        void render() override;

        // Input handling
        void ProcessInput(float dt);

    private:
        // Main camera
        Camera* camera = nullptr;

        // Simulation list
        std::unordered_map<glm::ivec3, Block> loadedBlocks;

        // Input
        glm::vec2 prevMousePos;
        float mouseSensitivity = 8.0f;
        float cameraSpeed = 4.0f;

        // Timing
        float elapsedTime = 0;
};