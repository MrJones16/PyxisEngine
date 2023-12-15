#include <Pyxis.h>

class ExampleLayer : public Pyxis::Layer
{
public:
	ExampleLayer() : Layer("Example")
	{

	}

	void OnUpdate() override
	{
		//PX_INFO("ExampleLayer::Update");
	}

	void OnEvent(Pyxis::Event& event) override
	{
		PX_TRACE("{0}", event);
	}

};

class Sandbox : public Pyxis::Application {
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Pyxis::ImGuiLayer());
	}
	~Sandbox()
	{

	}

};

Pyxis::Application* Pyxis::CreateApplication() {
	return new Sandbox();
}