#include "pxpch.h"

#include "NetworkCommon.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif


#define ASIO_STANDALONE

//#include <asio.hpp>
//asio / asio / include 

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

