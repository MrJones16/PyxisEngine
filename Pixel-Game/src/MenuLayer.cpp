#include "MenuLayer.h"

namespace Pyxis
{
	MenuLayer::MenuLayer()
	{
	}

	MenuLayer::~MenuLayer()
	{

	}

	void MenuLayer::OnAttach()
	{
	}

	void MenuLayer::OnDetach()
	{
	}


	void MenuLayer::OnUpdate(Timestep ts)
	{
		if (m_SinglePlayerLayer.expired() && m_MultiplayerLayer.expired() && m_HostingLayer.expired())
		{
			m_AnyGameLayerAttached = false;
		}
	}

	

	void MenuLayer::OnImGuiRender()
	{
		if (!m_AnyGameLayerAttached)
		{
			auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MenuDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
			//show main menu
			ImGui::SetNextWindowDockID(dock);
			if (ImGui::Begin("Main Menu", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Pyxis");

				if (ImGui::Button("Start Singleplayer"))
				{
					Ref<SingleplayerGameLayer> ref = CreateRef<SingleplayerGameLayer>();
					ref->Start();
					
					Application::Get().PushLayer(ref);
					m_SinglePlayerLayer = ref;
					m_AnyGameLayerAttached = true;
				}

				ImGui::InputText("IP Address", m_InputAddress, 22);
				if (ImGui::Button("Connect"))
				{
					Ref<MultiplayerGameLayer> ref = CreateRef<MultiplayerGameLayer>();
					PX_CORE_TRACE("References to Layer: {0}", ref.use_count());
					ref->ConnectIP(std::string(m_InputAddress));
					Application::Get().PushLayer(ref);
					m_MultiplayerLayer = ref;
					PX_CORE_TRACE("References to Layer: {0}", ref.use_count());
					m_AnyGameLayerAttached = true;
				}

				if (ImGui::Button("Host over Steam"))
				{
					Ref<HostingGameLayer> ref = CreateRef<HostingGameLayer>();
					ref->StartP2P(0);
					Application::Get().PushLayer(ref);
					m_HostingLayer = ref;
					m_AnyGameLayerAttached = true;
				}

				if (ImGui::Button("Host by IP"))
				{
					Ref<HostingGameLayer> ref = CreateRef<HostingGameLayer>();
					ref->StartIP(0);
					Application::Get().PushLayer(ref);
					m_HostingLayer = ref;
					m_AnyGameLayerAttached = true;
				}

				if (ImGui::Button("Quit Game"))
				{
					Application::Get().Close();
				}

			}
			ImGui::End();
			
		}
		
	}

	void MenuLayer::OnEvent(Event& e)
	{

	}

	void MenuLayer::FailedToConnect()
	{

	}
}