#pragma once

#include "Panel.h"

namespace Pyxis
{
	class SceneHierarchyPanel : public Panel
	{
	public:
		inline SceneHierarchyPanel(const Ref<Scene>& scene)
			: m_Scene(scene), m_SelectedEntity(nullptr)
		{

		}
		~SceneHierarchyPanel() = default;

		inline virtual void OnImGuiRender() override
		{
			if (ImGui::Begin("Scene Hierarchy"))
			{
				for each (auto pair in m_Scene->GetEntities())
				{
					DrawEntityNode(pair.second);
					
				}
			}
			ImGui::End();
		}

		inline Ref<Entity> GetSelectedEntity()
		{
			return m_SelectedEntity;
			/*std::unordered_map<uint32_t, Ref<Entity>>::const_iterator result = m_Scene->m_Entities.find(m_SelectedID);
			if (result != m_Scene->m_Entities.end())
			{
				return result->second;
			}
			return nullptr;*/
		}

		inline void DrawEntityNode(const Ref<Entity>& entity)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ((entity == m_SelectedEntity) ? ImGuiTreeNodeFlags_Selected : 0);
			
			bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity->GetID(), flags, entity->m_Name.c_str());

			if (ImGui::IsItemClicked())
			{
				m_SelectedEntity = entity;
			}

			if (opened)
			{
				//get child entities displayed as well
				for each (auto e in entity->m_Children)
				{
					DrawEntityNode(e);
				}
				ImGui::TreePop();
			}
			
		}
	private:
		Ref<Entity> m_SelectedEntity;
		Ref<Scene> m_Scene;
	};
}