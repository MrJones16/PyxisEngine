#pragma once

#include "Log.h"
#include "glm/glm.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat3x3.hpp"
#include "glm/mat3x4.hpp"
#include "glm/mat4x3.hpp"
#include "glm/mat4x4.hpp"

namespace Pyxis
{
	inline std::ostream& operator<<(std::ostream& os, glm::vec2 vec2)
	{
		return os << "<" << vec2.x << ", " << vec2.y << ">";
	}

	inline std::ostream& operator<<(std::ostream& os, glm::vec3 vec3)
	{
		return os << "<" << vec3.x << ", " << vec3.y << ">";
	}
	
	inline std::ostream& operator<<(std::ostream& os, glm::vec4 vec4)
	{
		return os << "<" << vec4.x << ", " << vec4.y << ">";
	}
}

