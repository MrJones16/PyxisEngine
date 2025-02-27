#pragma once

#include <Pyxis.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;



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

		if (r + g + b == 0) return 0x00000000;

		r = std::max(std::min(255, r + random), 0);
		g = std::max(std::min(255, g + random), 0);
		b = std::max(std::min(255, b + random), 0);

		return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
	}

	static uint32_t RGBAtoABGR(uint32_t RGBA)
	{
		int r = (RGBA & 0xFF000000) >> 24;
		int g = (RGBA & 0x00FF0000) >> 16;
		int b = (RGBA & 0x0000FF00) >> 8;
		int a = (RGBA & 0x000000FF) >> 0;

		//a = std::max(std::min(255, a + random), 0);

		return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | ((uint32_t)r << 0);
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
		uint32_t m_BaseColor = 0x00000000;

		//Currently used for things like glowing from heat.
		uint32_t m_Color = 0x00000000;

		bool m_Ignited = false;
		int m_Health = 100;

		float m_Temperature = 20;

		int8_t m_Horizontal = 0;

		bool m_Sliding = false;

		//Rigid means it belongs to a rigid body
		bool m_Rigid = false;
		//uint32_t m_LifeTime = 0;
		bool m_Updated = false;

		//Serialization of element
		friend void to_json(json& j, const Element& e)
		{
			j = json{ 
				{"ID", e.m_ID}, 
				{"Color", e.m_Color}, 
				{"BaseColor", e.m_BaseColor}, 
				{"Ignited", e.m_Ignited}, 
				{"Health", e.m_Health}, 
				{"Temperature", e.m_Temperature}, 
				{"Horizontal", e.m_Horizontal}, 
				{"Sliding", e.m_Sliding}, 
				{"Rigid", e.m_Rigid}, 
				{"Updated", e.m_Updated} 
			};
		}
		friend void from_json(const json& j, Element& e)
		{
			j.at("ID").get_to(e.m_ID);
			j.at("Color").get_to(e.m_Color);
			j.at("BaseColor").get_to(e.m_BaseColor);
			j.at("Ignited").get_to(e.m_Ignited);
			j.at("Health").get_to(e.m_Health);
			j.at("Temperature").get_to(e.m_Temperature);
			j.at("Horizontal").get_to(e.m_Horizontal);
			j.at("Sliding").get_to(e.m_Sliding);
			j.at("Rigid").get_to(e.m_Rigid);
			j.at("Updated").get_to(e.m_Updated);
		}
	};

	/// <summary>
	/// for tags, write the name of the tag in []'s like [meltable], and code will handle the rest
	/// </summary>
	struct Reaction
	{
	public:
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

		int probablility = 100;

		std::string input_cell_0 = "";
		std::string input_cell_1 = "";

		std::string output_cell_0 = "";
		std::string output_cell_1 = "";

		friend void to_json(json& j, const Reaction& r)
		{
			j = json{
				{"Type", "Reaction"},
				{"probability", r.probablility},
				{"input_cell_0", r.input_cell_0},
				{"input_cell_1", r.input_cell_1},
				{"output_cell_0", r.output_cell_0},
				{"output_cell_1", r.output_cell_1}
			};
		}
		friend void from_json(const json& j, Reaction& r)
		{
			j.at("probability").get_to(r.probablility);
			j.at("input_cell_0").get_to(r.input_cell_0);
			j.at("input_cell_1").get_to(r.input_cell_1);
			j.at("output_cell_0").get_to(r.output_cell_0);
			j.at("output_cell_1").get_to(r.output_cell_1);
		}

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

	/// <summary>
	/// all the properties of an element
	/// </summary>
	class ElementData
	{
	public:

		inline static std::vector<ElementData> s_ElementData;


		inline static std::vector<Reaction> s_Reactions;
		inline static std::unordered_map<uint32_t, std::unordered_map<uint32_t, ReactionResult>> s_ReactionTable;
		inline static std::unordered_map<std::string, std::vector<uint32_t>> s_TagElements;

		static ElementData& GetElementData(uint32_t id);
		inline static std::map<std::string, uint32_t> s_ElementNameToID;
		static ElementData& GetElementData(const std::string& name);
		
		static void LoadElementData(const std::string& path = "assets/data/CellData.json");

		static Element GetElement(uint32_t id, int x = 0, int y = 0);
		static Element GetElement(const std::string& name, int x = 0, int y = 0);
		

		//Building the reaction table so an element 
		static void BuildReactionTable();
		static bool StringContainsTag(const std::string& string);
		static std::string TagFromString(const std::string& stringWithTag);
		static std::string ReplaceTagInString(const std::string& stringToFill, const std::string& name);

		//Behavior and properties
		std::string name = "default";

		std::string texture_path = "";
		int width = 0;
		int height = 0;
		uint8_t* texture_data = nullptr;

		ElementType cell_type = ElementType::gas;

		std::vector<std::string> tags;

		//solid, movablesolid, liquid, gas, fire
		uint32_t color = 0xFFFF00FF;
		uint32_t density = 5;
		//0 - 100, 0 would slide forever, 100 won't slide at all
		uint32_t friction = 60;//chance to stop when sliding

		int health = 100;

		//flammable settings
		bool flammable = false;
		bool ignited = false;
		bool spread_ignition = false; // will ignore temp and ignite surrounding elements
		uint32_t spread_ignition_chance = 10;
		uint32_t ignited_color = 0;
		float ignition_temperature = 371.0f;
		float fire_temperature = 1000;
		uint32_t fire_color = RGBAtoABGR(0xf7e334FF);
		float fire_temperature_increase = 10;
		std::string burnt = "air";


		//Conductivity / Glowing
		bool glow = false;
		uint32_t conductivity = 0;
		float temperature = 20;


		//name of element to become if melted
		std::string melted = "";
		int melting_point = 100;

		//name of element to become if frozen
		std::string frozen = "";
		int freezing_point = 0;

		void SetTexture(std::string tex_path);

		//Get the element data as an element
		Element GetElement(int x = 0, int y = 0) const;
		

		//Turns a given element into the element described by this data
		//does not update the element's ID.
		void UpdateElementData(Element& element, int x, int y) const;
		void UpdateElementData(Element* element, int x, int y) const;
		
		
		friend void to_json(json& j, const ElementData& e);
		friend void from_json(const json& j, ElementData& e);
		

	};


}
