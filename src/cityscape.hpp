#pragma once

#include <iostream>
#include <unordered_map>
#include <deque>
#include <random>

// Required for hashing glm vectors for use as keys in a std::unordered_map
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// EnTT: https://github.com/skypjack/entt
#include <entt.hpp>

// Phi engine components
#include <phi/phi.hpp>

// Cityscape components
#include "building.hpp"
#include "groundtile.hpp"
#include "sky.hpp"

class Cityscape: public Phi::App
{
    // Interface
    public:

        Cityscape();
        ~Cityscape();

        // Simulates all city blocks and handles generation
        void Update(float delta) override;

        // Renders the cityscape
        void Render() override;

        // Input handling
        void ProcessInput(float delta);

    // Data / implementation
    private:

        // Single instance objects
        Phi::Camera camera;
        Sky sky;

        // Lighting pass objects
        GLuint dummyVAO; // When using attributeless rendering, a non-zero VAO must still be bound
        Phi::Shader globalLightShader;

        // Registry of all active entities
        entt::registry registry;

        // Map of city block IDs to entity id lists
        std::unordered_map<glm::ivec2, std::vector<entt::entity>> cityBlocks;

        // Queues of blocks to generate / delete (only used in infinite mode)
        std::deque<glm::ivec2> generationQueue;
        std::deque<glm::ivec2> deletionQueue;

        // Input
        glm::vec2 prevMousePos;

        // Settings
        float mouseSensitivity = 8.0f;
        float cameraSpeed = 8.0f;
        bool infinite = false;
        bool partyMode = false;
        bool fullscreen = false;
        bool vsync = false;
        bool keepGUIOpen = false;

        // Timing
        bool paused = false;
        bool timeAdvance = true;
        float lightTimer = 0.2f;

        // Internal methods for simulation / generation
        void Regenerate();
        void UpdateBlocks();
        void GenerateBlock(const glm::ivec2& id);
        void DeleteBlock(const glm::ivec2& id);

        // RNG objects
        static inline std::default_random_engine rng{4545L};
        static inline std::uniform_int_distribution<int> storyDist{3, Building::MAX_STORIES};
        static inline std::uniform_int_distribution<int> variantDist{0, Building::NUM_VARIANTS - 1};
        static inline std::uniform_int_distribution<int> boolDist{0, 1};
        static inline std::uniform_real_distribution<float> colorDist{0.0f, 1.0f};

        // Geometry buffer + textures for deferred rendering
        Phi::FrameBuffer* gBuffer = nullptr;
        Phi::Texture2D* gPositionTex = nullptr;
        Phi::Texture2D* gNormalTex = nullptr;
        Phi::Texture2D* gColorSpecTex = nullptr;
        Phi::Texture2D* gDepthStencilTex = nullptr;

        // Framebuffer update / regen methods
        void RecreateFBO();
};