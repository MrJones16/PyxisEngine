#pragma once

#include "Core.h"

namespace Pyxis
{

	class PYXIS_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
	};

	//define in CLIENT
	Application* CreateApplication();
}


