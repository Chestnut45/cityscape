#pragma once

#include <iostream>
#include <unordered_map>
#include <queue>
#include <random>

// Required for hashing glm vectors for use as keys in a std::unordered_map
// Define guard here since I'm unsure if any wolf files use GLM hashing
#ifndef GLM_ENABLE_EXPERIMENTAL
    #define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/hash.hpp>

// Cityscape components
#include "components/building.hpp"
#include "components/groundtile.hpp"
#include "components/sky.hpp"

// Phi resources
#include "phi/camera.hpp"
#include "phi/framebuffer.hpp"
#include "phi/shader.hpp"
#include "phi/texture2d.hpp"

// EnTT: https://github.com/skypjack/entt
#include "../thirdparty/entt.hpp"

// Dear ImGui: https://github.com/ocornut/imgui
#include "../thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/imgui_impl_glfw.h"
#include "../thirdparty/imgui/imgui_impl_opengl3.h"

// Wolf
#include "../wolf/wolf.h"
#include "phi/app.hpp"

class Cityscape: public wolf::App
{
    // Interface
    public:
        Cityscape();
        ~Cityscape();

        // Simulates all city blocks and handles generation
        void update(float delta) override;

        // Rendering methods
        void render() override;

        // Input handling
        void ProcessInput(float delta);

    // Data / implementation
    private:
        // Single instance objects
        Camera camera;
        Sky sky;

        // Lighting pass objects
        GLuint dummyVAO; // When using attributeless rendering, a non-zero VAO must still be bound
        Shader globalLightShader;

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

        // Timing
        float elapsedTime = 0;
        bool paused = false;
        bool timeAdvance = true;

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

        // Geometry buffer + textures for deferred shading
        FrameBuffer* gBuffer = nullptr;
        Texture2D* gPositionTex = nullptr;
        Texture2D* gNormalTex = nullptr;
        Texture2D* gColorSpecTex = nullptr;
        Texture2D* gDepthStencilTex = nullptr;

        // Framebuffer update / regen methods
        void RecreateFBO();
        void WindowResizeCallback(GLFWwindow* window, int width, int height);
        friend void WindowResizeCallback(GLFWwindow* window, int width, int height);
};