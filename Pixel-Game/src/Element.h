#pragma once

#include <Pyxis.h>

namespace Pyxis
{
	enum class ElementType {
		solid, movableSolid, liquid, gas, fire
	};

	enum class ElementSubType {
		None, Static
	};

	enum class ElementProperties
	{
		Flammable = 1, Meltable = 2, Corrosive = 4
	};



	/// <summary>
	/// Actual element data to be stored in the map, only data that differs on an instance of an element
	/// </summary>
	struct Element
	{
	public:
		Element() = default;
		Element(uint32_t id, uint32_t color, bool updated = false) {
			m_ID = id;
			m_Color = color;
			m_Updated = updated;
		}
		uint32_t m_ID = 0;
		uint32_t m_Color = 0xFF000000;

		int8_t horizontal = 0;

		bool m_Sliding = false;
		//uint32_t m_LifeTime = 0;
		bool m_Updated = false;
	};

	/// <summary>
	/// all the properties of an element
	/// </summary>
	struct ElementData
	{
		//Behavior and properties
		std::string name = "default";

		ElementType cell_type = ElementType::gas;
		//solid, movablesolid, liquid, gas, fire
		uint32_t color = 0xFFFF00FF;
		uint32_t density = 1;

		//0 - 10, 0 would slide forever, 10 won't slide at all
		uint32_t friction = 5;//chance to stop when sliding
		
		void UpdateElementData(Element& element)
		{
			element.m_Color = color;
		}

	};

	struct ReactionResult
	{
		ReactionResult() = default;
		ReactionResult(uint32_t cell0, uint32_t cell1) {
			cell0ID = cell0;
			cell1ID = cell1;
		}
		uint32_t cell0ID = 0;
		uint32_t cell1ID = 0;
	};

}
