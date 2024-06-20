#include <Pyxis.h>
#include "PixelGameServer.h"




class PixelGameServerApplication : public Pyxis::Application {
public:
	PixelGameServerApplication() : Application("Pyxis Server", 400, 50)
	{
		PushLayer(&m_ServerLayer);
	}
	~PixelGameServerApplication()
	{

	}
public:
	Pyxis::PixelGameServer m_ServerLayer = Pyxis::PixelGameServer(60000);
};

/*Application* Pyxis::CreateApplication() {
	return new PixelGameServerApplication();
}*/


//---------- Entry Point ----------//
//#include <Pyxis/Core/EntryPoint.h>

#ifdef PX_PLATFORM_WINDOWS

int main(int argc, char** argv) {

#ifndef PX_DIST
	Pyxis::Log::Init();
	PX_CORE_WARN("Initialized Log");
#endif // !PX_DIST

	PixelGameServerApplication app;

	//auto app = Pyxis::CreateApplication();
	app.Run();

	return 0;
}
	

#endif // !PX_PLATFORM_WINDOWS
