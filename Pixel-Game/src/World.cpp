#include "World.h"
#include "ChunkWorker.h"
#include <random>
#include <thread>
#include <mutex>

#include <tinyxml2/tinyxml2.h>

namespace Pyxis
{
	World::World()
	{

		if (!LoadElementData())
		{
			PX_ASSERT(false, "Failed to load element data, shutting down.");
			m_Error = true;
			Application::Get().Close();
			return;
		}

		BuildReactionTable();
	}

	bool World::LoadElementData()
	{
		//loading element data from xml
		tinyxml2::XMLDocument doc;
		doc.LoadFile("assets/Data/CellData.xml");
		if (doc.ErrorID())
		{
			PX_ERROR("Error in loading xml data");
			return false;
		}
		auto materials = doc.FirstChild();
		auto data = materials->FirstChildElement();
		//loop over each element or reaction, and add them
		while (data != nullptr)
		{
			std::string nodeName = data->Value();
			//node is an element, so create a new element data
			if (nodeName == "ElementData")
			{
				//initialize the element data to be modified
				ElementData elementData = ElementData();

				//get the name
				const char* name;
				auto error = data->QueryAttribute("name", &name);
				if (!error) {
					elementData.name = name;
				}
				else {
					data = data->NextSiblingElement();
					PX_ERROR("No name given to element, skipping");
					continue;
				}
				
				//gather each attribuite for the element and set the values
				const char* cell_type;
				error = data->QueryAttribute("cell_type", &cell_type);
				if (!error) {
					std::string cell_type_str = cell_type;
					//auto cell_type = attribute->Value();
					if (cell_type_str == "solid") {
						elementData.cell_type = ElementType::solid;
					}
					else if (cell_type_str == "movableSolid") {
						elementData.cell_type = ElementType::movableSolid;
					}
					else if (cell_type_str == "liquid") {
						elementData.cell_type = ElementType::liquid;
					}
					else if (cell_type_str == "gas") {
						elementData.cell_type = ElementType::gas;
					}
					else if (cell_type_str == "fire") {
						elementData.cell_type = ElementType::fire;
					}
				}
				else {
					PX_ERROR("No type given to element {0}, defaulting to gas", elementData.name);
				}

				const char* colorStr;
				error = data->QueryAttribute("color", &colorStr);
				uint32_t color = std::stoul(colorStr, nullptr, 16);
				if (!error) {
					elementData.color = RGBAtoABGR(color);
				}
				else
				{
					PX_INFO("Element {0} was given no color", elementData.name);
				}

				int health = 100;
				error = data->QueryIntAttribute("health", &health);
				if (!error) {
					elementData.health = health;
				}
				else
				{
					PX_INFO("Element {0} was given no health attribute", elementData.name);
				}

				uint32_t density;
				error = data->QueryUnsignedAttribute("density", &density);
				if (!error) {
					elementData.density = density;
				}
				else
				{
					PX_INFO("Element {0} was given no density", elementData.name);
				}

				uint32_t friction;
				error = data->QueryUnsignedAttribute("friction", &friction);
				if (!error) {
					elementData.friction = friction;
				}
				else
				{
					PX_INFO("Element {0} was given no friction", elementData.name);
				}

				///fire settings
				bool flammable = false;
				error = data->QueryBoolAttribute("flammable", &flammable);
				if (!error) {
					elementData.flammable = flammable;
				}
				else
				{
					PX_INFO("Element {0} was given no flammable attribute", elementData.name);
				}

				bool ignited = false;
				error = data->QueryBoolAttribute("ignited", &ignited);
				if (!error) {
					elementData.ignited = ignited;
				}
				else
				{
					PX_INFO("Element {0} was given no ignited attribute", elementData.name);
				}

				float ignition_temperature = 371;
				error = data->QueryFloatAttribute("ignition_temperature", &ignition_temperature);
				if (!error) {
					elementData.ignition_temperature = ignition_temperature;
				}
				else
				{
					PX_INFO("Element {0} was given no ignition_temperature", elementData.name);
				}

				float fire_temperature = 1000;
				error = data->QueryFloatAttribute("fire_temperature", &fire_temperature);
				if (!error) {
					elementData.fire_temperature = fire_temperature;
				}
				else
				{
					PX_INFO("Element {0} was given no ignition_temperature", elementData.name);
				}

				const char* burnt;
				error = data->QueryAttribute("burnt", &burnt);
				if (!error) {
					elementData.burnt = burnt;
				}
				else
				{
					//PX_INFO("Element {0} was given no _____", elementData.name);
				}

				bool glow = false;
				error = data->QueryBoolAttribute("glow", &glow);
				if (!error) {
					elementData.glow = glow;
				}
				else
				{
					PX_INFO("Element {0} was given no glow", elementData.name);
				}

				uint32_t conductivity = 0;
				error = data->QueryUnsignedAttribute("conductivity", &conductivity);
				if (!error) {
					elementData.conductivity = conductivity;
				}
				else
				{
					PX_INFO("Element {0} was given no conductivity", elementData.name);
				}

				const char* meltedStr;
				error = data->QueryAttribute("melted", &meltedStr);
				if (!error) {
					elementData.melted = meltedStr;
				}
				else
				{
					//PX_INFO("Element {0} was given no _____", elementData.name);
				}
				
				const char* frozenStr;
				error = data->QueryAttribute("frozen", &frozenStr);
				if (!error) {
					elementData.frozen = frozenStr;
				}
				else
				{
					//PX_INFO("Element {0} was given no _____", elementData.name);
				}

				float temperature = 20;
				error = data->QueryFloatAttribute("temperature", &temperature);
				if (!error) {
					elementData.temperature = temperature;
				}
				else
				{
					PX_INFO("Element {0} was given no temperature", elementData.name);
				}

				int meltingPoint = 100;
				error = data->QueryIntAttribute("melting_point", &meltingPoint);
				if (!error) {
					elementData.melting_point = meltingPoint;
				}
				else
				{
					PX_INFO("Element {0} was given no melting point", elementData.name);
				}

				int freezingPoint = 0;
				error = data->QueryIntAttribute("freezing_point", &freezingPoint);
				if (!error) {
					elementData.freezing_point = freezingPoint;
				}
				else
				{
					PX_INFO("Element {0} was given no freezing point", elementData.name);
				}
				
				//end of attributes, read for children like tags and graphics
				auto tags = data->FirstChildElement("Tags");
				if (tags)
				{
					for (auto tagElement = tags->FirstChildElement(); tagElement != nullptr; tagElement = tagElement->NextSiblingElement())
					{
						std::string tag = tagElement->Value();
						PX_TRACE("Added element {0} to tag {1}", elementData.name, tag);
						m_TagElements[tag].push_back(m_TotalElements);
					}
				}

				//finished reading data, add to the memory!
				m_ElementData.push_back(elementData);
				m_ElementIDs[elementData.name] = m_TotalElements++;
			}
			//node is a reaction, so add to reaction input list for later
			else if (nodeName == "Reaction")
			{
				//create result reaction
				Reaction reaction = Reaction();

				uint32_t probability;
				auto error = data->QueryUnsignedAttribute("probability", &probability);
				if (!error) {
					reaction.probablility = probability; 
				}

				//input cell 0
				const char* input0str;
				error = data->QueryAttribute("input_cell_0", &input0str);
				if (!error) {
					reaction.input_cell_0 = input0str;
				}
				else
				{
					PX_INFO("Reaction missing input cell 0");
				}

				//input cell 1
				const char* input1str;
				error = data->QueryAttribute("input_cell_1", &input1str);
				if (!error) {
					reaction.input_cell_1 = input1str;
				}
				else
				{
					PX_INFO("Reaction missing input cell 1");
				}

				//output cell 0
				const char* output0str;
				error = data->QueryAttribute("output_cell_0", &output0str);
				if (!error) {
					reaction.output_cell_0 = output0str;
				}
				else
				{
					PX_INFO("Reaction missing output cell 0");
				}

				//output cell 1
				const char* output1str;
				error = data->QueryAttribute("output_cell_1", &output1str);
				if (!error) {
					reaction.output_cell_1 = output1str;
				}
				else
				{
					PX_INFO("Reaction missing output cell 1");
				}
				
				//end of gathering data, so add to reaction list
				m_Reactions.push_back(reaction);
			}
			PX_TRACE("node name was: {0}", nodeName);
			data = data->NextSiblingElement();
		}


		////5
		//ElementData dampSand = ElementData();
		//dampSand.name = "dampSand";
		//dampSand.cell_type = ElementType::movableSolid;
		//dampSand.density = 2;
		//dampSand.color = 0xFF11AAAA;
		//dampSand.friction = 25;
		//m_ElementData.push_back(dampSand);
		//m_ElementIDs[dampSand.name] = m_TotalElements++;

		////6
		//ElementData wetSand = ElementData();
		//wetSand.name = "wetSand";
		//wetSand.cell_type = ElementType::movableSolid;
		//wetSand.density = 3;
		//wetSand.color = 0xFF229999;
		//wetSand.friction = 40;
		//m_ElementData.push_back(wetSand);
		//m_ElementIDs[wetSand.name] = m_TotalElements++;

		//when loading from xml, make a map of tags to elements, so
		//the reaction table can use it
		return true;
	}

