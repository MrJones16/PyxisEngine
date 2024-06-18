#include <Pyxis.h>
//---------- Entry Point ----------//
#include <Pyxis/Core/EntryPoint.h>

#include "Sandbox2D.h"

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