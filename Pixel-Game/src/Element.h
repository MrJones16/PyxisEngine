#pragma once

#include <Pyxis.h>

namespace Pyxis
{
	enum class ElementType {
		solid, liquid, gas, fire
	};

	enum class ElementSubType {
		None, Static
	};


	struct Element
	{
	public:
		Element() = default;

		// id can correlate to a table for elements, to use for things like name of the element
		// or other things that are not necessary to store in the element itself.
		uint32_t m_ID = 0; 
		//std::string m_Name = "name";


		
		uint32_t m_Color = 0xFF000000;

		//Behavior and properties
		ElementType m_Type = ElementType::gas;
		ElementSubType m_SubType = ElementSubType::None;

		/// <summary>
		/// bitfield of properties such as flammable, 
		/// </summary>
		uint64_t m_Tags = 0;

		int8_t m_Vertical = 0;
		int8_t m_Horizontal = 0;

		bool m_Updated = false;

	};

}
