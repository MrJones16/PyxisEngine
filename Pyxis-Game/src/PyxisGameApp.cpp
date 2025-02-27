#include <Pyxis.h>

#include <steam/steam_api.h>
#include <steam/isteamnetworkingutils.h>

//---------- Entry Point ----------//
#include <Pyxis/Core/EntryPoint.h>

#include <Pyxis/Game/SceneLayer.h>

#include "MenuNode.h"
#include "SteamManagerNode.h"


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
/// [({! DONE !})] Box2D rigid bodies and physics implemented into the game engine...
/// 
/// 
/// [({! DONE !})] MULTIPLAYER!!!
/// 
/// 
/// Saving and Loading worlds
/// 
/// 
/// World Generation
///		[({! DONE !})]start with a simple noise library, like fast noise lite, and get a heightmap of dirt?
/// 
/// possible rework of element updating, to make it so chunks won't be force updated when trying to unload them?
///		
/// 
/// Characters / Creatures, using pixel mapped animations for changing colors!
/// 
/// 
/// 
/// Feedback / Things to work on:
/// 
///		-	[Done!] Deterministic falling sand updating!
///		-	[Done!] Box2d simulation synchronized on multiplayer
///		-	[Done!] Pixel bodies storing a vector of their elements
///		-	[Done!] See eachothers cursors
///		-	[Done!] UDP and TCP, dumb tcp...
///		-	[Done!] make it so one player isnt super far ahead on input ticks
///		-	[Done!] Pixel bodies having a hidden bit
///		-	[Done!][hard]	Pixel bodies deforming, splitting
///		-	[Done!][hard]	UI Rework (text rendering, UI interaction, buttons, signals, ect)
/// 
/// 
///		-	Easier building of rigid and kinematic bodies. instead of a selection box...
///		-	Air brush is invisible
/// 
/// 
///		-	static collisions with ground near dynamic bodies
///		-	Pixel bodies throwing liquid/sand in the air (aka particle system)
/// 
/// 
///		-	Drag and Drop a png? or blueprints?
///		-	Saving and loading worlds (somewhat implemented with networking!)
/// 
/// 
///		-	[hard]	Players and Creatures, and their respective loading/unloading
///		-	Player movement
/// 
/// </summary>

namespace Pyxis
{

	class PyxisGame : public Pyxis::Application {
	public:
		PyxisGame()
			: Application("Pyxis", 1280, 720, "assets/Icon.png")
		{
			bool success = SteamAPI_Init();
			PX_CORE_ASSERT(success, "Failed to init steam api!");
			SteamNetworkingUtils()->InitRelayNetworkAccess();


			//overlay instead of layer so it is on top
			auto scene = CreateRef<SceneLayer>(false);//Set True for Debug!
			PushOverlay(scene);

			//create game nodes
			Instantiate<MenuNode>();
			Instantiate<SteamManagerNode>();

		}
		~PyxisGame()
		{
			SteamAPI_Shutdown();
		}
	};

	Pyxis::Application* CreateApplication() {
		return new PyxisGame();
	}
}
