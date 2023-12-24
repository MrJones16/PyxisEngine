#pragma once

#ifdef PX_PLATFORM_WINDOWS

extern Pyxis::Application* Pyxis::CreateApplication();

int main(int argc, char** argv) {

	Pyxis::Log::Init();
	PX_CORE_WARN("Initialized Log");
	int a = 5;
	PX_INFO("Hello! var = {0}", a);

	auto app = Pyxis::CreateApplication();
	app->Run();
	delete app;
}

#endif // !PX_PLATFORM_WINDOWS
