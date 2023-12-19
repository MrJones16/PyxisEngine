#include "pxpch.h"
#include "Pyxis/Renderer/Shader.h"

#include <glad/glad.h>
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

}
