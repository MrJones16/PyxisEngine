#include <Pyxis.h>

//---------- Entry Point ----------//
#include <Pyxis/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Pyxis
{
	class PyxisEditor : public Pyxis::Application {
	public:
		PyxisEditor()
			: Application("Pyxis Editor")
		{
			PushLayer(new EditorLayer());
		}
		~PyxisEditor()
		{

		}
	};

	Pyxis::Application* Pyxis::CreateApplication() {
		return new PyxisEditor();
	}
}
