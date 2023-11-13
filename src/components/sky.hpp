#pragma once

#include <vector>
#include <string>

#include "../components/lights.hpp"
#include "../resources/cubemap.hpp"
#include "../resources/shader.hpp"
#include "../resources/gpubuffer.hpp"
#include "../resources/vertexattributes.hpp"

// Centralize texture unit bindings so we don't have a bunch of magic numbers everywhere
enum class SkyTextureUnit : int
{
    Day = 0,
    Night = 1
};

// Sky component, handles 2 cubemaps representing day and night skyboxes
class Sky
{
    // Interface
    public:
        Sky(const std::string& daySkyboxPath, const std::string& nightSkyboxPath);
        ~Sky();

        // Delete copy constructor/assignment
        Sky(const Sky&) = delete;
        Sky& operator=(const Sky&) = delete;

        // Delete move constructor/assignment
        Sky(Sky&& other) = delete;
        void operator=(Sky&& other) = delete;

        // Simulation
        void Update();

        // Renders the sky
        void Draw();

        // Time of day variables
        // These are global to allow full control over timing,
        // and to give access to the ImGui window for editing
        float dayCycle = 45.0f;
        float currentTime = 0.0f;
        float offsetTime = 0.0f;

        // Accessors
        inline bool IsNight() const { return (currentTime > dayCycle / 2.0f); };

    // Data / implementation
    private:
        // Instance resources
        Cubemap dayBox;
        Cubemap nightBox;
        Shader skyboxShader;
        Shader sunShader;
        Shader moonShader;

        // Global light data
        GPUBuffer lightUBO;
        DirectionalLight sun;
        DirectionalLight moon;
        float ambient;
        float sunDistance = 800.0f;
        float moonDistance = 800.0f;

        // Static resources
        static inline GPUBuffer* skyboxVBO = nullptr;
        static inline VertexAttributes* skyboxVAO = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;

        // Geometric constants
        static constexpr float TAU = 6.28318530718;
};