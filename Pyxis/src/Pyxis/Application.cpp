#include "pxpch.h"
#include "Application.h"

#include "Events/ApplicationEvent.h"
#include "Pyxis/Log.h"

namespace Pyxis 
{
	Application::Application() {

	}

	Application::~Application() {
		
	};

	void Application::Run() {
		WindowResizeEvent e(1280,720);
		if (e.IsInCategory(EventCategoryApplication))
		{
			PX_TRACE(e);
		}
		if (e.IsInCategory(EventCategoryInput))
		{
			PX_TRACE(e);
		}

		while (true);
	}
}