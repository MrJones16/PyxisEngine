#pragma once

#include "Pyxis/Core/Core.h"
#include "Pyxis/Core/Panel.h"
#include <glm/glm/glm.hpp>

namespace Pyxis
{
	class ExamplePanel : public Panel
	{
	public:
		inline ExamplePanel()
		{
			
		}
		~ExamplePanel() = default;

		inline virtual void OnImGuiRender() override
		{
			if (ImGui::Begin("ExamplePanel"))
			{
				
			}
			ImGui::End();
		}

		
	private:
		
		
	};
}