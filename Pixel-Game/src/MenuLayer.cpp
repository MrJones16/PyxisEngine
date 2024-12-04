#include "MenuLayer.h"

namespace Pyxis
{
	MenuLayer::MenuLayer()
	{
	}

	MenuLayer::~MenuLayer()
	{
		if (!m_GameLayerAttached)
		{
			delete m_GameLayer;
		}
	}

	void MenuLayer::OnAttach()
	{
	}

	void MenuLayer::OnDetach()
	{

	}

	void MenuLayer::OnUpdate(Timestep ts)
	{
		//if the game layer isn't attached, then we need to be running the network update for it
		if (!m_GameLayerAttached)
		{
			m_GameLayer->UpdateInterface();
		}

	}

	

	void MenuLayer::OnImGuiRender()
	{
		if (m_GameLayer->GetConnectionStatus() == Network::ClientInterface::ConnectionStatus::Disconnected)
		{
			auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MenuDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
			//show main menu
			ImGui::SetNextWindowDockID(dock);
			if (ImGui::Begin("Main Menu", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Pyxis");

				if (ImGui::Button("Start Singleplayer"))
				{
					m_GameLayer->StartSingleplayer();
					AttachGameLayer();
				}

				ImGui::InputText("IP Address", m_InputAddress, 22);
				if (ImGui::Button("Connect"))
				{
					m_GameLayer->StartMultiplayer(std::string(m_InputAddress));
					AttachGameLayer();
				}

			}
			ImGui::End();
			
		}
		
	}

	void MenuLayer::OnEvent(Event& e)
	{

	}
	void MenuLayer::AttachGameLayer()
	{
		//stops displaying the main menu and play game
		Application::Get().PushLayer(m_GameLayer);
		m_GameLayerAttached = true;
	}
	void MenuLayer::DetachGameLayer()
	{
		//Make the menu an active layer and hide the game layer
		Application::Get().PopLayerQueue(m_GameLayer);
		m_GameLayerAttached = false;
		//i would like to clear the game state here as well. TODO
	}

	void MenuLayer::FailedToConnect()
	{

	}
}