#pragma once

#include <Pyxis/Network/Network.h>

namespace Pyxis
{
	enum class GameMessage : uint32_t
	{
		Ping,

		Server_ClientConnected,
		Server_ClientDisconnected,
		Server_ClientAccepted,
		Server_ClientAssignID,
		Server_ClientDesynced,

		Client_RegisterWithServer,
		Client_UnregisterWithServer,
		Client_RequestMergedTick,

		Game_RequestGameData,
		Game_GameData,
		Game_ResetBox2D,
		Game_Loaded,
		Game_TickToEnter,
		Game_TickClosure,
		Game_MergedTickClosure,

		Message_All,
		Server_Message,
	};

	class PixelClientInterface : public Network::ClientInterface<GameMessage>
	{
	public:
	};
}