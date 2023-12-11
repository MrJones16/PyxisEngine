#include <Pyxis.h>

class Sandbox : public Pyxis::Application {
public:
	Sandbox()
	{

	}
	~Sandbox()
	{

	}

};

Pyxis::Application* Pyxis::CreateApplication() {
	return new Sandbox();
}