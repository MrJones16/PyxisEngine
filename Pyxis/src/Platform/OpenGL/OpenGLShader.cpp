#include "pxpch.h"
#include "Pyxis/Renderer/Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "OpenGLShader.h"

namespace Pyxis
{
    OpenGLShader::OpenGLShader(const std::string& vertexPath, const std::string& fragmentPath)
        
    {
        // 1. retrieve the vertex/fragment source code from filePath
        //std::string vertexCode;
        //std::string fragmentCode;
        //std::ifstream vShaderFile;
        //std::ifstream fShaderFile;
        //// ensure ifstream objects can throw exceptions:
        //vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        //fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        //try
        //{
        //    // open files
        //    vShaderFile.open(vertexPath);
        //    fShaderFile.open(fragmentPath);
        //    std::stringstream vShaderStream, fShaderStream;
        //    // read file's buffer contents into streams
        //    vShaderStream << vShaderFile.rdbuf();
        //    fShaderStream << fShaderFile.rdbuf();
        //    // close file handlers
        //    vShaderFile.close();
        //    fShaderFile.close();
        //    // convert stream into string
        //    vertexCode = vShaderStream.str();
        //    fragmentCode = fShaderStream.str();
        //}
        //catch (std::ifstream::failure& e)
        //{
        //    PX_CORE_WARN("Shader File Error: {0}", e.what());//e.what();
        //}
        //const char* vShaderCode = vertexCode.c_str();
        //const char* fShaderCode = fragmentCode.c_str();

        const GLchar* source = vertexPath.c_str();

        // 2. compile shaders
        unsigned int vertex, fragment;
        GLint success = 0;
        // vertex shader

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &source, NULL);
        glCompileShader(vertex);

        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLint maxLength = 0;
            glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(vertex, maxLength, &maxLength, &infoLog[0]);

            //don't leak shaders
            glDeleteShader(vertex);

            PX_CORE_ERROR("Vertex Shader Compilation Failure!");
            PX_CORE_ERROR("{0}", infoLog.data());
            PX_CORE_ASSERT(false, "Vertex Shader Compilation Failure!");
            return;
        }

        // fragment Shader

        source = (const GLchar*)fragmentPath.c_str();

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &source, NULL);
        glCompileShader(fragment);

        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLint maxLength = 0;
            glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(fragment, maxLength, &maxLength, &infoLog[0]);

            //don't leak shaders
            glDeleteShader(fragment);
            glDeleteShader(vertex);

            PX_CORE_ERROR("Fragment Shader Compilation Failure!");
            PX_CORE_ERROR("{0}", infoLog.data());
            PX_CORE_ASSERT(false, "Fragment Shader Compilation Failure!");
            return;
        }

        // shader Program
        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertex);
        glAttachShader(m_RendererID, fragment);
        glLinkProgram(m_RendererID);

        glGetProgramiv(m_RendererID, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLint maxLength = 0;
            glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(m_RendererID);
            //don't leak shaders
            glDeleteShader(fragment);
            glDeleteShader(vertex);

            PX_CORE_ERROR("Shader Link Failure!");
            PX_CORE_ERROR("{0}", infoLog.data());
            PX_CORE_ASSERT(false, "Shader Link Failure!");
            return;
        }

        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::UploadUniformMat2(const std::string& name, const glm::mat2& matrix)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::UploadUniformFloat(const std::string& name, const float float1)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1fv(location, 1, &float1);
    }

    void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& float2)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform2fv(location, 1, glm::value_ptr(float2));
    }

    void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& float3)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform3fv(location, 1, glm::value_ptr(float3));
    }

    void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& float4)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform4fv(location, 1, glm::value_ptr(float4));
    }

    void OpenGLShader::UploadUniformInt(const std::string& name, const int int1)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1iv(location, 1, &int1);
    }

    void OpenGLShader::UploadUniformInt2(const std::string& name, const glm::ivec2& int2)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform2iv(location, 1, glm::value_ptr(int2));
    }

    void OpenGLShader::UploadUniformInt3(const std::string& name, const glm::ivec3& int3)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform3iv(location, 1, glm::value_ptr(int3));
    }

    void OpenGLShader::UploadUniformInt4(const std::string& name, const glm::ivec4& int4)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform4iv(location, 1, glm::value_ptr(int4));
    }

    void OpenGLShader::UploadUniformBool(const std::string& name, const int val)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1iv(location, 1, &val);
    }

}
