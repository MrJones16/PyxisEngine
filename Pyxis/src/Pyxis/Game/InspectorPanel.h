#pragma once

#include "Pyxis/Core/Panel.h"
#include "Pyxis/Core/Core.h"
#include "SceneHierarchyPanel.h"
#include <glm/glm.hpp>

namespace Pyxis
{
	class InspectorPanel : public Panel
	{
	public:
		inline InspectorPanel(const Ref<SceneHierarchyPanel> hierarchy)
			: m_SelectedID(0)
		{
			m_Hierarchy = hierarchy;
		}
		~InspectorPanel() = default;

		inline virtual void OnImGuiRender() override
		{
			if (ImGui::Begin("Inspector"))
			{
				Ref<Node> node = m_Hierarchy->GetSelectedNode();
				if (node != nullptr)
				{
					node->InspectorRender();
				}
			}
			ImGui::End();
		}

		
	private:
		uint32_t m_SelectedID;
		Ref<SceneHierarchyPanel> m_Hierarchy;
	};
}