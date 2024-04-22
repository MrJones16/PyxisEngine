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
		virtual void SetFloat3(const std::string& name, const glm::vec3& float3) const override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& float2) const override;
		virtual void SetFloat(const std::string& name, const float& float1) const override;

		virtual void SetInt(const std::string& name, int int1) const override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) const override;

	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	private:
		uint32_t m_RendererID;
		std::string m_Name;
	};
}