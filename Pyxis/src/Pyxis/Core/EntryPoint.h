#pragma once

extern Pyxis::Application* Pyxis::CreateApplication();

int main(int argc, char** argv) {

	#ifndef PX_DIST
		Pyxis::Log::Init();
		PX_CORE_WARN("Initialized Log");
	#endif // !PX_DIST

	
	int a = 5;

	auto app = Pyxis::CreateApplication();
	app->Run();
	delete app;
}