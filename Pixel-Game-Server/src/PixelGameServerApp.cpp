#include <Pyxis.h>
//---------- Entry Point ----------//
#include <Pyxis/Core/EntryPoint.h>

#include "PixelGameServer.h"

namespace Pyxis
{
	class PixelGameServerApplication : public Application {
	public:
		PixelGameServerApplication() : Application("Pixel-Game-Server", 400, 50)
		{
			PushLayer(new PixelGameServer());
		}
		~PixelGameServerApplication()
		{

		}
	};

	Application* Pyxis::CreateApplication() {
		return new PixelGameServerApplication();
	}
}

