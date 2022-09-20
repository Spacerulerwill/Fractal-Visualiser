#include "shader.h"
#include <iostream> 

Shader::Shader(std::string filepath) : m_Filepath(filepath) {
    ShaderSources shaders = ParseShader(filepath);
    u_ID = CreateShader(shaders.Vertex, shaders.Fragment);
}

ShaderSources Shader::ParseShader(const std::string& filepath) {

    std::ifstream stream(filepath);

    enum ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            ss[(int)type] << line << "\n";
        }
    }

    return {
        ss[(int)ShaderType::VERTEX].str(),
        ss[(int)ShaderType::FRAGMENT].str()
    };
}

unsigned int Shader::CompileShader(unsigned int type, std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* sourceStr = source.c_str();
    glShaderSource(id, 1, &sourceStr, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* msg = new char[length];
        glGetShaderInfoLog(id, length, &length, msg);
        std::cout << "Failed to compile shader " << (type == GL_VERTEX_SHADER ? "vertex" : "fragmenet") << std::endl;
        std::cout << msg << std::endl;
        glDeleteShader(id);
        delete[] msg;
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(std::string& vertex_source, std::string& fragmement_source) {

    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertex_source);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmement_source);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Shader::Bind() const {
    glUseProgram(u_ID);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

unsigned int Shader::GetID()
{
    return u_ID;
}

Shader::~Shader() {
    glDeleteProgram(u_ID);
}