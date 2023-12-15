#pragma once

#include "Pyxis/Core.h"

#include "Pyxis/Events/KeyEvent.h"
#include "Pyxis/Events/MouseEvent.h"
#include "Pyxis/Events/ApplicationEvent.h"

namespace Pyxis
{


	class PYXIS_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event);

		inline const std::string& GetName() const { return m_DebugName; }
	private:
		/*
			WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
			AppTick, AppUpdate, AppRender,
			KeyPressed, KeyReleased,
			MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
		*/

		//Copy what you need in your class. 

		//virtual bool OnMouseButtonPressedEvent  (MouseButtonPressedEvent& e);
		//virtual bool OnMouseButtonReleasedEvent (MouseButtonReleasedEvent& e);
		//virtual bool OnMouseMovedEvent          (MouseMovedEvent& e);
		//virtual bool OnMouseScrolledEvent       (MouseScrolledEvent& e);
		//virtual bool OnKeyPressedEvent          (KeyPressedEvent& e);
		//virtual bool OnKeyReleasedEvent         (KeyReleasedEvent& e);
		//virtual bool OnKeyTypedEvent            (KeyTypedEvent& e);
		//virtual bool OnWindowCloseEvent         (WindowCloseEvent& e);
		//virtual bool OnWindowResizeEvent        (WindowResizeEvent& e);
		//virtual bool OnWindowFocusEvent         (WindowFocusEvent& e);
		//virtual bool OnWindowFocusEvent         (WindowFocusEvent& e);
		//virtual bool OnWindowMoveEvent          (WindowMoveEvent& e);
		//virtual bool OnAppTickEvent             (AppTickEvent& e);
		//virtual bool OnAppUpdateEvent           (AppUpdateEvent& e);
		//virtual bool OnAppRenderEvent           (AppRenderEvent& e);

	protected:
		std::string m_DebugName;
	};
}

