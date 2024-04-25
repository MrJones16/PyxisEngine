#include "pxpch.h"
#include "Core.h"
#include "Layer.h"
#include "Pyxis/Events/Event.h"

namespace Pyxis
{

	Layer::Layer(const std::string& debugName) : m_DebugName(debugName)
	{

	}

	Layer::~Layer()
	{

	}

	void Layer::OnEvent(Event& e)
	{
		//EventDispatcher dispatcher(event);

		//dispatcher.Dispatch<MouseButtonPressedEvent>  (BIND_EVENT_FN(Layer::OnMouseButtonPressedEvent));
		//dispatcher.Dispatch<MouseButtonReleasedEvent> (BIND_EVENT_FN(Layer::OnMouseButtonReleasedEvent));
		//dispatcher.Dispatch<MouseMovedEvent>          (BIND_EVENT_FN(Layer::OnMouseMovedEvent));
		//dispatcher.Dispatch<MouseScrolledEvent>       (BIND_EVENT_FN(Layer::OnMouseScrolledEvent));
		//dispatcher.Dispatch<KeyPressedEvent>          (BIND_EVENT_FN(Layer::OnKeyPressedEvent));
		//dispatcher.Dispatch<KeyReleasedEvent>         (BIND_EVENT_FN(Layer::OnKeyReleasedEvent));
		//dispatcher.Dispatch<KeyTypedEvent>            (BIND_EVENT_FN(Layer::OnKeyTypedEvent));
		//dispatcher.Dispatch<WindowCloseEvent>         (BIND_EVENT_FN(Layer::OnWindowCloseEvent));
		//dispatcher.Dispatch<WindowResizeEvent>        (BIND_EVENT_FN(Layer::OnWindowResizeEvent));
		//dispatcher.Dispatch<WindowFocusEvent>         (BIND_EVENT_FN(Layer::OnWindowFocusEvent));
		//dispatcher.Dispatch<WindowMoveEvent>          (BIND_EVENT_FN(Layer::OnWindowMoveEvent));
		//dispatcher.Dispatch<AppTickEvent>             (BIND_EVENT_FN(Layer::OnAppTickEvent));
		//dispatcher.Dispatch<AppUpdateEvent>           (BIND_EVENT_FN(Layer::OnAppUpdateEvent));
		//dispatcher.Dispatch<AppRenderEvent>           (BIND_EVENT_FN(Layer::OnAppRenderEvent));

	}

}
