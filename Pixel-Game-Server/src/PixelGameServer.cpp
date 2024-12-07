#include "PixelGameServer.h"

#include <imgui.h>
//#include <glm/gtc/type_ptr.hpp>

//#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>


namespace Pyxis
{
	PixelGameServer::PixelGameServer(uint16_t port) : Layer("Pyxis Server"),
		m_OrthographicCameraController(5, 9.0f / 16.0f, -100, 100),
		m_World()//m_World("../Pixel-Game/assets")
	{
		m_SteamPort = port;
		m_World.m_ServerMode = true;
	}

	PixelGameServer::~PixelGameServer()
	{
		//stop the server
		Stop();
	}

	void PixelGameServer::OnAttach()
	{
		//m_ProfileResults = std::vector<ProfileResult>();
		Pyxis::Renderer2D::Init();

		//setup the log so that imgui can display messages and not just the console
		// TODO: setup logs / sinks for spdlog
		//Pyxis::Log::GetClientLogger() ...

		//STEAMTESTING
		Start(m_SteamPort);
	}

	void PixelGameServer::OnDetatch()
	{

	}

	void PixelGameServer::OnUpdate(Pyxis::Timestep ts)
	{
		UpdateInterface();
		HandleMessages();



		auto time = std::chrono::high_resolution_clock::now();
		//only update if there are players
		if (m_ClientDataMap.size() > 0)
		if (m_TickRate > 0 &&
			std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count()
			-
			std::chrono::time_point_cast<std::chrono::microseconds>(m_UpdateTime).time_since_epoch().count()
			>= (1.0f / m_TickRate) * 1000000.0f)
		{
			//pack the merged tick into a message and send to all clients
			Network::Message msg;
			msg.header.id = static_cast<uint32_t>(GameMessage::Game_MergedTickClosure);
			msg << m_CurrentMergedTickClosure.m_Data;
			msg << m_CurrentMergedTickClosure.m_ClientCount;
			msg << m_InputTick;
			SendMessageToAllClients(msg);
			HandleTickClosure(m_CurrentMergedTickClosure);

			m_InputTick++;

			//reset the merged tick
			m_CurrentMergedTickClosure = MergedTickClosure();
		}
		
	}

	void PixelGameServer::HandleMessages()
	{
		Ref<Network::Message> msg;
		while (PollMessage(msg))
		{
			switch (static_cast<GameMessage>(msg->header.id))
			{
			case GameMessage::Client_ClientData:
			{
				//add the client data to a map so we can send it to new players later
				PX_TRACE("Recieved client data from client: [{0}]:{1}", msg->clientHConnection, m_ClientDataMap[msg->clientHConnection].m_Name);
				m_ClientDataMap[msg->clientHConnection];
				*msg >> m_ClientDataMap[msg->clientHConnection];
				break;
			}
			case GameMessage::Client_RequestAllClientData:
			{
				PX_TRACE("Recieved request for client data from: [{0}]:{1}", msg->clientHConnection, m_ClientDataMap[msg->clientHConnection].m_Name);
				Network::Message clientDataMsg;
				clientDataMsg.header.id = static_cast<uint32_t>(GameMessage::Server_AllClientData);
				uint32_t numClients = 0;
				for (auto& clientPair : m_ClientDataMap)
				{
					if (clientPair.first != msg->clientHConnection)
					{
						clientDataMsg << clientPair.second;
						clientDataMsg << clientPair.first;
						numClients++;
					}
				}
				clientDataMsg << numClients;
				SendMessageToClient(msg->clientHConnection, clientDataMsg);
				PX_TRACE("Sent All Client Data to {0}", msg->clientHConnection);
				break;
			}
			case GameMessage::Client_RequestGameData:
			{
				//client is requesting the world data, so send it!
				//send the world and rigid body data
				Network::Message gameDataMsg;
				gameDataMsg.header.id = static_cast<uint32_t>(GameMessage::Server_GameData);
				//in order to send the game data, i'll pause the game for now while its sending?
				//or maybe not, and ill just keep updating and sending the client the 
				//merged ticks so it can catch up afterwards
				m_World.GetWorldData(gameDataMsg);
				gameDataMsg << m_World.m_Running;
				gameDataMsg << m_InputTick;
				PX_TRACE("Client Requested world Data. Size: {0}", gameDataMsg.size());
				SendMessageToClient(msg->clientHConnection, gameDataMsg);

				//as soon as a new client joins the game,
				//we have to reset everyones box2d simulation, so send that now so everyone stays in sync!
				Network::Message b2ResetMsg;
				b2ResetMsg.header.id = static_cast<uint32_t>(GameMessage::Game_ResetBox2D);
				m_World.ResetBox2D();
				b2ResetMsg << m_World.m_SimulationTick;
				SendMessageToAllClients(b2ResetMsg, msg->clientHConnection);
				break;
			}
			case GameMessage::Game_TickClosure:
			{
				//merge the recieved tick closure into our current merged tick closure!

				TickClosure tc;
				*msg >> tc.m_InputActionCount;
				*msg >> tc.m_Data;
				m_CurrentMergedTickClosure.AddTickClosure(tc, msg->clientHConnection);
				//finished! we just wait till it's time to send the message, 
				break;
			}
			}

		}
	}
	

