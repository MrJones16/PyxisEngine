#pragma once

#include "Panel.h"
#include "Pyxis/Core/Core.h"
#include "SceneHierarchyPanel.h"
#include <glm/glm/glm.hpp>

namespace Pyxis
{
	class InspectorPanel : public Panel
	{
	public:
		inline InspectorPanel(const Ref<SceneHierarchyPanel>& hierarchy)
			: m_SelectedID(0)
		{
			m_Hierarchy = hierarchy;
		}
		~InspectorPanel() = default;

		inline virtual void OnImGuiRender() override
		{
			if (ImGui::Begin("Inspector"))
			{
				Ref<Entity> entity = m_Hierarchy->GetSelectedEntity();
				if (entity != nullptr)
				{
					entity->InspectorRender();
				}
			}
			ImGui::End();
		}

		
	private:
		uint32_t m_SelectedID;
		Ref<SceneHierarchyPanel> m_Hierarchy;
	};
}