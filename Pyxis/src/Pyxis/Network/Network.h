#pragma once

#include <steam/steamnetworkingsockets.h>
//#include "NetworkMessage.h"

//#include "NetworkThreadSafeQueue.h"

/// <summary>
/// Overall guide on Pyxis Networking:
/// 
/// Network_Init is called on the creation of an application, and an application
/// won't run if it fails.
/// 
/// NetworkClient.h holds a ClientInterface, an object which can be used to connect to a server,
/// read messages, and send messages.
/// 
/// NetworkServer.h holds the ServerInterface, and object that lets you host a server, and virtual functions
/// to be overriden for logic on connections, messages, ect.
/// </summary>

namespace Pyxis
{
	namespace Network
	{
		static bool Network_Initialized = false;
		static bool Network_Init()
		{
			if (!Network_Initialized)
			{
				SteamDatagramErrMsg errMsg;
				if (!GameNetworkingSockets_Init(nullptr, errMsg))
				{
					PX_CORE_ERROR("SteamServer::Start->GameNetworkingSockets_Init failed.  {0}", errMsg);
					return false;
				}
				Network_Initialized = true;
				return true;
			}
			return true;
			
		}

	}
}