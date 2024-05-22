#pragma once
#include "World.h"

namespace Pyxis
{
	class ChunkWorker
	{
	public:
		//treat this class as a callable object, will use for threading
		void operator()(World& world, Chunk* chunk);
	};
}