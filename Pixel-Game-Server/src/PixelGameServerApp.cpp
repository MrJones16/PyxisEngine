#include <Pyxis.h>
#include "PixelGameServer.h"
#include <steam/steam_api.h>




class PixelGameServerApplication : public Pyxis::Application {
public:
	PixelGameServerApplication() : Application("Pyxis Server", 350, 200)
	{
		bool success = SteamAPI_Init();
		PX_CORE_ASSERT(success, "Failed to init steam api!");

		Pyxis::Ref<Pyxis::PixelGameServer> ref = Pyxis::CreateRef<Pyxis::PixelGameServer>();
		PushLayer(ref);
		m_ServerLayer = ref;
	}
	~PixelGameServerApplication()
	{
		SteamAPI_Shutdown();
	}
public:
	std::weak_ptr<Pyxis::PixelGameServer> m_ServerLayer;
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