	void World::BuildReactionTable()
	{
		m_ReactionLookup = std::vector<std::unordered_map<uint32_t, ReactionResult>>(m_TotalElements);

		std::string input0Tag, input1Tag;
		//loop over each reaction
		for each (Reaction reaction in m_Reactions)
		{
			//get the first string, input 0
			if (StringContainsTag(reaction.input_cell_0))
			{
				//contains a tag, so keep track of what the tag is and loop
				input0Tag = TagFromString(reaction.input_cell_0);
				for each (uint32_t id0 in m_TagElements[input0Tag])
				{
					uint32_t idOut0;
					if (StringContainsTag(reaction.output_cell_0))
					{
						idOut0 = m_ElementIDs[ReplaceTagInString(reaction.output_cell_0, m_ElementData[id0].name)];
					}
					else
					{
						idOut0 = m_ElementIDs[reaction.output_cell_0];
					}
					//id0 obtained, move onto next input
					if (StringContainsTag(reaction.input_cell_1))
					{
						input1Tag = TagFromString(reaction.input_cell_1);
						//input 1 has tag, so loop over tag elements again
						for each (uint32_t id1 in m_TagElements[input1Tag])
						{
							uint32_t idOut1;
							//id0 and 1 obtained
							if (StringContainsTag(reaction.output_cell_1))
							{
								idOut1 = m_ElementIDs[ReplaceTagInString(reaction.output_cell_1, m_ElementData[id1].name)];
							}
							else
							{
								idOut1 = m_ElementIDs[reaction.output_cell_1];
							}
							//all id's obtained
							m_ReactionLookup[id0][id1] = ReactionResult(reaction.probablility, idOut0, idOut1);
						}
					}
					else
					{
						uint32_t id1 = m_ElementIDs[reaction.input_cell_1];
						uint32_t idOut1 = m_ElementIDs[reaction.output_cell_1];
						//all id's obtained
						m_ReactionLookup[id0][id1] = ReactionResult(reaction.probablility, idOut0, idOut1);
						/*m_ReactionLookup[id1][id0] = ReactionResult(reaction.probablility, idOut1, idOut0);*/
					}
				}
			}
			else
			{
				//input 0 has no tag, so move on to next input
				uint32_t id0 = m_ElementIDs[reaction.input_cell_0];
				uint32_t idOut0 = m_ElementIDs[reaction.output_cell_0];
				if (StringContainsTag(reaction.input_cell_1))
				{
					input1Tag = TagFromString(reaction.input_cell_1);
					//input 1 has tag, so loop over tag elements again
					for each (uint32_t id1 in m_TagElements[input1Tag])
					{
						//id0 and 1 obtained
						uint32_t idOut1;
						if (StringContainsTag(reaction.output_cell_1))
						{
							idOut1 = m_ElementIDs[ReplaceTagInString(reaction.output_cell_1, m_ElementData[id1].name)];
						}
						else
						{
							idOut1 = m_ElementIDs[reaction.output_cell_1];
						}
						//all id's obtained
						m_ReactionLookup[id0][id1] = ReactionResult(reaction.probablility, idOut0, idOut1);
					}
				}
				else
				{
					uint32_t id1 = m_ElementIDs[reaction.input_cell_1];
					uint32_t idOut1 = m_ElementIDs[reaction.output_cell_1];
					//all id's obtained
					m_ReactionLookup[id0][id1] = ReactionResult(reaction.probablility, idOut0, idOut1);
				}
			}
		}



		//for each (Reaction reaction in m_Reactions)
		//{
		//	//add reaction to the lookup for both cell directions
		//	m_ReactionLookup[m_ElementIDs[reaction.input_cell_0]][m_ElementIDs[reaction.input_cell_1]] =
		//		ReactionResult(reaction.probablility,
		//			m_ElementIDs[reaction.output_cell_0],
		//			m_ElementIDs[reaction.output_cell_1]);

		//	m_ReactionLookup[m_ElementIDs[reaction.input_cell_1]][m_ElementIDs[reaction.input_cell_0]] =
		//		ReactionResult(reaction.probablility,
		//			m_ElementIDs[reaction.output_cell_1],
		//			m_ElementIDs[reaction.output_cell_0]);

		//}
		
		////sand to water becomes damp sand and air
		//m_ReactionLookup[2][3] = ReactionResult(5, 0);

		////damp sand and water becomes wet sand and air
		//m_ReactionLookup[5][3] = ReactionResult(6, 0);

		////wet sand and sand becomes two damp sand
		//m_ReactionLookup[6][2] = ReactionResult(5, 5);

	}

	bool World::StringContainsTag(const std::string& string)
	{
		if (string.find("[") != std::string::npos && string.find("[") != std::string::npos) return true;
		return false;
	}

