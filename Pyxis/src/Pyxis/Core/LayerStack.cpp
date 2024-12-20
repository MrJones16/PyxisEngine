#include "pxpch.h"
#include "LayerStack.h"


namespace Pyxis
{
	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		/*for (Layer* layer : m_Layers) {
			delete layer;
		}*/
	}

	void LayerStack::PushLayer(Ref<Layer> layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(Ref<Layer> overlay)
	{
		m_Layers.emplace_back(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopLayer(Ref<Layer> layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end()) {
			m_Layers.erase(it);
			m_LayerInsertIndex--;
			layer->OnDetach();
		}
	}

	void LayerStack::PopOverlay(Ref<Layer> overlay)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (it != m_Layers.end()) {
			m_Layers.erase(it);
			overlay->OnDetach();

		}
	}

}