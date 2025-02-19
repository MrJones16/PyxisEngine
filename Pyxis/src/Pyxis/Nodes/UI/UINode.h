#pragma once

#include <vector>
#include <Pyxis/Renderer/Renderer2D.h>
#include <Pyxis/Nodes/Node3D.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Pyxis
{
	namespace UI
	{

		/// <summary>
		/// A Base UI Node that can act as a root
		/// </summary>
		class UINode : public Node3D
		{
		public:
			inline static UUID s_MousePressedNodeID = 0;

			UINode(const std::string& name = "UINode") : Node3D(name) {};
			UINode(UUID id) : Node3D(id) {};
			virtual  ~UINode() = default;

			virtual void OnMousePressed(int mouseButton) {};
			virtual void OnMouseReleased(int mouseButton, bool continuous) {};
			//virtual void OnClick() {};

			//virtual void OnInspectorRender() override {};
			virtual void OnUpdate(Timestep ts) override {};
			virtual void OnRender() override {};

			//update propagation for UI elements
			virtual void PropagateUpdate()
			{
				for (auto& child : m_Children)
				{					
					if (UINode* node = dynamic_cast<UINode*>(child))
					{
						//PX_TRACE("Sent Propagation to child");
						node->PropagateUpdate();
					}
				}
			}

			//Serialization
			virtual void Serialize(json& j) override
			{
				Node3D::Serialize(j);
				j["Type"] = "UINode";
			}
			//virtual void Deserialize(json& j) override;


			virtual void UpdateLocalTransform() override
			{
				Node3D::UpdateLocalTransform();
				for (auto& child : m_Children)
				{
					if (UINode* node = dynamic_cast<UINode*>(child))
					{
						//PX_TRACE("Sent Propagation to child");
						node->PropagateUpdate();
					}
				}
			}
			
		};
		REGISTER_SERIALIZABLE_NODE(UINode);
	}

}