	std::string World::TagFromString(const std::string& stringWithTag)
	{
		int start = stringWithTag.find("[");
		int end = stringWithTag.find("]");
		return stringWithTag.substr(start + 1, (end - start) - 1);
	}

	std::string World::ReplaceTagInString(const std::string& stringToFill, const std::string& name)
	{
		int start = stringToFill.find("[");
		int end =   stringToFill.find("]");
		std::string result = stringToFill.substr(0, start) + name;
		if (end == stringToFill.size() - 1) return result;
		return result + stringToFill.substr(end + 1, (stringToFill.size() - end) - 1);
	}

	uint32_t World::RGBAtoABGR(uint32_t RGBA)
	{
		int r = (RGBA & 0xFF000000) >> 24;
		int g = (RGBA & 0x00FF0000) >> 16;
		int b = (RGBA & 0x0000FF00) >> 8;
		int a = (RGBA & 0x000000FF) >> 0;
		
		//a = std::max(std::min(255, a + random), 0);

		return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | ((uint32_t)r << 0);
	}

	uint32_t World::RandomizeABGRColor(uint32_t ABGR, int randomShift)
	{
		int random = ((std::rand() % (randomShift * 2)) - randomShift); //-20 to 20
		int r = (ABGR & 0x000000FF) >> 0;
		int g = (ABGR & 0x0000FF00) >> 8;
		int b = (ABGR & 0x00FF0000) >> 16;
		int a = (ABGR & 0xFF000000) >> 24;

		r = std::max(std::min(255, r + random), 0);
		g = std::max(std::min(255, g + random), 0);
		b = std::max(std::min(255, b + random), 0);
		
		return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
	}

	World::~World()
	{
		for each (auto& pair in m_Chunks)
		{
			delete(pair.second);
		}
		/*for each (Chunk * chunk in m_Chunks_TR)
		{
			delete(chunk);
		}
		for each (Chunk * chunk in m_Chunks_BL)
		{
			delete(chunk);
		}
		for each (Chunk * chunk in m_Chunks_BR)
		{
			delete(chunk);
		}*/
	}

	void World::AddChunk(const glm::ivec2& chunkPos)
	{
		//make sure chunk doesn't already exist
		if (m_Chunks.find(chunkPos) == m_Chunks.end())
		{
			Chunk* chunk = new Chunk(chunkPos);
			m_Chunks[chunkPos] = chunk;
		}
	}

	Chunk* World::GetChunk(const glm::ivec2& chunkPos)
	{
		auto it = m_Chunks.find(chunkPos);
		if (it != m_Chunks.end())
			return it->second;
		AddChunk(chunkPos);
		return m_Chunks[chunkPos];
	}


	void World::UpdateWorld()
	{

		/*m_Threads.clear();
		for each (auto& pair in m_Chunks)
		{
			m_Threads.push_back(std::thread(&World::UpdateChunk, pair.second));
		}
		for each (std::thread& thread in m_Threads)
		{
			thread.join();
		}*/

		for each (auto & pair in m_Chunks)
		{
			//BL
			UpdateChunkBucket(pair.second, 0, 0);
			UpdateChunkBucket(pair.second, 2, 0);
			UpdateChunkBucket(pair.second, 4, 0);
			UpdateChunkBucket(pair.second, 6, 0);
			UpdateChunkBucket(pair.second, 0, 2);
			UpdateChunkBucket(pair.second, 2, 2);
			UpdateChunkBucket(pair.second, 4, 2);
			UpdateChunkBucket(pair.second, 6, 2);
			UpdateChunkBucket(pair.second, 0, 4);
			UpdateChunkBucket(pair.second, 2, 4);
			UpdateChunkBucket(pair.second, 4, 4);
			UpdateChunkBucket(pair.second, 6, 4);
			UpdateChunkBucket(pair.second, 0, 6);
			UpdateChunkBucket(pair.second, 2, 6);
			UpdateChunkBucket(pair.second, 4, 6);
			UpdateChunkBucket(pair.second, 6, 6);
		}

		for each (auto & pair in m_Chunks)
		{
			//BR
			UpdateChunkBucket(pair.second, 0 + 1, 0);
			UpdateChunkBucket(pair.second, 2 + 1, 0);
			UpdateChunkBucket(pair.second, 4 + 1, 0);
			UpdateChunkBucket(pair.second, 6 + 1, 0);
			UpdateChunkBucket(pair.second, 0 + 1, 2);
			UpdateChunkBucket(pair.second, 2 + 1, 2);
			UpdateChunkBucket(pair.second, 4 + 1, 2);
			UpdateChunkBucket(pair.second, 6 + 1, 2);
			UpdateChunkBucket(pair.second, 0 + 1, 4);
			UpdateChunkBucket(pair.second, 2 + 1, 4);
			UpdateChunkBucket(pair.second, 4 + 1, 4);
			UpdateChunkBucket(pair.second, 6 + 1, 4);
			UpdateChunkBucket(pair.second, 0 + 1, 6);
			UpdateChunkBucket(pair.second, 2 + 1, 6);
			UpdateChunkBucket(pair.second, 4 + 1, 6);
			UpdateChunkBucket(pair.second, 6 + 1, 6);
		}

		for each (auto & pair in m_Chunks)
		{
			//TL
			UpdateChunkBucket(pair.second, 0, 0 + 1);
			UpdateChunkBucket(pair.second, 2, 0 + 1);
			UpdateChunkBucket(pair.second, 4, 0 + 1);
			UpdateChunkBucket(pair.second, 6, 0 + 1);
			UpdateChunkBucket(pair.second, 0, 2 + 1);
			UpdateChunkBucket(pair.second, 2, 2 + 1);
			UpdateChunkBucket(pair.second, 4, 2 + 1);
			UpdateChunkBucket(pair.second, 6, 2 + 1);
			UpdateChunkBucket(pair.second, 0, 4 + 1);
			UpdateChunkBucket(pair.second, 2, 4 + 1);
			UpdateChunkBucket(pair.second, 4, 4 + 1);
			UpdateChunkBucket(pair.second, 6, 4 + 1);
			UpdateChunkBucket(pair.second, 0, 6 + 1);
			UpdateChunkBucket(pair.second, 2, 6 + 1);
			UpdateChunkBucket(pair.second, 4, 6 + 1);
			UpdateChunkBucket(pair.second, 6, 6 + 1);
		}

		for each (auto & pair in m_Chunks)
		{
			//TR
			UpdateChunkBucket(pair.second, 0 + 1, 0 + 1);
			UpdateChunkBucket(pair.second, 2 + 1, 0 + 1);
			UpdateChunkBucket(pair.second, 4 + 1, 0 + 1);
			UpdateChunkBucket(pair.second, 6 + 1, 0 + 1);
			UpdateChunkBucket(pair.second, 0 + 1, 2 + 1);
			UpdateChunkBucket(pair.second, 2 + 1, 2 + 1);
			UpdateChunkBucket(pair.second, 4 + 1, 2 + 1);
			UpdateChunkBucket(pair.second, 6 + 1, 2 + 1);
			UpdateChunkBucket(pair.second, 0 + 1, 4 + 1);
			UpdateChunkBucket(pair.second, 2 + 1, 4 + 1);
			UpdateChunkBucket(pair.second, 4 + 1, 4 + 1);
			UpdateChunkBucket(pair.second, 6 + 1, 4 + 1);
			UpdateChunkBucket(pair.second, 0 + 1, 6 + 1);
			UpdateChunkBucket(pair.second, 2 + 1, 6 + 1);
			UpdateChunkBucket(pair.second, 4 + 1, 6 + 1);
			UpdateChunkBucket(pair.second, 6 + 1, 6 + 1);
		}


		//std::thread thread = std::thread(&World::UpdateChunk, this, m_Chunks[glm::ivec2(0,0)]);
		//thread.join();
		/*for (int x = 0; x < BUCKETSWIDTH; x++)
		{
			for (int y = 0; y < BUCKETSWIDTH; y++)
			{
				UpdateChunkBucket(m_Chunks[glm::ivec2(0, 0)], x, y);
			}
		}*/

		//m_Threads.clear();
		//for each (auto& pair in m_Chunks)
		//{
		//	Chunk* chunk = pair.second;
		//	m_Threads.push_back(std::thread(&World::UpdateChunk, this, chunk));
		//	/*m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 2, 0));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 4, 0));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 6, 0));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 0, 2));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 2, 2));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 4, 2));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 6, 2));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 0, 4));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 2, 4));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 4, 4));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 6, 4));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 0, 6));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 2, 6));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 4, 6));
		//	m_Threads.push_back(std::thread(&World::UpdateChunkBucket, this, chunk, 6, 6));*/
		//}

		//for each (std::thread& thread in m_Threads)
		//{
		//	thread.join();
		//}
		
		for each (auto & pair in m_Chunks)
		{
			pair.second->UpdateTexture();
		}
		

		//UpdateChunk(m_Chunks[{0, 0}]);

		m_UpdateBit = !m_UpdateBit;
	}

