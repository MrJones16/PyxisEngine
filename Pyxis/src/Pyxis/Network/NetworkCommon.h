#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif


#define ASIO_STANDALONE

//#include <asio.hpp>
//asio / asio / include 

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/basic_socket_iostream.hpp>


const uint64_t UDP_HANDSHAKE = -1;

namespace Pyxis
{
	namespace Network
	{
		enum class CustomMessageTypes : uint32_t
		{
			ServerAccept,
			ServerDeny,
			ServerPing,
			MessageAll,
			ServerMessage,
		};
	}
}
