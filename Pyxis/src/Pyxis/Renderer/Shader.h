#pragma once

#include <string>

namespace Pyxis
{
	class Shader
	{
	public:
		static Ref<Shader> Shader::Create(const std::string& filePath);
		static Ref<Shader> Shader::Create(const std::string& vertexSource, const std::string& fragmentSource);

		virtual ~Shader() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};
}