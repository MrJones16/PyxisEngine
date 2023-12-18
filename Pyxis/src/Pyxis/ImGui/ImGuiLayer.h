#pragma once

#include "Pyxis/Layer.h"

namespace Pyxis
{
	class PYXIS_API ImGuiLayer : public Layer
	{

	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

		float m_Time = 0.0f;
	};
}


