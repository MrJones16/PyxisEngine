#pragma once

#include <vector>
#include <Pyxis/Renderer/Renderer2D.h>
#include <Pyxis/Game/Node.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Pyxis
{
	/// <summary>
	/// A Base UI Node that can act as a root
	/// </summary>
	class UINode : public Node
	{
	public:
		UINode(const std::string& name = "UINode") : Node(name) {};
		virtual  ~UINode() = default;
		//virtual void InspectorRender() override;
		//virtual void OnUpdate(Timestep ts) override;
		//virtual void OnRender() override;
	};


	/// <summary>
	/// A UI Node that will display either the solid color given, or texture.
	/// </summary>
	class UIRect : public UINode
	{
	public:
		UIRect(const std::string& name = "UIRect") : UINode(name) {};
		UIRect(Ref<Texture2D> texture, const std::string& name = "UIRect");
		UIRect(const glm::vec4& color, const std::string& name = "UIRect");
		virtual ~UIRect() = default;

		virtual void InspectorRender() override;
		//virtual void OnUpdate(Timestep ts) override;
		virtual void OnRender() override;

		Ref<Texture2D> m_Texture = nullptr;
		glm::vec4 m_Color = glm::vec4(1);
		glm::vec2 m_Size = glm::vec2(1);
	};

}