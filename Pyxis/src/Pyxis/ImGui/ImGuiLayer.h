#pragma once

#include "Pyxis/Layer.h"

namespace Pyxis
{
	class PYXIS_API ImGuiLayer : public Layer
	{

	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach();
		void OnDetach();
		void OnUpdate();
		void OnEvent(Event& event);
	private:
		static void SetClipboardText(void* user_data, const char* text);
		static const char* GetClipboardText(void* user_data);

		bool OnMouseButtonPressedEvent  (MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent (MouseButtonReleasedEvent& e);
		bool OnMouseMovedEvent          (MouseMovedEvent& e);
		bool OnMouseScrolledEvent       (MouseScrolledEvent& e);
		bool OnKeyPressedEvent          (KeyPressedEvent& e);
		bool OnKeyReleasedEvent         (KeyReleasedEvent& e);
		bool OnKeyTypedEvent            (KeyTypedEvent& e);
		//bool OnWindowCloseEvent         (WindowCloseEvent& e);
		bool OnWindowResizeEvent        (WindowResizeEvent& e);
		//bool OnWindowFocusEvent         (WindowFocusEvent& e);
		//bool OnWindowFocusEvent         (WindowFocusEvent& e);
		//bool OnWindowMoveEvent          (WindowMoveEvent& e);
		//bool OnAppTickEvent             (AppTickEvent& e);
		//bool OnAppUpdateEvent           (AppUpdateEvent& e);
		//bool OnAppRenderEvent           (AppRenderEvent& e);

		float m_Time = 0.0f;
	};
}


