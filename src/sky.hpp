#pragma once

#include <vector>
#include <string>

#include <phi/phi.hpp>

#include "lights.hpp"

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
        // These are public to allow full control over timing,
        // and to give access to the ImGui window for editing
        float dayCycle = 45.0f;
        float currentTime = 0.0f;
        float offsetTime = 0.0f;

        // Accessors
        inline bool IsNight() const { return (currentTime > dayCycle / 2.0f); };

    // Data / implementation
    private:
        // Instance resources
        // Skybox textures
        Phi::Cubemap dayBox;
        Phi::Cubemap nightBox;

        // Global light data
        Phi::GPUBuffer lightUBO;
        DirectionalLight sun;
        DirectionalLight moon;
        float ambient = 0.0f;
        float sunDistance = 800.0f;
        float moonDistance = 500.0f;
        float sunRadius = 50.0f;
        float moonRadius = 10.0f;
        float sunlightInfluence = 2.0f;
        float moonlightInfluence = 0.32f;

        // Static internal resources
        static inline Phi::GPUBuffer* skyboxVBO = nullptr;
        static inline Phi::VertexAttributes* skyboxVAO = nullptr;
        static inline Phi::Shader* skyboxShader = nullptr;

        static inline Phi::GPUBuffer* sphereVBO = nullptr;
        static inline Phi::GPUBuffer* sphereEBO = nullptr;
        static inline Phi::VertexAttributes* sphereVAO = nullptr;
        static inline Phi::Shader* celestialBodyShader = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;
};