	//defines for easy to read code
	#define SwapWithOther chunk->SetElement(xOther, yOther, currElement); chunk->SetElement(x, y, other); UpdateChunkDirtyRect(x, y, chunk);
	#define SwapWithOtherChunk otherChunk->m_Elements[indexOther] = currElement; chunk->SetElement(x, y, other); UpdateChunkDirtyRect(x, y, chunk);

	#define SolveReaction currElement.m_ID = it->second.cell0ID;\
		m_ElementData[it->second.cell0ID].UpdateElementData(currElement);\
		other.m_ID = it->second.cell1ID;\
		m_ElementData[it->second.cell1ID].UpdateElementData(other);\
		chunk->m_Elements[xOther + yOther * CHUNKSIZE] = other;

	#define SolveReactionOtherChunk currElement.m_ID = it->second.cell0ID;\
		m_ElementData[it->second.cell0ID].UpdateElementData(currElement);\
		other.m_ID = it->second.cell1ID;\
		m_ElementData[it->second.cell1ID].UpdateElementData(other);\
		otherChunk->m_Elements[indexOther] = other;

	float interpolateBetweenValues(float lower, float higher, float val)
	{
		//460, 6000, 1000
		return std::min((std::max(lower, val) - lower), higher - lower) / (higher - lower);
	}


