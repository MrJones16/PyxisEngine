#pragma once

#include "Pyxis/Core/Core.h"
#include "glm/glm.hpp"

namespace Pyxis
{
	class PYXIS_API Input
	{
	public:
		static bool IsKeyPressed(int keycode);

		static bool IsMouseButtonPressed(int button);
		static glm::ivec2 GetMousePosition();
		static int GetMouseX();
		static int GetMouseY();
	};

}