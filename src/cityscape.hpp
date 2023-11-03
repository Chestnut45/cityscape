#pragma once

#include <iostream>
#include <unordered_map>
#include <queue>
#include <random>

// Required for hashing glm vectors for use as keys in a std::unordered_map
#ifndef GLM_ENABLE_EXPERIMENTAL
    #define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/hash.hpp>

// Components and resources
#include "components/building.hpp"
#include "components/camera.hpp"
#include "components/groundtile.hpp"
#include "components/sky.hpp"
#include "resources/framebuffer.hpp"
#include "resources/shader.hpp"
#include "resources/texture2d.hpp"

// EnTT: https://github.com/skypjack/entt
#include "entt.hpp"

class Cityscape: public wolf::App
{
    // Interface
    public:
        Cityscape();
        ~Cityscape();

        // Simulates all city blocks and handles generation
        void update(float dt) override;

        // Renders all currently active components
        void render() override;

        // Input handling
        void ProcessInput(float dt);

    // Data / implementation
    private:
        // Single instance objects
        bool infinite = false;
        Camera camera;
        Sky sky;

        // Registry of all active entities
        entt::registry registry;

        // Map of city block IDs to entity id lists
        std::unordered_map<glm::ivec2, std::vector<entt::entity>> cityBlocks;

        // Queues of blocks to generate / delete
        std::deque<glm::ivec2> generationQueue;
        std::deque<glm::ivec2> deletionQueue;

        // Geometry buffer for deferred shading
        FrameBuffer* gBuffer = nullptr;
        Texture2D* gPositionTex = nullptr;
        Texture2D* gNormalTex = nullptr;
        Texture2D* gColorSpecTex = nullptr;
        Texture2D* gDepthStencilTex = nullptr;

        // Input
        glm::vec2 prevMousePos;

        // Settings
        float mouseSensitivity = 8.0f;
        float cameraSpeed = 8.0f;

        // Timing
        float elapsedTime = 0;

        // Default random seed
        long int seed = 4545L;

        // Internal methods for simulation / generation
        void Regenerate();
        void UpdateBlocks();
        void GenerateBlock(const glm::ivec2& id);
        void DeleteBlock(const glm::ivec2& id);
};