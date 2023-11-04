#include "shader.hpp"

// Initialize the shader program
Shader::Shader()
{
    programID = glCreateProgram();
}

// Destructor
Shader::~Shader()
{
    glDeleteProgram(programID);
}

// Methods
// Sets this shader as the currently active program
void Shader::Use() const
{
    glUseProgram(programID);
}

// Loads a shader stage source file from disk
// Call this for each stage you want to add to a shader program
bool Shader::LoadShaderSource(GLenum stage, const std::string& sourcePath)
{
    // Read the file stream
    std::ifstream ifs(sourcePath);
    std::string shaderSourceString((std::istreambuf_iterator<char>(ifs)),
                                    (std::istreambuf_iterator<char>()));
    
    // Format source and create the shader object
    const GLchar* shaderSource = shaderSourceString.c_str();
    GLuint shader = glCreateShader(stage);

    // Compile shader
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    // Verify
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // Compilation failed, report error and return
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR: Shader compilation failed." << infoLog << std::endl;
        return false;
    }

    // Attach shader to the program and keep track
    glAttachShader(programID, shader);
    shaders.push_back(shader);   

    // Successful return if no errors
    return true;
}

// Link the shader program
// This method detaches and deletes all shaders from the program if successful
bool Shader::Link()
{
    // Link the program
    glLinkProgram(programID);

    // Verify link stage
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cout << "ERROR: Shader program link failed." << infoLog << std::endl;
        return false;
    }

    // Detach and delete all shader objects if linking is successful
    for (GLuint shader : shaders)
    {
        glDetachShader(programID, shader);
        glDeleteShader(shader);
    }

    return true;
}

// Binds a uniform block in the shader to a specific binding point
void Shader::BindUniformBlock(const std::string& blockName, GLuint bindingPoint)
{
    GLuint index = glGetUniformBlockIndex(programID, blockName.c_str());
    glUniformBlockBinding(programID, index, bindingPoint);
}

// Sets a uniform int value in the shader
void Shader::SetUniform(const std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

// Sets a uniform float value in the shader
void Shader::SetUniform(const std::string& name, float value)
{
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::SetUniform(const std::string& name, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, glm::value_ptr(value));
}