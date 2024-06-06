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
	/// helper to mix colors
	/// </summary>
	/// <param name="ABGR"></param>
	/// <param name="randomShift"></param>
	/// <returns></returns>
	static uint32_t RandomizeABGRColor(uint32_t ABGR, int randomShift = 20)
	{
		int random = ((std::rand() % (randomShift * 2)) - randomShift); //-20 to 20
		int r = (ABGR & 0x000000FF) >> 0;
		int g = (ABGR & 0x0000FF00) >> 8;
		int b = (ABGR & 0x00FF0000) >> 16;
		int a = (ABGR & 0xFF000000) >> 24;

		if (r + g + b == 0) return 0xFF000000;

		r = std::max(std::min(255, r + random), 0);
		g = std::max(std::min(255, g + random), 0);
		b = std::max(std::min(255, b + random), 0);

		return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
	}

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

		bool m_Ignited = false;
		int m_Health = 100;

		float m_Temperature = 20;

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

		int health = 100;

		//flammable settings
		bool flammable = false;
		bool ignited = false;
		float ignition_temperature = 371.0f;
		float fire_temperature = 1000;
		std::string burnt = "air";



		bool glow = false;
		uint32_t conductivity = 0;
		float temperature = 20;


		//name of element to become if melted
		std::string melted = "";
		int melting_point = 100;

		//name of element to become if frozen
		std::string frozen = "";
		int freezing_point = 100;

		
		void UpdateElementData(Element& element)
		{
			element.m_BaseColor = RandomizeABGRColor(color);
			element.m_Color = element.m_BaseColor;
			element.m_Temperature = temperature;
			element.m_Ignited = ignited;
			element.m_Health = health;
		}
		void UpdateElementData(Element* element)
		{
			element->m_BaseColor = RandomizeABGRColor(color);
			element->m_Color = element->m_BaseColor;
			element->m_Temperature = temperature;
			element->m_Ignited = ignited;
			element->m_Health = health;
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
