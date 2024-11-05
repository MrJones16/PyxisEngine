#pragma once

#include <imgui.h>

namespace Pyxis
{
	class Panel
	{
	public:
		Panel() {};
		virtual ~Panel() = default;
		virtual void OnImGuiRender() {};
	};
}