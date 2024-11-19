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


		//Attach / detach game depending on connection state
		if (m_GameLayer->GetConnectionStatus() == Network::ClientInterface::ConnectionStatus::Connected)
		{
			//if we are connected to the server, then lets make sure the game is attached
			if (!m_GameLayerAttached)
			{
				//attach the game layer if it isnt yet
				AttachGameLayer();
			}
		}
		else
		{
			//we aren't connected to the server, so lets make sure the game is detached
			if (m_GameLayerAttached)
			{
				//detach on disconnect
				DetachGameLayer();
				//m_GameLayer->GetConnectionStatus() = GameLayer::NotConnected;
			}
		}
	}

	

	void MenuLayer::OnImGuiRender()
	{

		switch (m_GameLayer->GetConnectionStatus())
		{
		case Network::ClientInterface::ConnectionStatus::Disconnected:
		{
			auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MenuDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
			//show main menu
			ImGui::SetNextWindowDockID(dock);
			if (ImGui::Begin("Main Menu", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Pyxis");

				ImGui::InputText("IP Address", m_InputAddress, 22);
				if (ImGui::Button("Connect"))
				{
					m_GameLayer->ConnectToServer(std::string(m_InputAddress));
				}

			}
			ImGui::End();
			break;
		}
		case Network::ClientInterface::ConnectionStatus::Connecting:
		{
			auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MenuDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
			//show that we are connecting
			ImGui::SetNextWindowDockID(dock);
			if (ImGui::Begin("Connecting Screen", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Connecting To Server...");
			}
			ImGui::End();
			break;
		}
		case Network::ClientInterface::ConnectionStatus::LostConnection:
		case Network::ClientInterface::ConnectionStatus::FailedToConnect:
		{
			auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MenuDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
			ImGui::SetNextWindowDockID(dock);
			if (ImGui::Begin("Connection Failed", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Connection Failed");

				ImGui::Text(m_GameLayer->m_ConnectionErrorMessage.c_str());
				if (ImGui::Button("Okay"))
				{
					m_GameLayer->Disconnect();
				}
			}
			ImGui::End();
			break;
		}
		default:
		{
			//do nothing
			auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MenuDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
			ImGui::SetNextWindowDockID(dock);
			if (ImGui::Begin("Connected", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Connected");
			}
			ImGui::End();
			break;
		}

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