	/// <summary>
	/// 
	/// updating a chunk overview:
	/// copy the dirt rect state, and clear it so we can re-create it while updating
	/// loop over the elements, and update them. if the elements they are trying to swap with
	/// is out of bounds, then find the other chunk and get the element like that.
	/// because an element can belong to another chunk, there is a lot more conditional logic.
	/// const int BUCKETS = chunk->BUCKETS;
	/// const int BUCKETSIZE = chunk->BUCKETSIZE;
	/// </summary>
	/// <param name="chunk"></param>
	/// <param name="bucketX"></param>
	/// <param name="bucketY"></param>
	void World::UpdateChunkBucket(Chunk* chunk, int bucketX, int bucketY)
	{

		//copy the dirty rect
		std::pair<glm::ivec2, glm::ivec2> minmax = chunk->m_DirtyRects[bucketX + bucketY * BUCKETSWIDTH];

		//instead of resetting completely, just shrink by 1. This allows elements that cross borders to update, since otherwise the dirty rect would
		//forget about it instead of shrinking and still hitting it.
		chunk->m_DirtyRects[bucketX + bucketY * BUCKETSWIDTH] = std::pair<glm::ivec2, glm::ivec2>(
			minmax.first + 1, //min grows
			minmax.second - 1 //max shrinks
		);
		 
		//get the min and max
		//loop from min to max in both "axies"?
		bool directionBit = m_UpdateBit;

		///////////////////////////////////////////////////////////////
		/// Main Update Loop, back and forth bottom to top, left,right,left....
		///////////////////////////////////////////////////////////////
		int startNumy = m_UpdateBit ? std::max(minmax.first.y, bucketY * BUCKETSIZE) : std::min(minmax.second.y, ((bucketY + 1) * BUCKETSIZE) - 1);
		int compNumberY = m_UpdateBit ? std::min(minmax.second.y, ((bucketY + 1) * BUCKETSIZE) - 1) + 1 : std::max(minmax.first.y, bucketY * BUCKETSIZE) - 1;
		if (minmax.first.x <= minmax.second.x && minmax.first.y <= minmax.second.y)
		for (int y = startNumy; y != compNumberY; m_UpdateBit ? y++ : y--) //going y then x so we do criss crossing 
		{
			directionBit = !directionBit;
			int startNum   = directionBit ? std::max(minmax.first.x, bucketX * BUCKETSIZE) : std::min(minmax.second.x, ((bucketX + 1) * BUCKETSIZE) - 1);
			int compNumber = directionBit ? std::min(minmax.second.x, ((bucketX + 1) * BUCKETSIZE) - 1) + 1 : std::max(minmax.first.x, bucketX * BUCKETSIZE) - 1;
			//PX_TRACE("x is: {0}", x);
			for (int x = startNum; x != compNumber; directionBit ? x++ : x--)
			{
				//we now have an x and y of the element in the array, so update it
				Element& currElement = chunk->m_Elements[x + y * CHUNKSIZE];
				ElementData& currElementData = m_ElementData[currElement.m_ID];

				//skip if already updated
				if (currElement.m_Updated == m_UpdateBit) continue;
				//flip the update bit so we know we updated this element
				currElement.m_Updated = m_UpdateBit;

				if (currElement.m_ID == 0) continue;

				if (currElementData.glow)
				{
					int r = (currElement.m_BaseColor >> 0) & 255;
					int g = (currElement.m_BaseColor >> 8) & 255;
					int b = (currElement.m_BaseColor >> 16) & 255;
					r = std::max(r, (int)(Pyxis::interpolateBetweenValues(460, 900, currElement.m_Temperature) * 255.0f));
					g = std::max(g, (int)(Pyxis::interpolateBetweenValues(460, 1500, currElement.m_Temperature) * 255.0f));
					b = std::max(b, (int)(Pyxis::interpolateBetweenValues(1000, 6000, currElement.m_Temperature) * 255.0f));
					currElement.m_Color = (currElementData.color & 0xFF000000) | ((b & 255) << 16) | ((g & 255) << 8) | (r & 255);
					//make it appear hotter depending on temp
					//temp range from 460 to 6000
				}

				int xOther = x;
				int yOther = y;

				int r = 0;

				//iterators for reaction lookup
				std::unordered_map<uint32_t, ReactionResult>::iterator it;
				std::unordered_map<uint32_t, ReactionResult>::iterator end;


				Element* elementTop;
				Element* elementBottom;
				Element* elementRight;
				Element* elementLeft;
				
				//get cardinal elements
				{
					if (IsInBounds(x, y + 1))
					{
						elementTop = chunk->m_Elements + (x)+(y + 1) * CHUNKSIZE;
					}
					else
					{
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(x, y + 1);
						Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						int indexOther = ((((x)+CHUNKSIZE) % CHUNKSIZE) + (((y + 1) + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
						elementTop = otherChunk->m_Elements + indexOther;
					}

					if (IsInBounds(x, y - 1))
					{
						elementBottom = chunk->m_Elements + (x)+(y - 1) * CHUNKSIZE;
					}
					else
					{
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(x, y - 1);
						Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						int indexOther = ((((x)+CHUNKSIZE) % CHUNKSIZE) + (((y - 1) + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
						elementBottom = otherChunk->m_Elements + indexOther;
					}

					if (IsInBounds(x + 1, y))
					{
						elementRight = chunk->m_Elements + (x + 1) + (y)*CHUNKSIZE;
					}
					else
					{
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(x + 1, y);
						Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						int indexOther = ((((x + 1) + CHUNKSIZE) % CHUNKSIZE) + (((y)+CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
						elementRight = otherChunk->m_Elements + indexOther;
					}

					if (IsInBounds(x - 1, y))
					{
						elementLeft = chunk->m_Elements + (x - 1) + (y)*CHUNKSIZE;
					}
					else
					{
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(x - 1, y);
						Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						int indexOther = ((((x - 1) + CHUNKSIZE) % CHUNKSIZE) + (((y)+CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE);
						elementLeft = otherChunk->m_Elements + indexOther;
					}
				}

				//check for reactions, left,up,right,down
				{
					it = m_ReactionLookup[currElement.m_ID].find(elementLeft->m_ID);
					end = m_ReactionLookup[currElement.m_ID].end();
					if (it != end && (std::rand() % 101) <= it->second.probability)
					{
						currElement.m_ID = it->second.cell0ID;
						m_ElementData[it->second.cell0ID].UpdateElementData(currElement);
						elementLeft->m_ID = it->second.cell1ID;
						m_ElementData[it->second.cell1ID].UpdateElementData(elementLeft);
						UpdateChunkDirtyRect(x, y, chunk);
						continue;
					}

					it = m_ReactionLookup[currElement.m_ID].find(elementTop->m_ID);
					end = m_ReactionLookup[currElement.m_ID].end();
					if (it != end && (std::rand() % 101) <= it->second.probability)
					{
						currElement.m_ID = it->second.cell0ID;
						m_ElementData[it->second.cell0ID].UpdateElementData(currElement);
						elementTop->m_ID = it->second.cell1ID;
						m_ElementData[it->second.cell1ID].UpdateElementData(elementTop);
						UpdateChunkDirtyRect(x, y, chunk);
						continue;
					}

					it = m_ReactionLookup[currElement.m_ID].find(elementRight->m_ID);
					end = m_ReactionLookup[currElement.m_ID].end();
					if (it != end && (std::rand() % 101) <= it->second.probability)
					{
						currElement.m_ID = it->second.cell0ID;
						m_ElementData[it->second.cell0ID].UpdateElementData(currElement);
						elementRight->m_ID = it->second.cell1ID;
						m_ElementData[it->second.cell1ID].UpdateElementData(elementRight);
						UpdateChunkDirtyRect(x, y, chunk);
						continue;
					}

					it = m_ReactionLookup[currElement.m_ID].find(elementBottom->m_ID);
					end = m_ReactionLookup[currElement.m_ID].end();
					if (it != end && (std::rand() % 101) <= it->second.probability)
					{
						currElement.m_ID = it->second.cell0ID;
						m_ElementData[it->second.cell0ID].UpdateElementData(currElement);
						elementBottom->m_ID = it->second.cell1ID;
						m_ElementData[it->second.cell1ID].UpdateElementData(elementBottom);
						UpdateChunkDirtyRect(x, y, chunk);
						continue;
					}
				}

				//handle universal element properties
				if (currElement.m_Temperature >= currElementData.melting_point)
				{
					//melt
					if (currElementData.melted != "")
					{
						int newID = m_ElementIDs[currElementData.melted];
						int temp = currElement.m_Temperature;
						m_ElementData[newID].UpdateElementData(currElement);
						currElement.m_Temperature = temp;
						currElement.m_ID = newID;
						chunk->UpdateDirtyRect(x, y);
					}
				}

				if (currElement.m_Temperature <= currElementData.freezing_point)
				{
					//freeze
					if (currElementData.frozen != "")
					{
						int newID = m_ElementIDs[currElementData.frozen];
						int temp = currElement.m_Temperature;
						m_ElementData[newID].UpdateElementData(currElement);
						currElement.m_Temperature = temp;
						currElement.m_ID = newID;
						chunk->UpdateDirtyRect(x, y);
					}
				}

				if (currElementData.conductivity > 0)
				{
					float tempBefore = currElement.m_Temperature;
					int minConductivity = 0;
					float diff = 0;

					minConductivity = std::min(currElementData.conductivity, m_ElementData[elementLeft->m_ID].conductivity);
					diff = ((currElement.m_Temperature - elementLeft->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
					if (diff != 0)
					{
						currElement.m_Temperature -= diff;
						elementLeft->m_Temperature += diff;
					}

					minConductivity = std::min(currElementData.conductivity, m_ElementData[elementTop->m_ID].conductivity);
					diff = ((float)(currElement.m_Temperature - elementTop->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
					if (diff != 0)
					{
						currElement.m_Temperature -= diff;
						elementTop->m_Temperature += diff;
					}

					minConductivity = std::min(currElementData.conductivity, m_ElementData[elementRight->m_ID].conductivity);
					diff = ((float)(currElement.m_Temperature - elementRight->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
					if (diff != 0)
					{
						currElement.m_Temperature -= diff;
						elementRight->m_Temperature += diff;
					}

					minConductivity = std::min(currElementData.conductivity, m_ElementData[elementBottom->m_ID].conductivity);
					diff = ((float)(currElement.m_Temperature - elementBottom->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
					if (diff != 0)
					{
						currElement.m_Temperature -= diff;
						elementBottom->m_Temperature += diff;
					}
					if (currElement.m_Temperature != tempBefore) chunk->UpdateDirtyRect(x, y);
					

				}

				if (currElementData.flammable)
				{
					if (currElement.m_Temperature >= currElementData.ignition_temperature) currElement.m_Ignited = true;
					//check for open air to burn
					int fireID = m_ElementIDs["fire"];
					ElementData& fireElementData = m_ElementData[fireID];
					if (currElement.m_Ignited)
					{
						if (currElement.m_ID != fireID && std::rand() % 100 < 11)
						{
							ElementData& elementLeftData = m_ElementData[elementLeft->m_ID];
							if (elementLeftData.cell_type == ElementType::gas)
							{
								fireElementData.UpdateElementData(elementLeft);
								elementLeft->m_ID = fireID;
								elementLeft->m_Temperature = currElementData.fire_temperature;

							}
							ElementData& elementTopData = m_ElementData[elementTop->m_ID];
							if (elementTopData.cell_type == ElementType::gas)
							{
								fireElementData.UpdateElementData(elementTop);
								elementTop->m_ID = fireID;
								elementTop->m_Temperature = currElementData.fire_temperature;
							}
							ElementData& elementRightData = m_ElementData[elementRight->m_ID];
							if (elementRightData.cell_type == ElementType::gas)
							{
								fireElementData.UpdateElementData(elementRight);
								elementRight->m_ID = fireID;
								elementRight->m_Temperature = currElementData.fire_temperature;
							}
							ElementData& elementBottomData = m_ElementData[elementBottom->m_ID];
							if (elementBottomData.cell_type == ElementType::gas)
							{
								fireElementData.UpdateElementData(elementBottom);
								elementBottom->m_ID = fireID;
								elementBottom->m_Temperature = currElementData.fire_temperature;
							}
						}
						currElement.m_Health--;
						if (currElement.m_Health <= 0)
						{
							currElement.m_ID = m_ElementIDs[currElementData.burnt];
							m_ElementData[currElement.m_ID].UpdateElementData(currElement);
							continue;
						}
					}
					
					
				}


				//switch the behavior based on element type
				switch (currElementData.cell_type)
				{
				case ElementType::solid:
					break;
				case ElementType::movableSolid:
					//check below, and move
					{
						ElementData& otherData = m_ElementData[elementBottom->m_ID];
						if (otherData.cell_type != ElementType::solid && otherData.cell_type != ElementType::movableSolid)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementBottom;
							*elementBottom = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							elementLeft->m_Sliding = true;
							elementRight->m_Sliding = true;
							continue;
						}
					}
					

					if (currElement.m_Sliding)
					{
						//chance to stop sliding
						int rand = std::rand() % 101;
						if (rand <= currElementData.friction)
						{
							currElement.m_Sliding = false;
							currElement.m_Horizontal = 0;
							continue;
						}
						if (currElement.m_Horizontal == 0)
						{
							currElement.m_Horizontal = (std::rand() % 2 == 0) ? -1 : 1;
						}
					}
					else
					{
						continue;
					}

					//try moving to the side
					if (currElement.m_Horizontal > 0)
					{
						ElementData& otherData = m_ElementData[elementRight->m_ID];
						if (otherData.cell_type != ElementType::solid && otherData.cell_type != ElementType::movableSolid)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}
					else
					{
						ElementData& otherData = m_ElementData[elementLeft->m_ID];
						if (otherData.cell_type != ElementType::solid && otherData.cell_type != ElementType::movableSolid)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
					}

					break;
				case ElementType::liquid:
					//check below, and move
					xOther = x;
					yOther = y - 1;
					if (IsInBounds(xOther, yOther))
					{
						//operate within the chunk, since we are in bounds
						//Element& other = chunk->GetElement(xOther, yOther);
						Element other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOther;
							continue;
						}
					}
					else
					{
						//we need to get the element by finding the other chunk
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
						Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
						Element other = otherChunk->m_Elements[indexOther];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOtherChunk;
							continue;
						}
					}
						
					r = std::rand() & 1 ? 1 : -1;
					//int r = (x ^ 98252 + (m_UpdateBit * y) ^ 6234561) ? 1 : -1;

					//try left/right then bottom left/right
					xOther = x - r;
					yOther = y;
					if (IsInBounds(xOther, yOther))
					{
						//operate within the chunk, since we are in bounds
						//Element& other = chunk->GetElement(xOther, yOther);
						Element other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOther;
							continue;
						}
					}
					else
					{
						//we need to get the element by finding the other chunk
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
						Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
						Element other = otherChunk->m_Elements[indexOther];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOtherChunk;
							continue;
						}
					}

					xOther = x + r;
					//yOther = y;
					if (IsInBounds(xOther, yOther))
					{
						//operate within the chunk, since we are in bounds
						//Element& other = chunk->GetElement(xOther, yOther);
						Element other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOther;
							continue;
						}
					}
					else
					{
						//we need to get the element by finding the other chunk
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
						Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
						Element other = otherChunk->m_Elements[indexOther];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOtherChunk;
							continue;
						}
					}

					//bottom left/right
					xOther = x - r;
					yOther = y - 1;
					if (IsInBounds(xOther, yOther))
					{
						//operate within the chunk, since we are in bounds
						//Element& other = chunk->GetElement(xOther, yOther);
						Element other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOther;
							continue;
						}
					}
					else
					{
						//we need to get the element by finding the other chunk
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
						Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
						Element other = otherChunk->m_Elements[indexOther];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOtherChunk;
							continue;
						}
					}

					xOther = x + r;
					//yOther = y - 1;
					if (IsInBounds(xOther, yOther))
					{
						//operate within the chunk, since we are in bounds
						//Element& other = chunk->GetElement(xOther, yOther);
						Element other = chunk->m_Elements[xOther + yOther * CHUNKSIZE];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOther;
							continue;
						}
					}
					else
					{
						//we need to get the element by finding the other chunk
						glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
						Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
						int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
						Element other = otherChunk->m_Elements[indexOther];
						ElementData& otherData = m_ElementData[other.m_ID];
						if (otherData.cell_type == ElementType::gas || otherData.cell_type == ElementType::fire || (otherData.cell_type == ElementType::liquid && otherData.density < currElementData.density))
						{
							SwapWithOtherChunk;
							continue;
						}
					}


					////check if there is no momentum and add some if needed
					//if (currElement.m_Vertical != 0 && currElement.m_Horizontal == 0) {
					//	currElement.m_Horizontal = m_UpdateBit ? 1 : -1;
					//	currElement.m_Vertical = 0;
					//}
					////skip if we are not needing to move
					//if (currElement.m_Horizontal == 0) continue;
					////try a side to move to based on horizontal
					//if (currElement.m_Horizontal < 0)
					//	//next check left and try to move
					//	xOther = x - 1;
					//else
					//	xOther = x + 1;
					//yOther = y;
					//if (IsInBounds(xOther, yOther))
					//{
					//	//operate within the chunk, since we are in bounds
					//	Element other = chunk->GetElement(xOther, yOther);
					//	if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
					//	{
					//		//other element is air so we move to it
					//		chunk->SetElement(xOther, yOther, currElement);
					//		chunk->SetElement(x, y, Element());
					//		UpdateChunkDirtyRect(x, y, chunk);
					//		continue;
					//	}
					//}
					//else
					//{
					//	//we need to get the element by finding the other chunk
					//	glm::ivec2 pixelSpace = chunk->m_ChunkPos * CHUNKSIZE + glm::ivec2(xOther, yOther);
					//	Chunk* otherChunk = GetChunk(PixelToChunk(pixelSpace));
					//	int indexOther = ((xOther + CHUNKSIZE) % CHUNKSIZE) + ((yOther + CHUNKSIZE) % CHUNKSIZE) * CHUNKSIZE;
					//	Element other = otherChunk->m_Elements[indexOther];
					//	if (other.m_Type == ElementType::gas || other.m_Type == ElementType::fire)
					//	{
					//		//other element is air so we move to it
					//		otherChunk->m_Elements[indexOther] = currElement;
					//		chunk->SetElement(x, y, other);
					//		UpdateChunkDirtyRect(x, y, chunk);
					//		continue;
					//	}
					//}
					////since the liquid didn't move, swap the horizontal
					//currElement.m_Horizontal = -currElement.m_Horizontal;

					break;
				case ElementType::gas:
					r = (std::rand() % 3) - 1; //-1 0 1
					if (r == 0)
					{
						//check above, and move
						{
							ElementData& otherData = m_ElementData[elementTop->m_ID];
							if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
							{
								currElement.m_Sliding = true;
								Element temp = currElement;
								chunk->m_Elements[x + y * CHUNKSIZE] = *elementTop;
								*elementTop = temp;
								UpdateChunkDirtyRect(x, y, chunk);
								continue;
							}
						}
					}

					//try left/right
					if (r > 0)
					{
						ElementData& otherData = m_ElementData[elementRight->m_ID];
						if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						{
							ElementData& otherData = m_ElementData[elementLeft->m_ID];
							if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
							{
								currElement.m_Sliding = true;
								Element temp = currElement;
								chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
								*elementLeft = temp;
								UpdateChunkDirtyRect(x, y, chunk);
								continue;
							}
						}
					}
					else
					{
						ElementData& otherData = m_ElementData[elementLeft->m_ID];
						if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						{
							ElementData& otherData = m_ElementData[elementRight->m_ID];
							if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
							{
								currElement.m_Sliding = true;
								Element temp = currElement;
								chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
								*elementRight = temp;
								UpdateChunkDirtyRect(x, y, chunk);
								continue;
							}
						}
					}

					break;
				case ElementType::fire:
					r = (std::rand() % 3) - 1; //-1 0 1
					if (r == 0)
					{
						//check above, and move
						{
							ElementData& otherData = m_ElementData[elementTop->m_ID];
							if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
							{
								currElement.m_Sliding = true;
								Element temp = currElement;
								chunk->m_Elements[x + y * CHUNKSIZE] = *elementTop;
								*elementTop = temp;
								UpdateChunkDirtyRect(x, y, chunk);
								continue;
							}
						}
					}

					//try left/right
					if (r > 0)
					{
						ElementData& otherData = m_ElementData[elementRight->m_ID];
						if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
							*elementRight = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						{
							ElementData& otherData = m_ElementData[elementLeft->m_ID];
							if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
							{
								currElement.m_Sliding = true;
								Element temp = currElement;
								chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
								*elementLeft = temp;
								UpdateChunkDirtyRect(x, y, chunk);
								continue;
							}
						}
					}
					else
					{
						ElementData& otherData = m_ElementData[elementLeft->m_ID];
						if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
						{
							currElement.m_Sliding = true;
							Element temp = currElement;
							chunk->m_Elements[x + y * CHUNKSIZE] = *elementLeft;
							*elementLeft = temp;
							UpdateChunkDirtyRect(x, y, chunk);
							continue;
						}
						{
							ElementData& otherData = m_ElementData[elementRight->m_ID];
							if (otherData.cell_type == ElementType::gas && otherData.density < currElementData.density)
							{
								currElement.m_Sliding = true;
								Element temp = currElement;
								chunk->m_Elements[x + y * CHUNKSIZE] = *elementRight;
								*elementRight = temp;
								UpdateChunkDirtyRect(x, y, chunk);
								continue;
							}
						}
					}

					break;
				}

			}
		}
	}

	void World::UpdateChunkDirtyRect(int x, int y, Chunk* chunk)
	{
		//this needs to update the surrounding buckets if the coord is on the corresponding edge
		//ex: if on top edge, also update that bucket using this x/y. this is fine since it clamps to
		//bucket size anyway
		std::pair<glm::ivec2, glm::ivec2>* minmax = (chunk->m_DirtyRects + (x / BUCKETSIZE) + (y / BUCKETSIZE) * BUCKETSWIDTH);
		//update minimums
		if (x < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = x - chunk->m_DirtyRectBorderWidth;
		if (y < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = y - chunk->m_DirtyRectBorderWidth;
		//update maximums
		if (x > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = x + chunk->m_DirtyRectBorderWidth;
		if (y > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = y + chunk->m_DirtyRectBorderWidth;

		//there are only 8 cases, so i will use a switch statement with bits?
		int result = 0;
		if (y % BUCKETSIZE == BUCKETSIZE - 1) result |= 8; //top
		if (x % BUCKETSIZE == BUCKETSIZE - 1) result |= 4; //right
		if (y % BUCKETSIZE == 0)			  result |= 2; //bottom
		if (x % BUCKETSIZE == 0)			  result |= 1; //left

		if (result == 0) return;
		//since we are on an edge, see if we need to get a different chunk

		//working on updating chunks
		Chunk* ChunkToUpdate;
		glm::ivec2 currentChunk = chunk->m_ChunkPos;
		int xOther = 0, yOther = 0;

		switch (result)
		{
		case 8:  // top

			//can optimize this if needed
			/*if (x >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			if (y >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			if (x < 0) currentChunk += glm::ivec2(-1, 0);
			if (y < 0) currentChunk += glm::ivec2(0, -1);*/
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 12: // top right
			
			//top
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//top right
			currentChunk = chunk->m_ChunkPos;
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			if (x + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//right
			currentChunk = chunk->m_ChunkPos;
			if (x + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}
			break;
		case 4:  // right
			if (x+1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 6:  // right bottom

			//right
			if (x + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//br
			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			if (x + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x + 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//bottom
			currentChunk = chunk->m_ChunkPos;
			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 2:  // bottom

			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 3:  // bottom left

			//bottom
			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//bottom left
			currentChunk = chunk->m_ChunkPos;
			if (y - 1 < 0) currentChunk += glm::ivec2(0, -1);
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y - 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//left
			currentChunk = chunk->m_ChunkPos;
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			break;
		case 1:  // left
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}
			break;
		case 9:  // top left
			//left
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);

			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = (y + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}

			//top left
			currentChunk = chunk->m_ChunkPos;
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			if (x - 1 < 0) currentChunk += glm::ivec2(-1, 0);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = ((x - 1) + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}
			break;

			//top
			currentChunk = chunk->m_ChunkPos;
			if (y + 1 >= CHUNKSIZE) currentChunk += glm::ivec2(0, 1);
			ChunkToUpdate = (chunk->m_ChunkPos == currentChunk) ? chunk : GetChunk(currentChunk);
			xOther = (x + CHUNKSIZE) % CHUNKSIZE;
			yOther = ((y + 1) + CHUNKSIZE) % CHUNKSIZE;
			{
				minmax = ChunkToUpdate->m_DirtyRects + (xOther / BUCKETSIZE) + (yOther / BUCKETSIZE) * BUCKETSWIDTH;
				//update minimums
				if (xOther < minmax->first.x + chunk->m_DirtyRectBorderWidth) minmax->first.x = xOther - chunk->m_DirtyRectBorderWidth;
				if (yOther < minmax->first.y + chunk->m_DirtyRectBorderWidth) minmax->first.y = yOther - chunk->m_DirtyRectBorderWidth;
				//update maximums
				if (xOther > minmax->second.x - chunk->m_DirtyRectBorderWidth) minmax->second.x = xOther + chunk->m_DirtyRectBorderWidth;
				if (yOther > minmax->second.y - chunk->m_DirtyRectBorderWidth) minmax->second.y = yOther + chunk->m_DirtyRectBorderWidth;
			}
		}
	}

	/// <summary>
	/// DEPRECATED, no longer used
	/// </summary>
	void World::UpdateChunk(Chunk* chunk)
	{
		//updating a chunk overview:
		//copy the dirt rect state, and clear it so we can re-create it while updating
		//loop over the elements, and update them. if the elements they are trying to swap with
		//is out of bounds, then find the other chunk and get the element like that.
		//because an element can belong to another chunk, there is a lot more conditional logic.
		//const int BUCKETS = chunk->BUCKETS;
		//const int BUCKETSIZE = chunk->BUCKETSIZE;


		//copy the dirty rects
		std::pair<glm::ivec2, glm::ivec2> DirtyRects[BUCKETSWIDTH * BUCKETSWIDTH];
		memcpy(DirtyRects, chunk->m_DirtyRects, sizeof(DirtyRects));
		//fix this if uncommented
		//chunk->m_DirtyRects[bucketX + bucketY * BUCKETSWIDTH] = std::pair<glm::ivec2, glm::ivec2>(glm::ivec2(), glm::ivec2());
		chunk->ResetDirtyRects();
		//memset(chunk->m_DirtyRects, 0, sizeof(DirtyRects));

		//loop over each dirty rect
		for (int bx = 0; bx < BUCKETSWIDTH; bx++)
		{
			for (int by = 0; by < BUCKETSWIDTH; by++)
			{
				//get the min and max
				auto& minmax = DirtyRects[bx + by * BUCKETSWIDTH];
				Element currElement;
				//loop from min to max in both "axies"?
				for (int x = minmax.first.x; x <= minmax.second.x; x++)
				{
					for (int y = minmax.first.y; y <= minmax.second.y; y++)
					{
						//we now have an x and y of the element in the array, so update it
						currElement = chunk->m_Elements[x + y * CHUNKSIZE];
						ElementData& currElementData = m_ElementData[currElement.m_ID];
						//skip if already updated
						if (currElement.m_Updated == m_UpdateBit) continue;
						//flip the update bit so we know we updated this element
						currElement.m_Updated = m_UpdateBit;

						//switch the behavior based on element type
						switch (currElementData.cell_type)
						{
						case ElementType::solid:

							//check below, and move
							if (IsInBounds(x, y - 1))
							{
								//operate within the chunk, since we are in bounds
								if (chunk->GetElement(x, y - 1).m_ID == 0)
								{
									//other element is air so we move to it
									chunk->SetElement(x, y - 1, currElement);
									chunk->SetElement(x, y, Element());
									chunk->UpdateDirtyRect(x, y);
									chunk->UpdateDirtyRect(x, y - 1);
									continue;
								}
							}
							else
							{
								//we need to get the element by finding the other chunk

							}

							break;
						}

					}
				}
			}
		}
	}

	void World::SetElement(const glm::ivec2& pixelPos, const Element& element)
	{
		//PX_TRACE("Setting element at ({0}, {1})", pixelPos.x, pixelPos.y);
		glm::ivec2 index = PixelToIndex(pixelPos);
		Element e = element;
		e.m_Updated = !m_UpdateBit;
		GetChunk(PixelToChunk(pixelPos))->SetElement(index.x, index.y, e);
	}

	void World::RenderWorld()
	{
		
		//PX_TRACE("Rendering world");
		for each (auto pair in m_Chunks)
		{
			//PX_TRACE("Drawing chunk {0}, {1}", pair.second->m_ChunkPos.x, pair.second->m_ChunkPos.y);
			Renderer2D::DrawQuad(glm::vec2(pair.second->m_ChunkPos.x + 0.5f, pair.second->m_ChunkPos.y + 0.5f), { 1,1 }, pair.second->m_Texture);
			
			//Renderer2D::DrawQuad(glm::vec3(pair.second->m_ChunkPos.x + 0.5f, pair.second->m_ChunkPos.y + 0.5f, 1.0f), {0.1f, 0.1f}, glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
		}
	}


	const bool World::IsInBounds(int x, int y)
	{
		//having y first might actually be faster, simply because things tend to fall
		if (y < 0 || y >= CHUNKSIZE) return false;
		if (x < 0 || x >= CHUNKSIZE) return false;
		return true;
	}

	//Helper to get a chunk from a world pixel position
	glm::ivec2 World::PixelToChunk(const glm::ivec2& pixelPos)
	{
		glm::ivec2 result;
		if (pixelPos.x < 0)
		{
			result.x = (pixelPos.x + 1) / CHUNKSIZE;
			result.x--;
		}
		else result.x = pixelPos.x / CHUNKSIZE;
		if (pixelPos.y < 0)
		{
			result.y = (pixelPos.y + 1) / CHUNKSIZE;
			result.y--;
		}
		else result.y = pixelPos.y / CHUNKSIZE;
		return result;
	}

	//Helper to get an index from a world pixel position
	glm::ivec2 World::PixelToIndex(const glm::ivec2& pixelPos)
	{
		glm::ivec2 result;
		if (pixelPos.x < 0)
		{
			result.x = CHUNKSIZE - (std::abs(pixelPos.x) % CHUNKSIZE);
		}
		else result.x = pixelPos.x % CHUNKSIZE;
		if (pixelPos.y < 0)
		{
			result.y = CHUNKSIZE - (std::abs(pixelPos.y) % CHUNKSIZE);
		}
		else result.y = pixelPos.y % CHUNKSIZE;
		return result;
	}

}

