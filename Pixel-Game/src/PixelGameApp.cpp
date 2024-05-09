#include <Pyxis.h>

//---------- Entry Point ----------//
#include <Pyxis/Core/EntryPoint.h>

#include "GameLayer.h"

namespace Pyxis
{
	class PixelGame : public Pyxis::Application {
	public:
		PixelGame()
			: Application("Pixel Game", 256 * 3, 256 * 3)
		{
			PushLayer(new GameLayer());
		}
		~PixelGame()
		{

		}
	};

	Pyxis::Application* Pyxis::CreateApplication() {
		return new PixelGame();
	}
}
