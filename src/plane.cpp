#include "plane.hpp"

// Create an instance of a plane (unit quad), linearly subdivided, facing Y+
//
// Ex:    ____
// S(0) = |__|
// 
// S(1) = _______
//        |__|__|
//        |__|__|
//
//        __________
// S(2) = |__|__|__|
//        |__|__|__|
//        |__|__|__|
// ...
Plane::Plane(GLuint subdivisions, GLuint shader) : translation(0.0f), rotation(0.0f), scale(1.0f), color(1.0f)
{
    // Calculate counts
    // Q(s) = s^2 + 2s + 1
    // V(s) = (s+1)^2 + 2(s+1) + 1
    // I(q) = 6q
    GLuint quadCount = pow(subdivisions, 2) + (2 * subdivisions) + 1;
    vertexCount = pow(subdivisions + 1, 2) + (2 * (subdivisions + 1)) + 1;
    indexCount = 6 * quadCount;

    // Calculate the size of a single quad at this resolution
    GLfloat quadSize = 1.0f / (subdivisions + 1);

    // The number of vertices along one axis (mostly a readability aid)
    GLuint vertsPerAxis = subdivisions + 2;

    // Initialize vertex / index arrays
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    // Generate unique vertices
    for (GLuint x = 0; x < vertsPerAxis; x++)
    {
        for (GLuint z = 0; z < vertsPerAxis; z++)
        {
            Vertex v = {-0.5f + (x * quadSize),
                        0,
                        -0.5f + (z * quadSize)};
            
            vertices.push_back(v);
        }
    }

    // Generate indices
    for (GLuint x = 0; x < subdivisions + 1; x++)
    {
        for (GLuint z = 0; z < subdivisions + 1; z++)
        {
            // Index of "upper-left" vertex for this quad
            GLuint index = z * vertsPerAxis + x;

            // First triangle
            indices.push_back(index);
            indices.push_back(index + vertsPerAxis);
            indices.push_back(index + 1);

            // Second triangle
            indices.push_back(index + 1);
            indices.push_back(index + vertsPerAxis);
            indices.push_back(index + vertsPerAxis + 1);
        }
    }

    // Create VAO
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create VBO and upload vertex data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Create EBO and upload index data
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLfloat), indices.data(), GL_STATIC_DRAW);

    // Configure attributes
    GLuint posLoc = glGetAttribLocation(shader, "vPos");
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(posLoc);

    // Unbind
    glBindVertexArray(0);

    // Cache uniform locations
    colorLoc = glGetUniformLocation(shader, "color");
    modelLoc = glGetUniformLocation(shader, "model");
}

// Cleanup
Plane::~Plane()
{
    // Delete OpenGL objects
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

// Renders the plane
void Plane::Draw(GLuint shader)
{
    // Redundant but safe
    glUseProgram(shader);

    // Set uniforms
    glUniform4fv(colorLoc, 1, glm::value_ptr(GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(GetTransform()));

    // Bind VAO and issue draw call
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Set the plane's position to (x, y, z)
void Plane::SetTranslation(GLfloat x, GLfloat y, GLfloat z)
{
    translation = glm::vec3(x, y, z);
}

// Translate the plane by (x, y, z)
void Plane::Translate(GLfloat x, GLfloat y, GLfloat z)
{
    translation += glm::vec3(x, y, z);
}

// Set the rotation of the plane in euler angles to (x, y, z) DEGREES
void Plane::SetRotation(GLfloat x, GLfloat y, GLfloat z)
{
    rotation = glm::vec3(glm::radians(x), glm::radians(y), glm::radians(z));
}

// Adds the rotation vector (x, y, z) to the current rotation vector
void Plane::Rotate(GLfloat x, GLfloat y, GLfloat z)
{
    rotation += glm::vec3(glm::radians(x), glm::radians(y), glm::radians(z));
}

// Set's the plane's scaling along the x, y, and z axes
void Plane::SetScale(GLfloat x, GLfloat y, GLfloat z)
{
    scale = glm::vec3(x, y, z);
}

// Adds the scaling vector (x, y, z) to the current scale vector
void Plane::Scale(GLfloat x, GLfloat y, GLfloat z)
{
    scale += glm::vec3(x, y, z);
}

// Sets the color of the entire plane
// Format is normalized floats (0-1 for each component)
void Plane::SetColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
}

// Calculate the transform matrix based on position, rotation, and scale vectors
glm::mat4 Plane::GetTransform() const
{
    // Initialize as the identity matrix
    glm::mat4 transform = glm::mat4(1);

    // Apply translation last
    transform = glm::translate(transform, translation);

    // Apply rotation second (x, then y, then z)
    transform = glm::rotate(transform, rotation.x, glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, rotation.y, glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, rotation.z, glm::vec3(0, 0, 1));

    // Apply scale first
    transform = glm::scale(transform, scale);

    // Return a copy
    return transform;
}

// Returns a copy of the color vector for this plane
glm::vec4 Plane::GetColor() const
{
    return color;
}