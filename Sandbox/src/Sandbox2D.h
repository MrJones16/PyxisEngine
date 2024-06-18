#pragma once

#include "Pyxis.h"
#include <Pyxis/Network/Network.h>
#include "Pyxis/Core/OrthographicCameraController.h"

class SandboxClient : public Pyxis::Network::ClientInterface<Pyxis::Network::CustomMessageTypes>
{
public:
	void PingServer()
	{
		Pyxis::Network::Message<Pyxis::Network::CustomMessageTypes> msg;
		msg.header.id = Pyxis::Network::CustomMessageTypes::ServerPing;
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		msg << timeNow;
		m_Connection->Send(msg);
	}
	void MessageAll()
	{
		Pyxis::Network::Message<Pyxis::Network::CustomMessageTypes> msg;
		msg.header.id = Pyxis::Network::CustomMessageTypes::MessageAll;
		m_Connection->Send(msg);
	}
};

class Sandbox2D : public Pyxis::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach();
	virtual void OnDetatch();

	virtual void OnUpdate(Pyxis::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Pyxis::Event& e) override;
	bool OnKeyPressedEvent(Pyxis::KeyPressedEvent& event);
	bool OnWindowResizeEvent(Pyxis::WindowResizeEvent& event);

private:
	Pyxis::OrthographicCameraController m_OrthographicCameraController;

	//SandboxServer m_ServerInterface;

	SandboxClient m_ClientInterface;


};