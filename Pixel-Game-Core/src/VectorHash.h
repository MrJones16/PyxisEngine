#pragma once

#include <Pyxis.h>

namespace Pyxis
{


	//A class used in the third type for std::unordered_map
	class HashVector
	{
	public:
		size_t operator()(const glm::ivec2& vector) const
		{
			size_t seed = 0;
			seed = vector.x + 0xae3779b9 + (seed << 6) + (seed >> 2);
			seed ^= vector.y + 0x87d81ab8 + (seed << 7) + (seed >> 3);
			return seed;
		}

		bool operator()(const glm::ivec2& a, const glm::ivec2& b) const
		{
			return a.x == b.x && a.y == b.y;
		}

	};

	// Comparison functor for glm::ivec2, for std::map
	struct IVec2Compare {
		bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
			// First compare x, then y if x values are equal
			if (a.x != b.x) {
				return a.x < b.x;
			}
			return a.y < b.y;
		}
	};

	
}