#include "SceneLayer.h"
#include <Pyxis/Core/Application.h>

#include <Pyxis/Renderer/Renderer2D.h>
#include <Pyxis/Renderer/RenderCommand.h>

#include <Pyxis/Core/Input.h>

namespace Pyxis
{

	SceneLayer::SceneLayer(bool debug) :
		m_Debug(debug)
	{
		//EventSignal::s_WindowResizeEventSignal.AddReciever(m_ResizeEventReciever);
		FontLibrary::AddFont("Aseprite", "assets/fonts/Aseprite.ttf");
	}

	SceneLayer::~SceneLayer()
	{

	}

	void SceneLayer::OnAttach()
	{
		m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
		Renderer2D::Init();

		FrameBufferSpecification fbspec;
		fbspec.Attachments =
		{
			{FrameBufferTextureFormat::RGBA8, FrameBufferTextureType::Color},
			{FrameBufferTextureFormat::R32UI, FrameBufferTextureType::Color},
			{FrameBufferTextureFormat::Depth, FrameBufferTextureType::Depth}
		};
		fbspec.Width = m_ViewportSize.x;
		fbspec.Height = m_ViewportSize.y;
		m_SceneFrameBuffer = FrameBuffer::Create(fbspec);
	}

	void SceneLayer::OnDetach()
	{

	}

	void SceneLayer::OnUpdate(Timestep ts)
	{
		PROFILE_SCOPE("SceneLayer OnUpdate");

		//clear dead nodes
		while (Node::NodesToDestroyQueue.size() > 0)
		{
			Node* node = Node::Nodes[Node::NodesToDestroyQueue.front()];			
			
			if (node != nullptr)
			{
				//clear parent for children
				for (auto child : node->m_Children)
				{
					
					child->m_Parent = nullptr;
				}
				//clear parent's child of this
				if (node->m_Parent != nullptr)
				{
					node->m_Parent->RemoveChild(node->shared_from_this());
				}
			}			
			Node::Nodes.erase(Node::NodesToDestroyQueue.front());
			Node::NodesToDestroyQueue.pop();
		}

		//rendering
		#if STATISTICS
		Renderer2D::ResetStats();
		#endif

		{
			PROFILE_SCOPE("Renderer Prep");
			m_SceneFrameBuffer->Bind();
			RenderCommand::SetClearColor({ 198 / 255.0f, 239 / 255.0f, 249 / 255.0f, 1 });
			RenderCommand::Clear();
			uint32_t clear = 0;
			m_SceneFrameBuffer->ClearColorAttachment(1, &clear);
			PX_CORE_ASSERT(Camera::Main() != nullptr, "There is no main camera!");
			Camera::Main()->RecalculateViewMatrix();
			Renderer2D::BeginScene(Camera::Main());
		}


		//only run per tick rate
		auto time = std::chrono::high_resolution_clock::now();
		if (m_FixedUpdateRate > 0 &&
			std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count()
			-
			std::chrono::time_point_cast<std::chrono::microseconds>(m_FixedUpdateTime).time_since_epoch().count()
			>= (1.0f / m_FixedUpdateRate) * 1000000.0f)
		{
			PROFILE_SCOPE("Fixed Update");

			m_FixedUpdateTime = time;
			
			for (auto node : Node::Nodes)
			{
				
				
				if (node.second != nullptr)
				{
					node.second->OnUpdate(ts);
					node.second->OnFixedUpdate();
					node.second->OnRender();
				}
				else
				{
					//node was null so add it to list to delete
					m_NullNodeQueue.push(node.first);
				}
			}

		}
		else
		{
			for (auto node : Node::Nodes)
			{
				if (node.second != nullptr)
				{
					node.second->OnUpdate(ts);
					node.second->OnRender();
				}
				else
				{
					//node was null so add it to list to delete
					m_NullNodeQueue.push(node.first);
				}
			}
		}

		//remove null nodes from map
		while (!m_NullNodeQueue.empty())
		{
			Node::Nodes.erase(m_NullNodeQueue.front());
			m_NullNodeQueue.pop();
		}
		/*auto[x,y] = Input::GetMousePosition();
		glm::vec2 worldPos = m_CameraNode->MouseToWorldPos({x, y});
		PX_TRACE("Mouse Pos: ({0},{1})", x, y);
		PX_TRACE("World Pos: ({0},{1})", worldPos.x, worldPos.y);

		Renderer2D::DrawText("Hello!", glm::mat4(1), FontLibrary::GetFont("Aseprite"));*/



		Renderer2D::EndScene();

		auto mp = Input::GetMousePosition();
		//flip the y so bottom left is 0,0
		mp.y = m_ViewportSize.y - mp.y;
		if (mp.x >= 0 && mp.x < m_ViewportSize.x && mp.y >= 0 && mp.y < m_ViewportSize.y)
		{
			uint32_t nodeID; m_SceneFrameBuffer->ReadPixel(1, mp.x, mp.y, &nodeID);
			Node::s_HoveredNodeID = nodeID;
		}

		

		m_SceneFrameBuffer->Unbind();
		//m_SceneFrameBuffer->BindColorAttachmentTexture(0);
		Renderer2D::DrawScreenQuad(m_SceneFrameBuffer->GetColorAttachmentRendererID(0));
	}

