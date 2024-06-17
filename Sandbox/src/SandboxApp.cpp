#include <Pyxis.h>
//---------- Entry Point ----------//
#include <Pyxis/Core/EntryPoint.h>

#include "Sandbox2D.h"


class ExampleLayer : public Pyxis::Layer
{
private:
	

public:
	ExampleLayer() : Layer("Example")
	{
		
	}
	

	void OnUpdate(Pyxis::Timestep ts) override
	{

	}

	virtual void OnImGuiRender() override
	{
		
	}

	void OnEvent(Pyxis::Event& event) override
	{
		Pyxis::EventDispatcher dispatcher(event);
		//dispatcher.Dispatch<Pyxis::KeyPressedEvent>(PX_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Pyxis::KeyPressedEvent& event) {
		//PX_CORE_INFO("Pressed Key");
		return false;
	}

};

class Sandbox : public Pyxis::Application {
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}
	~Sandbox()
	{

	}
};

Pyxis::Application* Pyxis::CreateApplication() {
	return new Sandbox();
}