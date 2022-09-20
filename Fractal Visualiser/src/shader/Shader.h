#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

struct ShaderSources {
    std::string Vertex;
    std::string Fragment;
};

class Shader {
protected:
    unsigned int u_ID;
    std::string m_Filepath;

public:
    Shader(std::string filepath);
    Shader() {}
    ~Shader();

    void Bind() const;
    void Unbind() const;
    unsigned int GetID();
    int GetLocation(std::string name);

    ShaderSources ParseShader(const std::string& filepath);
    unsigned int CompileShader(unsigned int type, std::string& source);
    unsigned int CreateShader(std::string& vertex_source, std::string& fragmement_source);
};