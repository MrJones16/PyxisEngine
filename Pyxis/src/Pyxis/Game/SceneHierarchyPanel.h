#pragma once

#include "Pyxis/Core/Panel.h"

namespace Pyxis
{
	class SceneHierarchyPanel : public Panel
	{
	public:
		inline SceneHierarchyPanel(const Ref<Scene>& scene)
			: m_Scene(scene), m_SelectedNode(nullptr)
		{

		}
		~SceneHierarchyPanel() = default;

		inline virtual void OnImGuiRender() override
		{
			if (ImGui::Begin("Scene Hierarchy"))
			{
				for (auto& pair : m_Scene->GetNodes())
				{
					DrawNodeNode(pair.second);
				}
			}
			ImGui::End();
		}

		inline Ref<Node> GetSelectedNode()
		{
			return m_SelectedNode;
			/*std::unordered_map<uint32_t, Ref<Node>>::const_iterator result = m_Scene->m_Entities.find(m_SelectedID);
			if (result != m_Scene->m_Entities.end())
			{
				return result->second;
			}
			return nullptr;*/
		}

		inline void DrawNodeNode(const Ref<Node>& Node)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ((Node == m_SelectedNode) ? ImGuiTreeNodeFlags_Selected : 0);
			
			bool opened = ImGui::TreeNodeEx((void*)(uint32_t)Node->GetID(), flags, Node->m_Name.c_str());

			if (ImGui::IsItemClicked())
			{
				m_SelectedNode = Node;
			}

			if (opened)
			{
				//get child entities displayed as well
				for (auto& node : Node->m_Children)
				{
					DrawNodeNode(node);
				}
				ImGui::TreePop();
			}
			
		}
	private:
		Ref<Node> m_SelectedNode;
		Ref<Scene> m_Scene;
	};
}