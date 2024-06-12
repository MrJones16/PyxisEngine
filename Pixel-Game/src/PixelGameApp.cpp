#include <Pyxis.h>

//---------- Entry Point ----------//
#include <Pyxis/Core/EntryPoint.h>

#include "GameLayer.h"

/// <summary>
/// The basic things to work on in order:
/// 
/// 
/// [({! DONE !})] Updating surrounding dirty rects if needed! falling sand won't tell others it moved, you know?
///		will be a major help if done
/// 
/// 
/// [({! DONE !})] better drawing UI:
///		</ show what is about to be placed? might be tricky...
///		</ possibly add the element selection to a dock inside game
/// 
/// Adding more elements, properties, ect.
///		
///	Basic elements:
///		/Fire 
///		Smoke
///		/Wood
///		/Oil
///		/Coal
///		Acid
///		
/// Interesting thoughts:
///		Wire ( or metals and have conductive / electric charge property)
///		magnet?
///		Uranium
///		
/// [({! DONE !})] Making elements loaded from a xml / json file, serializing!
///	
/// [in progress]
///	Texture sampling for elements:
///		no texture: randomize slightly
///		With texture: when placed just modulo the position by tex size and use it?
/// 
/// 
/// 
/// World Generation
/// 
/// [({! DONE !})]start with a simple noise library, like fast noise lite, and get a heightmap of dirt?
///	
/// 
/// Possibly making it possible to save and load the world
/// 
/// 
/// 
/// possible rework of element updating, to make it so chunks won't be force updated when trying to unload them?
/// 
/// Box2D rigid bodies and physics implemented into the game engine...
///		sounds like a very painful time
/// 
/// characters / creatures, using pixel mapped animations for changing colors!
/// 
/// 
/// </summary>

namespace Pyxis
{

	class PixelGame : public Pyxis::Application {
	public:
		PixelGame()
			: Application("Pixel Game", 1280, 720)
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
