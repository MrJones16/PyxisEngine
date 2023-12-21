#pragma once

namespace Pyxis
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexPath, const std::string& fragmentPath);
		~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		//virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;
		//virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& float4) override;

		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;
		virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) override;
		virtual void UploadUniformMat2(const std::string& name, const glm::mat2& matrix) override;

		virtual void UploadUniformFloat(const std::string& name, const float float1) override;
		virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& float2) override;
		virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& float3) override;
		virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& float4) override;

		virtual void UploadUniformInt(const std::string& name, const int int1) override;
		virtual void UploadUniformInt2(const std::string& name, const glm::ivec2& int2) override;
		virtual void UploadUniformInt3(const std::string& name, const glm::ivec3& int3) override;
		virtual void UploadUniformInt4(const std::string& name, const glm::ivec4& int4) override;

		virtual void UploadUniformBool(const std::string& name, const int val) override;
	private:
		uint32_t m_RendererID;
	};
}