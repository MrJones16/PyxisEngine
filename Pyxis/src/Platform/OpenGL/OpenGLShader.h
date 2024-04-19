#pragma once

#include "Pyxis/Renderer/Shader.h"
#include <glm/glm.hpp>

// TODO: Remove
typedef unsigned int GLenum;

namespace Pyxis
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filePath);
		OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
		~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual const std::string& GetName() const override { return m_Name; }

		virtual void SetMat4(const std::string& name, const glm::mat4& matrix) const override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& float4) const override;
		//virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;
		//virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& float4) override;

		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat2(const std::string& name, const glm::mat2& matrix);

		void UploadUniformFloat(const std::string& name, const float float1);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& float2);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& float3);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& float4);

		void UploadUniformInt(const std::string& name, const int int1);
		void UploadUniformInt2(const std::string& name, const glm::ivec2& int2);
		void UploadUniformInt3(const std::string& name, const glm::ivec3& int3);
		void UploadUniformInt4(const std::string& name, const glm::ivec4& int4);

		void UploadUniformBool(const std::string& name, const int val);
	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	private:
		uint32_t m_RendererID;
		std::string m_Name;
	};
}