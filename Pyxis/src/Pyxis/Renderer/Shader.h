#pragma once

#include <glm/glm.hpp>

namespace Pyxis
{
	class Shader
	{
	public:
		virtual ~Shader() {};
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		inline virtual uint32_t GetRendererID() const { return m_RendererID; }

		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;
		virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void UploadUniformMat2(const std::string& name, const glm::mat2& matrix) = 0;

		virtual void UploadUniformFloat(const std::string& name,  const float float1) = 0;
		virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& float2) = 0;
		virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& float3) = 0;
		virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& float4) = 0;

		virtual void UploadUniformInt(const std::string& name, const int int1) = 0;
		virtual void UploadUniformInt2(const std::string& name, const glm::ivec2& int2) = 0;
		virtual void UploadUniformInt3(const std::string& name, const glm::ivec3& int3) = 0;
		virtual void UploadUniformInt4(const std::string& name, const glm::ivec4& int4) = 0;

		virtual void UploadUniformBool(const std::string& name, const int val) = 0;

		static Shader* Shader::Create(const std::string& vertexPath, const std::string& fragmentPath);
	private:
		uint32_t m_RendererID;
	};
}