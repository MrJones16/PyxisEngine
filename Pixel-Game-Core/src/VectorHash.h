#pragma once

#include <Pyxis.h>

namespace Pyxis
{
	class HashVector
	{
	public:
		size_t operator()(glm::ivec2 vector) const
		{
			size_t seed = 0;
			seed = vector.x + 0xae3779b9 + (seed << 6) + (seed >> 2);
			seed ^= vector.y + 0x87d81ab8 + (seed << 7) + (seed >> 3);
			return seed;
		}
	};
}