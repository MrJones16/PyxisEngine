#pragma once

#include <string>
#include <glm/glm.hpp>


namespace Pyxis
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		static Ref<Shader> Shader::Create(const std::string& filePath);
		static Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);

		virtual const std::string& GetName() const = 0;

		virtual void Bind()   const = 0;
		virtual void Unbind() const = 0;

		virtual void SetMat4   (const std::string& name, const glm::mat4& matrix) const = 0;
		virtual void SetFloat4 (const std::string& name, const glm::vec4& float4) const = 0;
	};

	class ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);

		Ref<Shader> Load(const std::string& filepath); // Texture.glsl
		Ref<Shader> Load(const std::string& name, const std::string& filePath);

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name);
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}