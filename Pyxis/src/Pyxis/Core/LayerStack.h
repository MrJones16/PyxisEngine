#pragma once

#include "Pyxis/Core/Core.h"
#include "Layer.h"


namespace Pyxis
{
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Ref<Layer> layer);
		void PushOverlay(Ref<Layer> overlay);
		void PopLayer(Ref<Layer> layer);
		void PopOverlay(Ref<Layer> overlay);

		std::vector<Ref<Layer>>::iterator begin() { return m_Layers.begin(); }
		std::vector<Ref<Layer>>::iterator end() { return m_Layers.end(); }

		std::vector<Ref<Layer>>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		std::vector<Ref<Layer>>::reverse_iterator rend() { return m_Layers.rend(); }

	private:
		std::vector<Ref<Layer>> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};
}