	void SceneLayer::DrawNodeTree(Ref<Node> Node)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ((Node == m_SelectedNode) ? ImGuiTreeNodeFlags_Selected : 0);

		bool opened = ImGui::TreeNodeEx((void*)(uint32_t)Node->GetUUID(), flags, Node->m_Name.c_str());

		if (ImGui::IsItemClicked())
		{
			m_SelectedNode = Node;
		}

		if (opened)
		{
			//get child entities displayed as well
			for (auto& node : Node->m_Children)
			{
				DrawNodeTree(node);
			}
			ImGui::TreePop();
		}

	}

	void SceneLayer::OnImGuiRender()
	{
		//PX_TRACE("Node Count ImGui: {0}", Node::Nodes.size());
		if (m_Debug)
		{
			if (ImGui::Begin("Scene Hierarchy"))
			{
				for (auto& node : m_RootNode.m_Children)
				{
					DrawNodeTree(node);
				}
			}
			ImGui::End();


			if (ImGui::Begin("Inspector"))
			{
				if (m_SelectedNode != nullptr)
				{
					m_SelectedNode->OnInspectorRender();
				}
			}
			ImGui::End();
		}

		for (auto node : Node::Nodes)
		{
			if (node.second != nullptr)
			{
				node.second->OnImGuiRender();
			}
			else
			{
				//node was null so add it to list to delete
				m_NullNodeQueue.push(node.first);
			}
			
		}

		//remove null nodes from map
		while (!m_NullNodeQueue.empty())
		{
			Node::Nodes.erase(m_NullNodeQueue.front());
			m_NullNodeQueue.pop();
		}

		//auto dockID = ImGui::DockSpaceOverViewport(ImGui::GetID("SceneLayerDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		//ImGui::SetNextWindowDockID(dockID);
		//if (ImGui::Begin("Scene", (bool*)0, ImGuiWindowFlags_NoTitleBar))
		//{
		//	//m_SceneViewIsFocused = ImGui::IsWindowFocused();

		//	auto viewportOffset = ImGui::GetCursorScreenPos();
		//	auto newViewportSize = ImGui::GetContentRegionAvail();

		//	m_ViewportBounds[0] = { viewportOffset.x, viewportOffset.y };
		//	m_ViewportBounds[1] = { viewportOffset.x + newViewportSize.x, viewportOffset.y + newViewportSize.y };

		//	//Application::Get().GetImGuiLayer()->BlockEvents(false);
		//	ImGui::Image(
		//		(ImTextureID)(m_SceneFrameBuffer->GetColorAttachmentRendererID(0)),
		//		newViewportSize,
		//		ImVec2(0, 1),
		//		ImVec2(1, 0),
		//		ImVec4(1, 1, 1, 1)
		//		//ImVec4(1, 1, 1, 1) border color
		//	);
		//	//m_ViewportOffset = ImGui::GetItemRectMin();

		//	if (m_ViewportSize.x != newViewportSize.x || m_ViewportSize.y != newViewportSize.y)
		//	{
		//		m_ViewportSize = { newViewportSize.x, newViewportSize.y };
		//		//m_OrthographicCameraController.SetAspect(m_ViewportSize.y / m_ViewportSize.x);
		//		m_SceneFrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		//	}

		//	ImGui::End();
		//}
		//ImGui::PopStyleVar();
	}

	void SceneLayer::OnEvent(Event& e)
	{
		
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(PX_BIND_EVENT_FN(SceneLayer::OnWindowResizeEvent));
		dispatcher.Dispatch<KeyPressedEvent>(PX_BIND_EVENT_FN(SceneLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(PX_BIND_EVENT_FN(SceneLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(PX_BIND_EVENT_FN(SceneLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(PX_BIND_EVENT_FN(SceneLayer::OnMouseScrolledEvent));
	}

	glm::ivec2 SceneLayer::GetMousePositionImGui()
	{
		///if not using a framebuffer / imgui image, just use Pyxis::Input::GetMousePosition();
		//TODO: Set up ifdef for using imgui? or just stop using imgui... lol
		auto [mx, my] = ImGui::GetMousePos();

		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;

		return { (int)mx, (int)my };
	}

	bool SceneLayer::OnWindowResizeEvent(WindowResizeEvent& event)
	{
		m_ViewportSize = { event.GetWidth(), event.GetHeight()};
		Camera* cam = Camera::Main();
		if (cam != nullptr)
		{
			cam->SetAspect(m_ViewportSize.y / m_ViewportSize.x);
		}
		//m_OrthographicCameraController.SetAspect(m_ViewportSize.y / m_ViewportSize.x);
		m_SceneFrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		return false;
	}

	bool SceneLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		if (event.GetKeyCode() == PX_KEY_O)
		{
			json j;
			m_RootNode.Serialize(j);
			std::cout << j.dump(4) << std::endl;
			PX_CORE_TRACE("De-Serializing the collected nodes...");
			Ref<Node> newNode = Node::DeserializeNode(j);

			if (newNode != nullptr)
			{
				Ref<Node> clonedNode = CreateRef<Node>("Cloned Nodes");
				m_RootNode.AddChild(clonedNode);
				clonedNode->AddChild(newNode);
			}
			
			
		}
		return false;
	}

	bool SceneLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		//let the UI keep track of what has been pressed, so that way buttons can be on release!
		UI::UINode::s_MousePressedNodeID = Node::s_HoveredNodeID;

		//see if the node is valid, and if it is then send the event through.
		if (Node::Nodes.contains(Node::s_HoveredNodeID))
		{
			//the hovered node is valid

			//try to cast it to a UI node, and call onMousePressed
			if (UI::UINode* uinode = dynamic_cast<UI::UINode*>(Node::Nodes[Node::s_HoveredNodeID]))
			{
				uinode->OnMousePressed(event.GetMouseButton());
			}

		}
		return false;
	}


	/// <summary>
	/// checks to see if we released on the same object we pressed, and if so calls
	/// OnMouseReleased with continuous as true. otherwise, will call onmousereleased
	/// on both the previously selected object, and the hovered object. 
	/// </summary>
	/// <param name="event"></param>
	/// <returns></returns>
	bool SceneLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event)
	{
		if (UI::UINode::s_MousePressedNodeID == Node::s_HoveredNodeID)
		{
			//the hovered node is what we pressed last
			if (Node::Nodes.contains(Node::s_HoveredNodeID))
			{
				//the hovered node is valid
				
				//try to cast it to UI 
				if (UI::UINode* uinode = dynamic_cast<UI::UINode*>(Node::Nodes[Node::s_HoveredNodeID]))
				{
					uinode->OnMouseReleased(event.GetMouseButton(), true);
				}
			}
		}
		else
		{
			//the hovered node is not what we pressed originally, so call released on original press
			if (Node::Nodes.contains(UI::UINode::s_MousePressedNodeID))
			{
				//the hovered node is valid

				//try to cast it to UI 
				if (UI::UINode* uinode = dynamic_cast<UI::UINode*>(Node::Nodes[UI::UINode::s_MousePressedNodeID]))
				{
					uinode->OnMouseReleased(event.GetMouseButton(), false);
				}
			}

			///here if we want to call released on UI nodes without having to press first?
			//if (Node::Nodes.contains(Node::s_HoveredNodeID))
			//{
			//	//the hovered node is valid

			//	//try to cast it to UI 
			//	if (UI::UINode* uinode = dynamic_cast<UI::UINode*>(Node::Nodes[Node::s_HoveredNodeID]))
			//	{
			//		uinode->OnMouseReleased(event.GetMouseButton(), false);
			//	}
			//}
		}

		
		return false;
	}

	bool SceneLayer::OnMouseScrolledEvent(MouseScrolledEvent& event)
	{
		return false;
	}
	

}
