#pragma once

#include <Pyxis/Events/ApplicationEvent.h>
#include <Pyxis/Events/KeyEvent.h>
#include <Pyxis/Events/MouseEvent.h>
#include <Pyxis/Events/Signal.h>

namespace Pyxis
{

	class EventSignal
	{
	public:
		//application signals
		inline static Signal<void(WindowResizeEvent&)>			s_WindowResizeEventSignal;
		inline static Signal<void(WindowFocusEvent&)>			s_WindowFocusEventSignal;
		inline static Signal<void(WindowMoveEvent&)>			s_WindowMoveEventSignal;

		//key signals											
		inline static Signal<void(KeyPressedEvent&)>			s_KeyPressedEventSignal;
		inline static Signal<void(KeyReleasedEvent&)>			s_KeyReleasedEventSignal;
		inline static Signal<void(KeyTypedEvent&)>				s_KeyTypedEventSignal;

		//mouse signals											
		inline static Signal<void(MouseMovedEvent&)>			s_MouseMovedEventSignal;
		inline static Signal<void(MouseButtonPressedEvent&)>	s_MouseButtonPressedEventSignal;
		inline static Signal<void(MouseButtonReleasedEvent&)>	s_MouseButtonReleasedEventSignal;
		inline static Signal<void(MouseScrolledEvent&)>			s_MouseScrolledEventSignal;

	};

}
