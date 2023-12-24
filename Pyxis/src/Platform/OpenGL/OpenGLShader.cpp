#include "pxpch.h"
#include "OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Pyxis
{
    static GLenum ShaderTypeFromString(const std::string& type)
    {
        if (type == "vertex")
            return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel")
            return GL_FRAGMENT_SHADER;
        PX_CORE_ASSERT(false, "Unknown shader type!");
        return 0;
    }

    OpenGLShader::OpenGLShader(const std::string& filePath)
    {
        std::string source = ReadFile(filePath);
        auto shaderSources = PreProcess(source);
        Compile(shaderSources);

        // Extract name from filepath
        auto lastSlash = filePath.find_last_of("/\\");
        lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        auto lastDot = filePath.rfind(".");
        auto count = lastDot == std::string::npos ? filePath.size() - lastSlash : lastDot - lastSlash;
        m_Name = filePath.substr(lastSlash, count);

    }
    OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
    {
        std::unordered_map<GLenum, std::string> shaderSources;
        shaderSources[GL_VERTEX_SHADER] = vertexSource;
        shaderSources[GL_FRAGMENT_SHADER] = fragmentSource;
        Compile(shaderSources);
    }

    std::string OpenGLShader::ReadFile(const std::string& filePath)
    {
        std::string result;
        std::ifstream inFile(filePath, std::ios::in | std::ios::binary);
        if (inFile)
        {
            inFile.seekg(0, std::ios::end);
            result.resize(inFile.tellg());
            inFile.seekg(0, std::ios::beg);
            inFile.read(&result[0], result.size());
            inFile.close();
        }
        else {
            PX_CORE_ERROR("Could not open file '{0}'", filePath);
        }
        return result;
    }

    std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source) 
    {
        std::unordered_map<GLenum, std::string> shaderSources;

        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t typeStart = source.find(typeToken, 0); // find "#type"
        while (typeStart != std::string::npos) // while finding #type
        {
            size_t eol = source.find_first_of("\r\n", typeStart); // find end of line
            PX_CORE_ASSERT(eol != std::string::npos, "Syntax Error"); // assert not eof
            size_t begin = typeStart + typeTokenLength + 1; // set begin to "#type "<-
            std::string type = source.substr(begin, eol - begin); // find substring like "vertex"
            PX_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

            //get next line to start shader string at
            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            typeStart = source.find(typeToken, nextLinePos); // find next "#type"
            //if there is no #type, use end of file, else use next #type as end of string
            shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos,
                typeStart - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos)); 
        }
        return shaderSources;
    }

    void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
    {
        GLuint program = glCreateProgram();
        PX_CORE_ASSERT(shaderSources.size() <= 2, "Too many shader sources");
        std::array<GLenum, 2> glShaderIDs;
        int GlShaderIDIndex = 0;
        for (auto & kv : shaderSources)
        {
            GLenum type = kv.first;
            const std::string& source = kv.second;

            GLuint shader = glCreateShader(type);

            const GLchar* sourceCStr = source.c_str();
            glShaderSource(shader, 1, &sourceCStr, 0);
            glCompileShader(shader);

            GLint success = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (success == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                //don't leak shaders
                glDeleteShader(shader);

                PX_CORE_ERROR("Shader Compilation Failure!");
                PX_CORE_ERROR("{0}", infoLog.data());
                PX_CORE_ASSERT(false, "Shader Compilation Failure!");
                break;
            }

            glAttachShader(program, shader);
            glShaderIDs[GlShaderIDIndex++] = shader;
        }

        
        glLinkProgram(program);
        
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int*)& isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);
            //don't leak shaders
            for (auto id : glShaderIDs)
                glDeleteShader(id);

            PX_CORE_ERROR("Shader Link Failure!");
            PX_CORE_ERROR("{0}", infoLog.data());
            PX_CORE_ASSERT(false, "Shader Link Failure!");
            return;
        }

        //don't leak shaders
        for (auto id : glShaderIDs)
            glDetachShader(program, id);

        //only assign rendererID to program if all shaders succeeded;
        m_RendererID = program;
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
        glUniform1f(location, float1);
    }

    void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& float2)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform2f(location, float2.x, float2.y);
    }

    void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& float3)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform3f(location, float3.x, float3.y, float3.z);
    }

    void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& float4)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform4f(location, float4.x, float4.y, float4.z, float4.w);
    }

    void OpenGLShader::UploadUniformInt(const std::string& name, const int int1)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1i(location, int1);
    }

    void OpenGLShader::UploadUniformInt2(const std::string& name, const glm::ivec2& int2)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform2i(location, int2.x, int2.y);
    }

    void OpenGLShader::UploadUniformInt3(const std::string& name, const glm::ivec3& int3)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform3i(location, int3.x, int3.y, int3.z);
    }

    void OpenGLShader::UploadUniformInt4(const std::string& name, const glm::ivec4& int4)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform4i(location, int4.x, int4.y, int4.z, int4.w);
    }

    void OpenGLShader::UploadUniformBool(const std::string& name, const int val)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1i(location, val);
    }

    

}
