#pragma once

#include <iostream>
#include <unordered_map>

#ifndef GLM_ENABLE_EXPERIMENTAL
    #define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/hash.hpp>

// Components and resources
#include "components/camera.hpp"
#include "components/groundtile.hpp"
#include "components/sky.hpp"
#include "resources/shader.hpp"

// EnTT: https://github.com/skypjack/entt
#include "entt.hpp"

class Cityscape: public wolf::App
{
    public:
        Cityscape();
        ~Cityscape();

        // Generation / simulation
        void GenerateBlock(const glm::ivec2& id);
        void DeleteBlock(const glm::ivec2& id);

        // Simulates all city blocks and handles generation
        void update(float dt) override;

        // Renders all currently active components
        void render() override;

        // Input handling
        void ProcessInput(float dt);

    private:
        // Main camera
        Camera* camera = nullptr;
        Sky* sky = nullptr;

        // Entity registry
        entt::registry registry;

        // Map of city block IDs to entity id lists
        std::unordered_map<glm::ivec2, std::vector<entt::entity>> cityBlocks;

        // Input
        glm::vec2 prevMousePos;

        // Settings
        float mouseSensitivity = 8.0f;
        float cameraSpeed = 4.0f;

        // Timing
        float elapsedTime = 0;
};