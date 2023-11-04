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
        Sky(const std::string& daySkyboxPath, const std::string& nightSkyboxPath, const std::string& skyVS, const std::string& skyFS);
        ~Sky();

        // Delete copy constructor/assignment
        Sky(const Sky&) = delete;
        Sky& operator=(const Sky&) = delete;

        // Delete move constructor/assignment
        Sky(Sky&& other) = delete;
        void operator=(Sky&& other) = delete;

        // Advances the time by delta, setting appropriate variables
        void AdvanceTime(float delta);

        // Renders the sky
        void Draw();

        // Accessors
        const DirectionalLight& GetGlobalLight() const { return globalLight; };
        float GetAmbient() const { return ambient; };

    // Data / implementation
    private:
        // Instance resources
        Cubemap dayBox;
        Cubemap nightBox;
        Shader skyShader;

        // Time of day variables
        float dayCycle = 24;
        float currentTime = 0;

        // Main directional light
        DirectionalLight globalLight;
        float ambient = 0;
        const glm::vec3 dayColor{1.0f, 0.9f, 0.5f};
        const glm::vec3 nightColor{0.2f, 0.2f, 0.3f};

        // Static resources
        static inline GPUBuffer* skyboxVBO = nullptr;
        static inline VertexAttributes* skyboxVAO = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;

        // Geometric constants
        static constexpr float TAU = 6.28318530718;
};

GLenum glCheckError();