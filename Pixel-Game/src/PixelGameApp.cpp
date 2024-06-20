#include <Pyxis.h>

//---------- Entry Point ----------//
#include <Pyxis/Core/EntryPoint.h>

#include "GameLayer.h"

/// <summary>
/// The basic things to work on in order:
/// 
/// [({! DONE !})] Updating surrounding dirty rects if needed! falling sand won't tell others it moved, you know?
///		will be a major help if done
/// 
/// 
/// [({! DONE !})] better drawing UI:
///		</ show what is about to be placed? might be tricky...
///		</ possibly add the element selection to a dock inside game
/// 
///		
///	[({! DONE !})] Basic elements:
///		/Fire 
///		Smoke
///		/Wood
///		/Oil
///		/Coal
///		Acid
///		
///		Interesting thoughts:
///		Wire ( or metals and have conductive / electric charge property)
///		magnet?
///		Uranium
///		
/// [({! DONE !})] Making elements loaded from a xml / json file, serializing!
///	
/// 
///	[({! DONE !})] Texture sampling for elements:
///		no texture: randomize slightly
///		With texture: when placed just modulo the position by tex size and use it?
/// 
/// 
/// 
/// World Generation
///		[({! DONE !})]start with a simple noise library, like fast noise lite, and get a heightmap of dirt?
///	
/// 
/// Possibly making it possible to save and load the world
/// 
/// 
/// 
/// possible rework of element updating, to make it so chunks won't be force updated when trying to unload them?
/// 
/// [ MOSTLY DONE ] Box2D rigid bodies and physics implemented into the game engine...
///		sounds like a very painful time
/// 
/// characters / creatures, using pixel mapped animations for changing colors!
/// 
/// MULTIPLAYER??!??!?!
///		experiment with streaming the world data to a second client? i really want this tbh
///		any game is better with friends
/// 
/// multiplayer notes:
/// 
/// the server will hold a main game state, the actual "world" to be saved or loaded.
/// clients will hold a copy of that game state, and a layer on top of that called a "desync state"
/// the desync state will hold the "input actions" of the player right now, and when those input
/// actions finally make a round trip, they will update the game state for the player, and refresh
/// the desync state accordingly
/// 
/// COMBINED NEEDED THINGS:
/// * Input Actions: all input actions used by the player to interact with the game.
/// 
/// 
/// SERVER DATASTRUCTURES:
/// * World (the authoritative world state)
/// 
/// 
/// CLIENT DATASTRUCTURES
/// 
/// * World (game state)
/// * Desynced World (desynced game state for instant player movement, the feel good)
/// * Network Input Listener (Gets all inputs from player and )
/// 
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
