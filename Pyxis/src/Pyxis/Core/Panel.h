#pragma once

#include <ImGui/imgui.h>

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