	void PixelGameServer::OnImGuiRender()
	{

		auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MainDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);

		//top menu bar
		if (ImGui::BeginMainMenuBar())
		{
			//ImGui::DockSpaceOverViewport();
			if (ImGui::BeginMenu("File"))
			{
				ImGui::Text("nothing here yet!");
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::SetNextWindowDockID(dock);
		if (ImGui::Begin("Server"))
		{
			//ImGui::Text("Sleep Timer");
			//ImGui::ProgressBar(static_cast<float>(m_SleepDelay) / static_cast<float>(m_SleepDelayMax));
			ImGui::Text(("Players:" + std::to_string(m_ClientDataMap.size())).c_str());
		}
		ImGui::End();

		/*if (ImGui::Begin("NetworkDebug"))
		{
			ImGui::Text(("Input Tick:" + std::to_string(m_InputTick)).c_str());
		}
		ImGui::End();*/
	}

	void PixelGameServer::OnEvent(Pyxis::Event& e)
	{
		//m_OrthographicCameraController.OnEvent(e);
		//Pyxis::EventDispatcher dispatcher(e);
		//dispatcher.Dispatch<Pyxis::WindowResizeEvent>(PX_BIND_EVENT_FN(Sandbox2D::OnWindowResizeEvent));
	}

	bool PixelGameServer::OnWindowResizeEvent(Pyxis::WindowResizeEvent& event) {
		//m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
		//Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		////Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		return false;
	}

	//bool PixelGameServer::OnClientConnect(std::shared_ptr<Network::Connection<GameMessage>> client)
	//{
	//	//I am able to block the incoming connection if i desire here
	//	m_PlayerCount++;

	//	return true;
	//}

	/*void PixelGameServer::OnClientDisconnect(std::shared_ptr<Network::Connection<GameMessage>> client)
	{
		PX_TRACE("Removing Client [{0}]", client->GetID());
		m_PlayerCount--;
		m_ClientsNeededForTick.erase(client->GetID());

		Network::Message<GameMessage> disconnectMsg;
		disconnectMsg.header.id = GameMessage::Server_ClientDisconnected;
		disconnectMsg << client->GetID();
		MessageAllClients(disconnectMsg, client);
		return;
	}*/

	
	void PixelGameServer::OnClientDisconnect(HSteamNetConnection client)
	{
		//remove client from map
		m_ClientDataMap.erase(client);
	}

	/// <summary>
	/// Volatile to the merged tick closure.
	/// </summary>
	/// <param name="tc"></param>
	void PixelGameServer::HandleTickClosure(MergedTickClosure& tc)
	{
		for (int i = 0; i < tc.m_ClientCount; i++)
		{
			HSteamNetConnection clientID;
			tc >> clientID;
			uint32_t inputActionCount;
			tc >> inputActionCount;
			for (int i = 0; i < inputActionCount; i++)
			{
				InputAction IA;
				tc >> IA;
				switch (IA)
				{
				/*case InputAction::Add_Player:
				{
					uint64_t ID;
					tc >> ID;

					glm::ivec2 pixelPos;
					tc >> pixelPos;

					m_World.CreatePlayer(ID, pixelPos);
					break;
				}*/
				case InputAction::PauseGame:
				{
					m_World.m_Running = false;
					break;
				}
				case InputAction::ResumeGame:
				{
					m_World.m_Running = true;
					break;
				}
				case InputAction::TransformRegionToRigidBody:
				{
					uint64_t UUID;
					tc >> UUID;

					glm::ivec2 maximum;
					tc >> maximum;

					glm::ivec2 minimum;
					tc >> minimum;

					b2BodyType type;
					tc >> type;

					int width = (maximum.x - minimum.x) + 1;
					int height = (maximum.y - minimum.y) + 1;
					if (width * height <= 0) break;
					glm::ivec2 newMin = maximum;
					glm::ivec2 newMax = minimum;
					//iterate over section and find the width, height, center, ect
					int mass = 0;
					for (int x = 0; x < width; x++)
					{
						for (int y = 0; y < height; y++)
						{
							glm::ivec2 pixelPos = glm::ivec2(x + minimum.x, y + minimum.y);
							auto& element = m_World.GetElement(pixelPos);
							auto& elementData = m_World.m_ElementData[element.m_ID];
							if ((elementData.cell_type == ElementType::solid || elementData.cell_type == ElementType::movableSolid) && element.m_Rigid == false)
							{
								if (pixelPos.x < newMin.x) newMin.x = pixelPos.x;
								if (pixelPos.y < newMin.y) newMin.y = pixelPos.y;
								if (pixelPos.x > newMax.x) newMax.x = pixelPos.x;
								if (pixelPos.y > newMax.y) newMax.y = pixelPos.y;
								mass++;
							}
						}
					}
					if (mass < 2) continue;//skip if we are 1 element or 0
					PX_TRACE("transforming {0} elements to a rigid body", mass);

					width = (newMax.x - newMin.x) + 1;
					height = (newMax.y - newMin.y) + 1;

					glm::ivec2 origin = { width / 2, height / 2 };
					std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements;
					for (int x = 0; x < width; x++)
					{
						for (int y = 0; y < height; y++)
						{
							glm::ivec2 pixelPos = { x + newMin.x, y + newMin.y };

							//loop over every element, grab it, and make it rigid if it is a movable Solid
							auto& element = m_World.GetElement(pixelPos);
							auto& elementData = m_World.m_ElementData[element.m_ID];
							if ((elementData.cell_type == ElementType::solid || elementData.cell_type == ElementType::movableSolid) && element.m_Rigid == false)
							{
								element.m_Rigid = true;
								//set the elements at the local position to be the element pulled from world
								elements[glm::ivec2(x - origin.x, y - origin.y)] = RigidBodyElement(element, pixelPos);
								m_World.SetElement(pixelPos, Element());
							}
						}
					}
					glm::ivec2 size = newMax - newMin;
					PX_TRACE("Mass is: {0}", mass);
					PixelRigidBody* body = new PixelRigidBody(UUID, size, elements, type, m_World.m_Box2DWorld);
					if (body->m_B2Body == nullptr)
					{
						PX_TRACE("Failed to create rigid body");
						continue;
					}
					else
					{
						m_World.m_PixelBodyMap[body->m_ID] = body;
						auto pixelPos = (newMin + newMax) / 2;
						if (width % 2 == 0) pixelPos.x += 1;
						if (height % 2 == 0) pixelPos.y += 1;
						body->SetPixelPosition(pixelPos);
						m_World.PutPixelBodyInWorld(*body);
					}


					break;
				}
				case Pyxis::InputAction::Input_Move:
				{
					//PX_TRACE("input action: Input_Move");
					break;
				}
				case Pyxis::InputAction::Input_Place:
				{
					//PX_TRACE("input action: Input_Place");
					bool rigid;
					glm::ivec2 pixelPos;
					uint32_t elementID;
					BrushType brush;
					uint8_t brushSize;
					tc >> rigid >> pixelPos >> elementID >> brush >> brushSize;

					m_World.PaintBrushElement(pixelPos, elementID, brush, brushSize);
					break;
				}
				case Pyxis::InputAction::Input_StepSimulation:
				{
					PX_TRACE("input action: Input_StepSimulation");
					m_World.UpdateWorld();
					break;
				}
				case InputAction::Input_MousePosition:
				{
					//add new mouse position to data
					glm::ivec2 mousePos;
					HSteamNetConnection clientID;
					tc >> mousePos >> clientID;
					m_ClientDataMap[clientID].m_CurosrPixelPosition = mousePos;
					break;
				}
				case InputAction::ClearWorld:
				{
					m_World.Clear();
					break;
				}
				default:
				{
					PX_TRACE("Input Action Error. You have probably mis-aligned the data");
					break;
				}
				}
			}
		}
		if (m_World.m_Running)
		{
			m_World.UpdateWorld();
		}
		else
		{
			m_World.UpdateTextures();
		}
	}

}
