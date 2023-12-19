#pragma once

namespace Pyxis
{
	class Shader
	{
	public:
		virtual ~Shader() {};
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static Shader* Shader::Create(const std::string& vertexPath, const std::string& fragmentPath);
	private:
		uint32_t m_RendererID;
	};
}