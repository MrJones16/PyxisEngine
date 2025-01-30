#pragma once

#include <Pyxis/Nodes/Node.h>
#include <steam/steam_api.h>

namespace Pyxis
{

	class SteamManagerNode : public Node
	{
	public:
		SteamManagerNode() : Node("Steam Manager") 
		{

		}
		virtual void OnUpdate(Timestep ts)
		{
			SteamAPI_RunCallbacks();
		}
	};
	
}
