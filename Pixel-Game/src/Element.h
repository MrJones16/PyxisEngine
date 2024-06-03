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
		uint32_t m_BaseColor = 0xFF000000;
		uint32_t m_Color = 0xFF000000;

		int32_t m_Temperature = 20;

		int8_t m_Horizontal = 0;

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

		//0 - 2^32-1
		uint32_t density = 1;

		//0 - 100, 0 would slide forever, 100 won't slide at all
		uint32_t friction = 5;//chance to stop when sliding

		int32_t temperature = 20;
		bool glow = false;

		//name of element to become if melted
		std::string melted = "";
		int melting_point = 100;

		//name of element to become if frozen
		std::string frozen = "";
		int freezing_point = 100;

		
		void UpdateElementData(Element& element)
		{
			element.m_BaseColor = color;
			element.m_Color = color;
			element.m_Temperature = temperature;
		}

	};

	/// <summary>
	/// for tags, write the name of the tag in []'s like [meltable], and code will handle the rest
	/// </summary>
	struct Reaction
	{
		Reaction()
		{
			probablility = 100;

			input_cell_0 = "";
			input_cell_1 = "";

			output_cell_0 = "";
			output_cell_1 = "";
		}
		Reaction(int prob, std::string in0, std::string in1, std::string out0, std::string out1)
		{
			probablility = prob;
			input_cell_0 = in0;
			input_cell_1 = in1;
			
			output_cell_0 = out0;
			output_cell_1 = out1;
		}

		int probablility;

		std::string input_cell_0;
		std::string input_cell_1;

		std::string output_cell_0;
		std::string output_cell_1;

	};

	struct ReactionResult
	{
		ReactionResult() = default;
		ReactionResult(uint32_t percentChance, uint32_t cell0, uint32_t cell1) {
			probability = percentChance;
			cell0ID = cell0;
			cell1ID = cell1;
		}
		uint32_t probability = 100;
		uint32_t cell0ID = 0;
		uint32_t cell1ID = 0;
	};

}
