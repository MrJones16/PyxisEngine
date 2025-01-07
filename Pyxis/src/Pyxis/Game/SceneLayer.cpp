#include "SceneLayer.h"
#include "Node.h"
#include <Pyxis/Renderer/Renderer2D.h>
#include <Pyxis/Renderer/RenderCommand.h>
#include <Pyxis/Renderer/UI.h>

namespace Pyxis
{

	SceneLayer::SceneLayer()
	{

	}

	SceneLayer::~SceneLayer()
	{

	}

	void SceneLayer::OnAttach()
	{
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
		PROFILE_SCOPE("GameLayer::OnUpdate");

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
			Renderer2D::BeginScene(m_MainCamera);
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
				node.second->OnUpdate(ts);
				node.second->OnFixedUpdate();
				node.second->OnRender();
			}

		}
		else
		{
			for (auto node : Node::Nodes)
			{
				node.second->OnUpdate(ts);
				node.second->OnRender();
			}
		}

		Renderer2D::EndScene();

		auto mp = GetMousePositionImGui();
		//flip the y so bottom left is 0,0
		mp.y = m_ViewportSize.y - mp.y;
		if (mp.x >= 0 && mp.x < m_ViewportSize.x && mp.y >= 0 && mp.y < m_ViewportSize.y)
		{
			uint32_t nodeID; m_SceneFrameBuffer->ReadPixel(1, mp.x, mp.y, &nodeID);
			m_HoveredNodeID = nodeID;
		}

		m_SceneFrameBuffer->Unbind();
	}

	void SceneLayer::OnImGuiRender()
	{
		auto dockID = ImGui::DockSpaceOverViewport(ImGui::GetID("SceneLayerDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		ImGui::SetNextWindowDockID(dockID);
		if (ImGui::Begin("Scene", (bool*)0, ImGuiWindowFlags_NoTitleBar))
		{
			//m_SceneViewIsFocused = ImGui::IsWindowFocused();

			auto viewportOffset = ImGui::GetCursorScreenPos();
			auto newViewportSize = ImGui::GetContentRegionAvail();

			m_ViewportBounds[0] = { viewportOffset.x, viewportOffset.y };
			m_ViewportBounds[1] = { viewportOffset.x + newViewportSize.x, viewportOffset.y + newViewportSize.y };

			//Application::Get().GetImGuiLayer()->BlockEvents(false);
			ImGui::Image(
				(ImTextureID)(m_SceneFrameBuffer->GetColorAttachmentRendererID(0)),
				newViewportSize,
				ImVec2(0, 1),
				ImVec2(1, 0),
				ImVec4(1, 1, 1, 1)
				//ImVec4(1, 1, 1, 1) border color
			);
			//m_ViewportOffset = ImGui::GetItemRectMin();

			if (m_ViewportSize.x != newViewportSize.x || m_ViewportSize.y != newViewportSize.y)
			{
				m_ViewportSize = { newViewportSize.x, newViewportSize.y };
				//m_OrthographicCameraController.SetAspect(m_ViewportSize.y / m_ViewportSize.x);
				m_SceneFrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			}

			ImGui::End();
		}
		ImGui::PopStyleVar();
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
		return false;
	}

	bool SceneLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		return false;
	}

	bool SceneLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		//let the UI keep track of what has been pressed, so that way buttons can be on release!
		UI::UINode::s_MousePressedNodeID = m_HoveredNodeID;

		//see if the node is valid, and if it is then send the event through.
		if (Node::Nodes.contains(m_HoveredNodeID))
		{
			//the hovered node is valid

			//try to cast it to a UI node, and call onMousePressed
			if (UI::UINode* uinode = dynamic_cast<UI::UINode*>(Node::Nodes[m_HoveredNodeID]))
			{
				uinode->OnMousePressed(event.GetMouseButton());
			}

		}
		return false;
	}

	bool SceneLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event)
	{
		if (Node::Nodes.contains(m_HoveredNodeID))
		{
			//the hovered node is valid

			//try to cast it to UI 
			if (UI::UINode* uinode = dynamic_cast<UI::UINode*>(Node::Nodes[m_HoveredNodeID]))
			{
				uinode->OnMouseReleased(event.GetMouseButton());
				//if this was pressed on before as well, call onClick()
				if (UI::UINode::s_MousePressedNodeID == m_HoveredNodeID)
					uinode->OnClick();
			}



		}
		return false;
	}

	bool SceneLayer::OnMouseScrolledEvent(MouseScrolledEvent& event)
	{
		return false;
	}
	

}
