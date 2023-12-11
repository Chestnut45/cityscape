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
        
        // Constants
        const size_t SNOWFLAKE_COUNT = 20'000;
        const int BLOCK_SIZE = 16;
        const int HALF_BLOCK_SIZE = BLOCK_SIZE / 2;

        // Main components
        Phi::Camera mainCamera;
        Sky sky;

        // Registry of all active entities
        entt::registry registry;

        // City block map and simulation queues
        std::unordered_map<glm::ivec2, std::vector<entt::entity>> cityBlocks;
        std::deque<glm::ivec2> generationQueue;
        std::deque<glm::ivec2> deletionQueue;

        // Models
        Phi::Model* streetLightModel = nullptr;
        Phi::Model* snowbankModel = nullptr;
        
        // Shaders
        Phi::Shader depthTransferShader;
        Phi::Shader globalLightShader;
        Phi::Shader streetLightShader;
        Phi::Shader lightSourceShader;
        Phi::Shader snowEffectShader;
        Phi::Shader snowbankShader;

        // Other resources
        Phi::GPUBuffer* snowBuffer = nullptr;
        Phi::VertexAttributes snowVAO;
        GLuint dummyVAO;

        // Input
        glm::vec2 prevMousePos;

        // Settings
        int renderDistance = 5.0f;
        bool keepGUIOpen = false;
        bool fullscreen = false;
        bool vsync = false;
        float mouseSensitivity = 0.045f;
        float cameraSpeed = 8.0f;

        bool partyMode = false;
        bool festiveMode = false;
        bool lightsAlwaysOn = false;
        bool automaticLights = true;

        bool snow = false;
        float snowIntensity = 1.0f;
        float snowAccumulation = 0.0f;
        float baseAccumulationLevel = 0.02f;
        float maxAccumulation = 1.5f;

        // Timing
        bool paused = false;
        bool timeAdvance = true;
        float lightTimer = 0.2f;
        float lastFrameTime = 0.0f;

        // Internal statistics
        int buildingDrawCount = 0;
        int lightDrawCount = 0;

        // Internal methods for simulation / generation
        void Regenerate();
        void UpdateBlocks();
        void UpdateLights();
        void GenerateBlock(const glm::ivec2& id);
        void DeleteBlock(const glm::ivec2& id);

        // RNG
        static inline std::default_random_engine rng{4545L};
        static inline std::uniform_int_distribution<int> storyDist{3, Building::MAX_STORIES};
        static inline std::uniform_int_distribution<int> variantDist{0, Building::NUM_VARIANTS - 1};
        static inline std::uniform_int_distribution<int> boolDist{0, 1};
        glm::vec4 RandomColor() const;

        // Geometry buffer + textures for deferred rendering
        Phi::FrameBuffer* gBuffer = nullptr;
        Phi::Texture2D* gPositionTex = nullptr;
        Phi::Texture2D* gNormalTex = nullptr;
        Phi::Texture2D* gColorSpecTex = nullptr;
        Phi::Texture2D* gDepthStencilTex = nullptr;

        // Shadow map depth texture
        Phi::Texture2D* shadowDepthTex = nullptr;

        // Framebuffer update / regen methods
        void RecreateFBO();
};