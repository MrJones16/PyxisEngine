#pragma once
#include <Pyxis/Nodes/UINode.h>
#include <Pyxis/Renderer/Camera.h>

namespace Pyxis
{
	namespace UI
	{
		class UIScreenSpace : public UINode
		{
			UIScreenSpace(const std::string& name = "UIScreenSpace") : UINode(name)
			{
				
			}

			glm::mat4 GetWorldTransform() override
			{
				if (Camera::Main() != nullptr)
				{
					return glm::inverse(Camera::Main()->GetViewProjectionMatrix());
				}
				else
				{
					return m_LocalTransform;
				}


			}
		};
	}
}
