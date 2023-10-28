#pragma once

#include <vector>
#include <string>

#include "../resources/cubemap.hpp"
#include "../resources/shader.hpp"

// Centralize texture unit bindings
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

        Sky(const std::vector<std::string>& dayFaces,
            const std::vector<std::string>& nightFaces,
            const std::string& skyVS, const std::string& skyFS);

        ~Sky();

        // Delete copy constructor/assignment
        Sky(const Sky&) = delete;
        Sky& operator=(const Sky&) = delete;

        // Delete move constructor/assignment
        Sky(Sky&& other) = delete;
        void operator=(Sky&& other) = delete;

        // Renders the sky
        void Draw();

    // Data / implementation
    private:
        // Resources
        Cubemap dayBox;
        Cubemap nightBox;
        Shader skyShader;
};