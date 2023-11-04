#pragma once

#include <glm/glm.hpp>

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 color;
};

struct PointLight
{
    glm::vec3 position;
    glm::vec3 color;
};