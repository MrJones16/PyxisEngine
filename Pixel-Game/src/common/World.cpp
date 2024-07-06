#include "World.h"
#include "ChunkWorker.h"
#include <random>
#include <thread>
#include <mutex>

#include <tinyxml2/tinyxml2.h>
#include <box2d/b2_math.h>
#include <poly2tri/poly2tri.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Pyxis
{
	World::World(std::string assetPath, int seed)
	{
		
		m_Box2DWorld = new b2World({ 0, -9.8f });

		if (!LoadElementData(assetPath))
		{
			PX_ASSERT(false, "Failed to load element data, shutting down.");
			PX_ERROR("Failed to load element data, shutting down.");
			m_Error = true;
			Application::Get().Sleep(10000);
			Application::Get().Close();
			return;
		}

		BuildReactionTable();

		//set up world noise data
		Initialize(seed);
	}

	void World::Initialize(int worldSeed)
	{
		m_HeightNoise = FastNoiseLite(m_WorldSeed);
		m_CaveNoise = FastNoiseLite(m_WorldSeed);
	}

	bool World::LoadElementData(std::string assetPath)
	{
		//loading element data from xml
		tinyxml2::XMLDocument doc;
		doc.LoadFile((assetPath + "/data/CellData.xml").c_str());
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

				///////////////////////////////////////////////////
				/// Basic Attributes
				///////////////////////////////////////////////////
				
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

				//get the texture
				const char* texture;
				error = data->QueryAttribute("texture", &texture);
				if (!error) {
					elementData.SetTexture(assetPath + "/" + texture);
				}
				else {
					PX_ERROR("No texture given to element, using color");
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
				if (!error) {
					uint32_t color = std::stoul(colorStr, nullptr, 16);
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

				///////////////////////////////////////////////////
				/// Flammable Attributes
				///////////////////////////////////////////////////

				bool flammable = false;
				error = data->QueryBoolAttribute("flammable", &flammable);
				if (!error) {
					elementData.flammable = flammable;
				}
				else
				{
					PX_INFO("Element {0} was given no flammable attribute", elementData.name);
				}

				bool spread_ignition = false;
				error = data->QueryBoolAttribute("spread_ignition", &spread_ignition);
				if (!error) {
					elementData.spread_ignition = spread_ignition;
				}
				else
				{
					PX_INFO("Element {0} was given no spread_ignition attribute", elementData.name);
				}

				
				uint32_t spread_ignition_chance = 0;
				error = data->QueryUnsignedAttribute("spread_ignition_chance", &spread_ignition_chance);
				if (!error) {
					elementData.spread_ignition_chance = spread_ignition_chance;
				}
				else
				{
					PX_INFO("Element {0} was given no spread_ignition_chance", elementData.name);
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

				const char* ignitedColorStr;
				error = data->QueryAttribute("ignited_color", &ignitedColorStr);
				if (!error) {
					uint32_t ignited_color = std::stoul(ignitedColorStr, nullptr, 16);
					elementData.ignited_color = RGBAtoABGR(ignited_color);
				}
				else
				{
					PX_INFO("Element {0} was given no ignited_color", elementData.name);
					elementData.ignited_color = 0;
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
					PX_INFO("Element {0} was given no fire_temperature", elementData.name);
				}

				const char* fireColorStr;
				error = data->QueryAttribute("fire_color", &fireColorStr);
				if (!error) {
					uint32_t fire_color = std::stoul(fireColorStr, nullptr, 16);
					elementData.fire_color = RGBAtoABGR(fire_color);
				}
				else
				{
					PX_INFO("Element {0} was given no fire_color", elementData.name);
				}

				float fire_temperature_increase = 1.0f;
				error = data->QueryFloatAttribute("fire_temperature_increase", &fire_temperature_increase);
				if (!error) {
					elementData.fire_temperature_increase = fire_temperature_increase;
				}
				else
				{
					elementData.fire_temperature_increase = (elementData.fire_temperature) / elementData.health;
					PX_INFO("Element {0} was given no fire_temperature_increase, defaulting to {1}", elementData.name, elementData.fire_temperature_increase);

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

				///////////////////////////////////////////////////
				/// Temperature Attributes
				///////////////////////////////////////////////////

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
	}


	/// <summary>
	/// Takes the message with the world data, and loads the world with it
	/// Expects this order to pull items out
	///
	/// world seed				| int
	/// 
	/// How many chunks			| uint32
	///		chunk pos			| ivec2
	///		chunk dirtyrects	| pair<glm::ivec2,glm::ivec2>
	///		chunk data			| Element...
	/// how many pixel bodies	| uint32
	///		rigid body data		| 
	///			id				| uint64_t
	///			size			| glm::ivec2
	///			array			| Element*
	///			position	
	///			rotation
	///			type
	///			angular velocity
	///			linear velocity
	/// </summary>
	/// <param name="msg"></param>
	void World::LoadWorld(Network::Message<GameMessage>& msg)
	{

		//things still to synchronize:
		//
		// pixel body velocity
		// chunk dirty rects
		// 
		//
		
		msg >> m_WorldSeed;
		Initialize(m_WorldSeed);

		msg >> m_UpdateBit;
		msg >> m_SimulationTick;

		//Loading the chunks
		uint32_t chunkCount = 0;
		msg >> chunkCount;
		PX_TRACE("Going to load {0} chunks", chunkCount);
		glm::ivec2 chunkPos;
		for (int i = 0; i < chunkCount; i++)
		{
			msg >> chunkPos;

			PX_ASSERT(m_Chunks.find(chunkPos) == m_Chunks.end(), "Tried to load a chunk that already existed");
			Chunk* chunk = new Chunk(chunkPos);
			for (int ii = (BUCKETSWIDTH * BUCKETSWIDTH) - 1; ii >= 0; ii--)
			{
				msg >> chunk->m_DirtyRects[ii];
			}
			m_Chunks[chunkPos] = chunk;
			for (int ii = (CHUNKSIZE * CHUNKSIZE) - 1; ii >= 0; ii--)
			{
				msg >> chunk->m_Elements[ii];
			}

			chunk->UpdateWholeTexture();

		}


		uint32_t pixelBodyCount = 0;
		msg >> pixelBodyCount;
		PX_TRACE("Going to load {0} pixel bodies", pixelBodyCount);
		for (int i = 0; i < pixelBodyCount; i++)
		{
			uint64_t uuid;
			glm::ivec2 size;
			msg >> uuid >> size;
			std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements;
			int count;
			msg >> count;
			for (int i = 0; i < count; i++)
			{
				glm::ivec2 localpos;
				RigidBodyElement rbe;
				msg >> localpos >> rbe;
				elements[localpos] = rbe;
			}
			glm::vec2 position;
			float rotation;
			b2BodyType type;
			float angularVelocity;
			b2Vec2 linearVelocity;
			msg >> position >> rotation >> type >> angularVelocity >> linearVelocity;
			PixelRigidBody* body = new PixelRigidBody(uuid, size, elements, type, m_Box2DWorld);
			m_PixelBodyMap[uuid] = body;
			body->SetTransform(position, rotation);
			body->SetAngularVelocity(angularVelocity);
			body->SetLinearVelocity(linearVelocity);
			// can safely ignore putting it in the world, since when we got the world
			// data it is already there!
			
		}
	}

	void World::GetWorldData(Network::Message<GameMessage>& msg)
	{
		for each (auto pair in m_PixelBodyMap)
		{
			msg << pair.second->m_B2Body->GetLinearVelocity();
			msg << pair.second->m_B2Body->GetAngularVelocity();
			msg << pair.second->m_B2Body->GetType();
			msg << pair.second->m_B2Body->GetAngle();
			msg << pair.second->m_B2Body->GetPosition();
			//msg << pair.second->m_Elements;
			int elementCount = 0;
			for each (auto pair in pair.second->m_Elements)
			{
				msg << pair.second;
				msg << pair.first;
				elementCount++;
			}
			msg << elementCount;
			glm::ivec2 size = { pair.second->m_Width, pair.second->m_Height };
			msg << size;
			msg << pair.first;
		}
		msg << uint32_t(m_PixelBodyMap.size());
		PX_TRACE("Uploaded {0} pixel bodies", m_PixelBodyMap.size());

		for each (auto pair in m_Chunks)
		{
			for (int i = 0; i < CHUNKSIZE * CHUNKSIZE; i++)
			{
				msg << pair.second->m_Elements[i];
			}
			for each (auto minmax in pair.second->m_DirtyRects)
			{
				msg << minmax;
			}
			msg << pair.first;
		}

		msg << uint32_t(m_Chunks.size());
		PX_TRACE("Uploaded {0} chunks", m_Chunks.size());

		msg << m_SimulationTick;
		msg << m_UpdateBit;
		msg << m_WorldSeed;
	}

	bool World::StringContainsTag(const std::string& string)
	{
		if (string.find("[") != std::string::npos && string.find("]") != std::string::npos) return true;
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

	World::~World()
	{
		PX_TRACE("Deleting World");

		//delete pixel bodies
		for each (auto pixelBody in m_PixelBodyMap)
		{
			delete pixelBody.second;
		}
		delete m_Box2DWorld;
		m_Box2DWorld = nullptr;
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
			GenerateChunk(chunk);
			chunk->UpdateWholeTexture();
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

	void World::GenerateChunk(Chunk* chunk)
	{
		glm::vec2 chunkPixelPos = chunk->m_ChunkPos * CHUNKSIZE;
		for (int x = 0; x < CHUNKSIZE; x++)
		{
			for (int y = 0; y < CHUNKSIZE; y++)
			{
				glm::vec2 pixelPos = chunkPixelPos + glm::vec2(x,y);
				if (pixelPos.y >=0)
				{
					float amplitude = 20.0f;
					float heightNoise = ((m_HeightNoise.GetNoise(pixelPos.x, 0.0f) + 1) / 2) * amplitude;
					float surfaceTop = heightNoise + 80;
					float grassWidth = 20;
					if (pixelPos.y > surfaceTop + grassWidth)
					{
						//air
					}
					else if (pixelPos.y > surfaceTop)
					{
						chunk->SetElement(x, y, GetElementByName("grass", x, y));
					}
					else if (pixelPos.y > heightNoise)
					{
						//dirt
						chunk->SetElement(x, y, GetElementByName("dirt", x, y));
					}
					else
					{
						//under the noise value, so stone, blended into the caves
						float caveNoise = (m_CaveNoise.GetNoise(pixelPos.x, pixelPos.y) + 1) / 2.0f;
						if (caveNoise >= 0.25f)
						{
							chunk->SetElement(x, y, GetElementByName("stone", x, y));
						}
					}
					
				}
				else
				{
					//under y==0, so 
					float caveNoise = (m_CaveNoise.GetNoise(pixelPos.x, pixelPos.y) + 1) / 2.0f;
					if (caveNoise >= 0.25f)
					{
						chunk->SetElement(x, y, GetElementByName("stone", x, y));
					}
				}
			}
		}
		
	}

	Element World::GetElementByName(std::string elementName, int x, int y)
	{
		Element& result = Element();
		result.m_ID = m_ElementIDs[elementName];
		m_ElementData[result.m_ID].UpdateElementData(result, x, y);
		return result;
	}

	Element& World::GetElement(const glm::ivec2& pixelPos)
	{
		auto chunkPos = PixelToChunk(pixelPos);
		auto index = PixelToIndex(pixelPos);
		return GetChunk(chunkPos)->m_Elements[index.x + index.y * CHUNKSIZE];
	}

	void World::SetElement(const glm::ivec2& pixelPos, const Element& element)
	{
		Chunk* chunk = GetChunk(PixelToChunk(pixelPos));
		auto index = PixelToIndex(pixelPos);
		if (element.m_ID == m_ElementIDs["debug_heat"])
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature++;
		}
		else if (element.m_ID == m_ElementIDs["debug_cool"])
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature--;
		}
		else
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE] = element;
		}
		UpdateChunkDirtyRect(index.x, index.y, chunk);

	}

	void World::SetElementWithoutDirtyRectUpdate(const glm::ivec2& pixelPos, const Element& element)
	{
		Chunk* chunk = GetChunk(PixelToChunk(pixelPos));
		auto index = PixelToIndex(pixelPos);
		if (element.m_ID == m_ElementIDs["debug_heat"])
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature++;
		}
		else if (element.m_ID == m_ElementIDs["debug_cool"])
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE].m_Temperature--;
		}
		else
		{
			chunk->m_Elements[index.x + index.y * CHUNKSIZE] = element;
		}

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

		
		//keep a vector of new bodies to add to the simulation if any get split
		std::vector<PixelRigidBody*> newPixelBodies;
		//keep a vector of new bodies to remove from the simulation if any get erased
		std::vector<uint64_t> erasedPixelBodies;

		//pull pixel bodies out of the simulation
		for each (auto pair in m_PixelBodyMap)
		{
			if (!pair.second->m_B2Body->IsAwake()) 
			{
				//skip sleeping bodies
				continue;
			}
			glm::ivec2 centerPixelWorld = { pair.second->m_B2Body->GetPosition().x * PPU, pair.second->m_B2Body->GetPosition().y * PPU };


			std::vector<glm::ivec2> elementsToRemove;
			for each (auto mappedElement in pair.second->m_Elements)
			{
				//the world position of the element is already known
				Element& worldElement = GetElement(mappedElement.second.worldPos);
				ElementData& elementData = m_ElementData[worldElement.m_ID];
				if (!mappedElement.second.hidden) // make sure we aren't hidden before trying to pull
				if (mappedElement.second.element.m_ID != worldElement.m_ID || !worldElement.m_Rigid)
				{

					//element has changed over the last update, could have been removed
					//or melted or something of the sort.
					// if it isn't just hidden, we need to re-create our rigid body!
					
					if (elementData.cell_type == ElementType::solid || (elementData.cell_type == ElementType::movableSolid && worldElement.m_Rigid))
					{
						//replaced element is able to continue being part of the solid!
						//this could be the player replacing the blocks, or a solid block
						//reacts with something and stays solid, like getting stained or something
						//idk
						//either way, in this situation we just pull the new element
						pair.second->m_Elements[mappedElement.first].element = worldElement;
						SetElementWithoutDirtyRectUpdate(mappedElement.second.worldPos, Element());
					}
					else
					{
						//the element that has taken over the spot is not able to be
						//a solid, so we need to re-construct the rigid body without that
						//element! so we leave it in the sim, and erase the previous from
						//the body
						elementsToRemove.push_back(mappedElement.first);
					}
					PX_TRACE("Element changed over update at ({0},{1})", mappedElement.second.worldPos.x, mappedElement.second.worldPos.y);
				}
				else
				{
					//element should be the same, so nothing has changed, pull the element out
					pair.second->m_Elements[mappedElement.first].element = worldElement;
					SetElementWithoutDirtyRectUpdate(mappedElement.second.worldPos, Element());
				}
			}
			m_PixelBodyMap[pair.first]->m_InWorld = false;
			//now that we pulled all the elements out, try to re-construct the body if needed:
			if (elementsToRemove.size() > 0)
			{
				//we need to reconstruct!

				//remove the outdated elements
				for each (auto localPos in elementsToRemove)
				{
					pair.second->m_Elements.erase(localPos);
				}

				//TODO: get this out of the iteration...
				
				auto newBodies = pair.second->RecreateB2Body(m_SimulationTick * 11, m_Box2DWorld);
				if (pair.second->m_B2Body == nullptr)
				{
					//we need to delete the pixel body, since it is now empty!
					erasedPixelBodies.push_back(pair.first);
				}
				for each (auto body in newBodies)
				{
					newPixelBodies.push_back(body);
				}
			}
		}
		//add the new bodies to the simulation, because they are not in the world and don't
		//need to be pulled out!
		for each (auto body in newPixelBodies)
		{
			m_PixelBodyMap[body->m_ID] = body;
		}

		//erase the erased bodies
		for each (auto ID in erasedPixelBodies)
		{
			delete m_PixelBodyMap[ID];
			m_PixelBodyMap.erase(ID);
		}

		int velocityIterations = 6;
		int positionIterations = 2;
		m_Box2DWorld->Step(1.0f / 60.0f, velocityIterations, positionIterations);

		//put pixel bodies back into the simulation, and solve collisions
		for each (auto pair in m_PixelBodyMap)
		{
			//skip bodies left in the world that were sleeping
			if (pair.second->m_InWorld) continue;
			pair.second->m_InWorld = true;

			glm::ivec2 centerPixelWorld = { pair.second->m_B2Body->GetPosition().x * PPU, pair.second->m_B2Body->GetPosition().y * PPU };

			float angle = pair.second->m_B2Body->GetAngle();
			auto rotationMatrix = glm::mat2x2(1);

			//if angle gets above 45 degrees, apply a 90 deg rotation first
			while (angle > 0.78539816339f)
			{
				angle -= 1.57079632679f;
				rotationMatrix *= glm::mat2x2(0, -1, 1, 0);
			}
			while (angle < -0.78539816339f)
			{
				angle += 1.57079632679f;
				rotationMatrix *= glm::mat2x2(0, 1, -1, 0);
			}

			float A = -std::tan(angle / 2);
			float B = std::sin(angle);
			auto horizontalSkewMatrix = glm::mat2x2(1, 0, A, 1);//0 a
			auto verticalSkewMatrix = glm::mat2x2(1, B, 0, 1);// b 0
			for each (auto mappedElement in pair.second->m_Elements)
			{
				//find the element in the world by using the transform of the body and
				//the position of the element in the array
				glm::ivec2 skewedPos = mappedElement.first * rotationMatrix;

				//horizontal skew:
				int horizontalSkewAmount = (float)skewedPos.y * A;
				skewedPos.x += horizontalSkewAmount;

				//vertical skew
				int skewAmount = (float)skewedPos.x * B;
				skewedPos.y += skewAmount;

				//horizontal skew:
				horizontalSkewAmount = (float)skewedPos.y * A;
				skewedPos.x += horizontalSkewAmount;

				//pixel pos is the pixel at the center of the array, so lets use it
				//the new position of the element is the skewed position + center offsef
				glm::ivec2 worldPixelPos = glm::ivec2(skewedPos.x, skewedPos.y) + centerPixelWorld;
				//now that we know the world position, cache it for pulling it out later
				pair.second->m_Elements[mappedElement.first].worldPos = worldPixelPos;
				//PX_TRACE("placed world pos: ({0},{1})", worldPixelPos.x, worldPixelPos.y);
				//set the world element from the array at the found position
				if (GetElement(worldPixelPos).m_ID != 0)
				{
					//we are replacing something that is in the way!

					//TODO: instead of always hiding, throw liquids ect as a particle!
					pair.second->m_Elements[mappedElement.first].hidden = true;

				}
				else
				{
					//we are no longer hidden!
					pair.second->m_Elements[mappedElement.first].hidden = false;
					if (std::abs(pair.second->m_B2Body->GetAngularVelocity()) > 0.01f || pair.second->m_B2Body->GetLinearVelocity().LengthSquared() > 0.01f)
					{
						//we are moving, so update dirty rect
						//PX_TRACE("we are moving!: angular: {0}, linear: {1}", pair.second->m_B2Body->GetAngularVelocity(), pair.second->m_B2Body->GetLinearVelocity().LengthSquared());
						SetElement(worldPixelPos, mappedElement.second.element);
					}
					else
					{
						//we are still, so stop updating region!
						SetElementWithoutDirtyRectUpdate(worldPixelPos, mappedElement.second.element);
					}
					
				}

			}
		}


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

		
		if (!m_ServerMode)
		{
			for each (auto & pair in m_Chunks)
			{
				pair.second->UpdateTexture();
			}
		}
		

		//UpdateChunk(m_Chunks[{0, 0}]);

		m_UpdateBit = !m_UpdateBit;
		m_SimulationTick++;
	}

	void World::UpdateTextures()
	{
		for each (auto & pair in m_Chunks)
		{
			pair.second->UpdateTexture();
		}
	}

	//defines for easy to read code
	#define SwapWithOther chunk->SetElement(xOther, yOther, currElement); chunk->SetElement(x, y, other); UpdateChunkDirtyRect(x, y, chunk);
	#define SwapWithOtherChunk otherChunk->m_Elements[indexOther] = currElement; chunk->SetElement(x, y, other); UpdateChunkDirtyRect(x, y, chunk);

	/// <summary>
	/// returns the percent value of the value between the points, from 0-1
	/// </summary>

	float interpolateBetweenValues(float lower, float higher, float val)
	{
		//460, 6000, 1000
		return std::min((std::max(lower, val) - lower), higher - lower) / (higher - lower);
	}


	/// <summary>
	/// 
	/// updating a chunk overview:
	/// copy the dirt rect state, and shrink it by 1
	/// loop over the elements, and update them. if the elements they are trying to swap with
	/// is out of bounds, then find the other chunk and get the element like that.
	/// because an element can belong to another chunk, there is a lot more conditional logic.
	/// const int BUCKETS = chunk->BUCKETS;
	/// const int BUCKETSIZE = chunk->BUCKETSIZE;
	/// </summary>
	void World::UpdateChunkBucket(Chunk* chunk, int bucketX, int bucketY)
	{

		//copy the dirty rect
		std::pair<glm::ivec2, glm::ivec2> minmax = chunk->m_DirtyRects[bucketX + bucketY * BUCKETSWIDTH];

		//make sure to wake up any pixel bodies in area
		b2AABB queryRegion;
		glm::vec2 lower = glm::vec2(minmax.first + (chunk->m_ChunkPos * CHUNKSIZE)) / PPU;
		glm::vec2 upper = glm::vec2(minmax.second + (chunk->m_ChunkPos * CHUNKSIZE)) / PPU;
		queryRegion.lowerBound = b2Vec2(lower.x, lower.y);
		queryRegion.upperBound = b2Vec2(upper.x, upper.y);
		WakeUpQueryCallback callback;
		m_Box2DWorld->QueryAABB(&callback, queryRegion);

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
				//first lets seed random, so the simulation is deterministic!
				SeedRandom(x, y);
				Element& currElement = chunk->m_Elements[x + y * CHUNKSIZE];
				ElementData& currElementData = m_ElementData[currElement.m_ID];

				//skip if already updated
				if (currElement.m_Updated == m_UpdateBit) continue;
				//flip the update bit so we know we updated this element
				currElement.m_Updated = m_UpdateBit;

				if (currElement.m_ID == 0) continue;
				

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

				ElementData& elementLeftData = m_ElementData[elementLeft->m_ID];
				ElementData& elementRightData = m_ElementData[elementRight->m_ID];
				ElementData& elementTopData = m_ElementData[elementTop->m_ID];
				ElementData& elementBottomData = m_ElementData[elementBottom->m_ID];
				//check for reactions, left,up,right,down
				{
					it = m_ReactionLookup[currElement.m_ID].find(elementLeft->m_ID);
					end = m_ReactionLookup[currElement.m_ID].end();
					if (it != end && (std::rand() % 101) <= it->second.probability)
					{
						currElement.m_ID = it->second.cell0ID;
						m_ElementData[it->second.cell0ID].UpdateElementData(currElement, x, y);
						elementLeft->m_ID = it->second.cell1ID;
						m_ElementData[it->second.cell1ID].UpdateElementData(elementLeft, x-1, y);
						UpdateChunkDirtyRect(x, y, chunk);
						continue;
					}

					it = m_ReactionLookup[currElement.m_ID].find(elementTop->m_ID);
					end = m_ReactionLookup[currElement.m_ID].end();
					if (it != end && (std::rand() % 101) <= it->second.probability)
					{
						currElement.m_ID = it->second.cell0ID;
						m_ElementData[it->second.cell0ID].UpdateElementData(currElement, x, y);
						elementTop->m_ID = it->second.cell1ID;
						m_ElementData[it->second.cell1ID].UpdateElementData(elementTop, x, y+1);
						UpdateChunkDirtyRect(x, y, chunk);
						continue;
					}

					it = m_ReactionLookup[currElement.m_ID].find(elementRight->m_ID);
					end = m_ReactionLookup[currElement.m_ID].end();
					if (it != end && (std::rand() % 101) <= it->second.probability)
					{
						currElement.m_ID = it->second.cell0ID;
						m_ElementData[it->second.cell0ID].UpdateElementData(currElement, x, y);
						elementRight->m_ID = it->second.cell1ID;
						m_ElementData[it->second.cell1ID].UpdateElementData(elementRight, x+1, y);
						UpdateChunkDirtyRect(x, y, chunk);
						continue;
					}

					it = m_ReactionLookup[currElement.m_ID].find(elementBottom->m_ID);
					end = m_ReactionLookup[currElement.m_ID].end();
					if (it != end && (std::rand() % 101) <= it->second.probability)
					{
						currElement.m_ID = it->second.cell0ID;
						m_ElementData[it->second.cell0ID].UpdateElementData(currElement, x, y);
						elementBottom->m_ID = it->second.cell1ID;
						m_ElementData[it->second.cell1ID].UpdateElementData(elementBottom, x, y-1);
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
						m_ElementData[newID].UpdateElementData(currElement, x, y);
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
						m_ElementData[newID].UpdateElementData(currElement, x, y);
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
						currElement.m_Temperature -= diff / currElementData.density;
						elementLeft->m_Temperature += diff / elementLeftData.density;
					}

					minConductivity = std::min(currElementData.conductivity, m_ElementData[elementTop->m_ID].conductivity);
					diff = ((float)(currElement.m_Temperature - elementTop->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
					if (diff != 0)
					{
						currElement.m_Temperature -= diff / currElementData.density;
						elementTop->m_Temperature += diff / elementTopData.density;
					}

					minConductivity = std::min(currElementData.conductivity, m_ElementData[elementRight->m_ID].conductivity);
					diff = ((float)(currElement.m_Temperature - elementRight->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
					if (diff != 0)
					{
						currElement.m_Temperature -= diff / currElementData.density;
						elementRight->m_Temperature += diff / elementRightData.density;
					}

					minConductivity = std::min(currElementData.conductivity, m_ElementData[elementBottom->m_ID].conductivity);
					diff = ((float)(currElement.m_Temperature - elementBottom->m_Temperature) * ((float)minConductivity / 100.0f)) / 2;
					if (diff != 0)
					{
						currElement.m_Temperature -= diff / currElementData.density;
						elementBottom->m_Temperature += diff / elementBottomData.density;
					}
					if (std::abs(currElement.m_Temperature - tempBefore) > 0.01f) chunk->UpdateDirtyRect(x, y);
					

				}

				if (currElementData.flammable)
				{
					if (currElement.m_Temperature < currElementData.ignition_temperature && !currElementData.spread_ignition) currElement.m_Ignited = false;
					if (currElement.m_Temperature >= currElementData.ignition_temperature) currElement.m_Ignited = true;
					if (currElement.m_Ignited)
					{
						//try to spread ignition to surrounding elements
						//if (elementTopData.flammable) 
						if (currElementData.spread_ignition && std::rand() % 100 < currElementData.spread_ignition_chance)
						{
							if (elementLeftData.flammable) elementLeft->m_Ignited = true;
							if (elementTopData.flammable) elementTop->m_Ignited = true;
							if (elementRightData.flammable) elementRight->m_Ignited = true;
							if (elementBottomData.flammable) elementBottom->m_Ignited = true;
						}

						//check for open air to burn
						int fireID = m_ElementIDs["fire"];
						ElementData& fireElementData = m_ElementData[fireID];
						if (currElement.m_ID != fireID) //&& std::rand() % 101 < 5
						{

							int healthDiff = currElement.m_Health;
							if (elementLeftData.cell_type == ElementType::gas || elementLeftData.cell_type == ElementType::fire)
							{
								fireElementData.UpdateElementData(elementLeft, x-1, y);
								elementLeft->m_ID = fireID;
								elementLeft->m_Temperature = currElementData.fire_temperature;
								elementLeft->m_BaseColor = currElementData.fire_color;
								elementLeft->m_Color = currElementData.fire_color;
								currElement.m_Health--;
								if (currElement.m_Temperature < currElementData.fire_temperature - currElementData.fire_temperature_increase) currElement.m_Temperature += currElementData.fire_temperature_increase;
							}
							
							if (elementTopData.cell_type == ElementType::gas || elementTopData.cell_type == ElementType::fire)
							{
								fireElementData.UpdateElementData(elementTop, x, y+1);
								elementTop->m_ID = fireID;
								elementTop->m_Temperature = currElementData.fire_temperature;
								elementTop->m_BaseColor = currElementData.fire_color;
								elementTop->m_Color = currElementData.fire_color;
								currElement.m_Health--;
								if (currElement.m_Temperature < currElementData.fire_temperature - currElementData.fire_temperature_increase) currElement.m_Temperature += currElementData.fire_temperature_increase;
							}
							
							if (elementRightData.cell_type == ElementType::gas || elementRightData.cell_type == ElementType::fire)
							{
								fireElementData.UpdateElementData(elementRight, x+1, y);
								elementRight->m_ID = fireID;
								elementRight->m_Temperature = currElementData.fire_temperature;
								elementRight->m_BaseColor = currElementData.fire_color;
								elementRight->m_Color = currElementData.fire_color;
								currElement.m_Health--;
								if (currElement.m_Temperature < currElementData.fire_temperature - currElementData.fire_temperature_increase) currElement.m_Temperature += currElementData.fire_temperature_increase;
							}
							
							if (elementBottomData.cell_type == ElementType::gas || elementBottomData.cell_type == ElementType::fire)
							{
								fireElementData.UpdateElementData(elementBottom, x, y-1);
								elementBottom->m_ID = fireID;
								elementBottom->m_Temperature = currElementData.fire_temperature;
								elementBottom->m_BaseColor = currElementData.fire_color;
								elementBottom->m_Color = currElementData.fire_color;
								currElement.m_Health--;
								if (currElement.m_Temperature < currElementData.fire_temperature - currElementData.fire_temperature_increase) currElement.m_Temperature += currElementData.fire_temperature_increase;
							}
							if (currElement.m_Health != healthDiff)
							{
								currElement.m_Ignited = true;
							}
							else currElement.m_Ignited = false;
						}
					}
					
					
					if (currElement.m_Ignited)
					{
						if (currElement.m_Health <= 0)
						{
							currElement.m_ID = m_ElementIDs[currElementData.burnt];
							int temp = currElement.m_Temperature;
							m_ElementData[currElement.m_ID].UpdateElementData(currElement, x, y);
							currElement.m_Temperature = temp;
							continue;
						}
					}
					
					
				}


				//update the texture of the element based on temp / glow / ect
				uint32_t EditedBaseColor = currElement.m_BaseColor;
				if (currElement.m_Ignited)
				{
					//update color to reflect being on fire
					if (currElementData.ignited_color != 0)
						EditedBaseColor = RandomizeABGRColor(currElementData.ignited_color, 5);
				}

				if (currElementData.glow)
				{
					int r = (EditedBaseColor >> 0) & 255;
					int g = (EditedBaseColor >> 8) & 255;
					int b = (EditedBaseColor >> 16) & 255;
					int a = EditedBaseColor & 0xff000000;
					r = std::max(r, (int)(Pyxis::interpolateBetweenValues(460, 900, currElement.m_Temperature) * 255.0f));
					g = std::max(g, (int)(Pyxis::interpolateBetweenValues(460, 1500, currElement.m_Temperature) * 255.0f));
					b = std::max(b, (int)(Pyxis::interpolateBetweenValues(1000, 6000, currElement.m_Temperature) * 255.0f));
					EditedBaseColor = a | ((b & 255) << 16) | ((g & 255) << 8) | (r & 255);
					//make it appear hotter depending on temp
					//temp range from 460 to 6000
				}

				currElement.m_Color = EditedBaseColor;


				//switch the behavior based on element type
				switch (currElementData.cell_type)
				{
				case ElementType::solid:
					break;
				case ElementType::movableSolid:

					if (currElement.m_Rigid) continue;
					//check below, and move
					{
						ElementData& otherData = m_ElementData[elementBottom->m_ID];
						if (otherData.cell_type != ElementType::solid && otherData.cell_type != ElementType::movableSolid && otherData.density <= currElementData.density)
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
						if (otherData.cell_type != ElementType::solid && otherData.cell_type != ElementType::movableSolid && otherData.density <= currElementData.density)
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
						if (otherData.cell_type != ElementType::solid && otherData.cell_type != ElementType::movableSolid && otherData.density <= currElementData.density)
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
					//temp drop quickly
					//currElement.m_Temperature *= 0.95f;
					currElement.m_Health--;
					//die out if cold
					if (currElement.m_Temperature < currElementData.ignition_temperature)
					{
						currElement.m_ID = 0;//air
						m_ElementData[0].UpdateElementData(currElement, x, y);
						continue;
					}
					//fire gets special color treatment, basically going from starting color, and
					//losing its b, g, r color values at slower rates, respectively based on health
					//get rgb values, and diminish them onto the color
					
					uint32_t colorRed   = (currElement.m_BaseColor & 0x000000FF) >> 0;
					uint32_t colorGreen = (currElement.m_BaseColor & 0x0000FF00) >> 8;
					uint32_t colorBlue = (currElement.m_BaseColor & 0x00FF0000) >> 16;
					uint32_t colorAlpha  = currElement.m_BaseColor & 0xFF000000;

					float percentAlive = (float)currElement.m_Health / (float)currElementData.health;
					colorRed *= Pyxis::interpolateBetweenValues(-10, 20, currElement.m_Health);
					colorGreen *= Pyxis::interpolateBetweenValues(0, 20, currElement.m_Health);
					colorBlue *= Pyxis::interpolateBetweenValues(5, 20, currElement.m_Health);
					colorAlpha *= Pyxis::interpolateBetweenValues(0, 20, currElement.m_Health);

					currElement.m_Color = colorAlpha | (colorBlue << 16) | (colorGreen << 8) | colorRed;
					
					r = (std::rand() % 101);
					if (r > 20 && r < 80) //60% to go up
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
							else if (otherData.cell_type == ElementType::fire)
							{
								//moving to fire, so combine temp and leave air
								elementTop->m_Temperature = (elementTop->m_Temperature + currElement.m_Temperature) / 2;
								currElement.m_ID = 0;
								m_ElementData[0].UpdateElementData(currElement, x, y);
							}
						}
					}
					else if (r > 50) // left / right
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
						else if (otherData.cell_type == ElementType::fire)
						{
							//moving to fire, so combine temp and leave air
							elementRight->m_Temperature = (elementRight->m_Temperature + currElement.m_Temperature) / 2;
							currElement.m_ID = 0;
							m_ElementData[0].UpdateElementData(currElement, x, y);
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
							else if (otherData.cell_type == ElementType::fire)
							{
								//moving to fire, so combine temp and leave air
								elementLeft->m_Temperature = (elementLeft->m_Temperature + currElement.m_Temperature) / 2;
								currElement.m_ID = 0;
								m_ElementData[0].UpdateElementData(currElement, x, y);
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
						else if (otherData.cell_type == ElementType::fire)
						{
							//moving to fire, so combine temp and leave air
							elementLeft->m_Temperature = (elementLeft->m_Temperature + currElement.m_Temperature) / 2;
							currElement.m_ID = 0;
							m_ElementData[0].UpdateElementData(currElement, x, y);
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
							else if (otherData.cell_type == ElementType::fire)
							{
								//moving to fire, so combine temp and leave air
								elementRight->m_Temperature = (elementRight->m_Temperature + currElement.m_Temperature) / 2;
								currElement.m_ID = 0;
								m_ElementData[0].UpdateElementData(currElement, x, y);
							}
						}
					}

					break;
				}

			}
		}
	}


	/// <summary>
	/// updated the chunks dirty rect, and will spread the
	/// dirty rect to neighboring ones if it is touching the edge
	/// </summary>
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
	/// wipes the world, and makes the first chunk empty
	/// </summary>
	void World::Clear()
	{
		for each (auto& pair in m_Chunks)
		{
			delete pair.second;
		}
		m_Chunks.clear();

		m_Box2DWorld->~b2World();
		for each (auto pair in m_PixelBodyMap)
		{
			delete pair.second;
		}
		m_PixelBodyMap.clear();

		m_Box2DWorld = new b2World({ 0, -9.8f });

		//AddChunk(glm::ivec2(0, 0));
		//m_Chunks[{0, 0}]->Clear();

		////create a border around first chunk
		//Element ceramic = Element();
		//ceramic.m_ID = m_ElementIDs["ceramic"];
		//ElementData& elementData = m_ElementData[ceramic.m_ID];
		//elementData.UpdateElementData(ceramic, 0, 0);
		//for (int i = 0; i < CHUNKSIZE; i++)
		//{
		//	SetElement({ i, 0 }, ceramic);//bottom
		//	SetElement({ i, CHUNKSIZE - 1 }, ceramic);//top
		//	SetElement({ 0, i }, ceramic);//left
		//	SetElement({ CHUNKSIZE - 1, i }, ceramic);//right
		//}
	}

	/// <summary>
	/// Renders the world using Pyxis::renderer2d draw quad
	/// </summary>
	void World::RenderWorld()
	{

		//PX_TRACE("Rendering world");
		for each (auto pair in m_Chunks)
		{
			//PX_TRACE("Drawing chunk {0}, {1}", pair.second->m_ChunkPos.x, pair.second->m_ChunkPos.y);
			Renderer2D::DrawQuad(glm::vec2(pair.second->m_ChunkPos.x + 0.5f, pair.second->m_ChunkPos.y + 0.5f), { 1,1 }, pair.second->m_Texture);

			//Renderer2D::DrawQuad(glm::vec3(pair.second->m_ChunkPos.x + 0.5f, pair.second->m_ChunkPos.y + 0.5f, 1.0f), {0.1f, 0.1f}, glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
		}

		//float pixelSize = (1.0f / CHUNKSIZE);
		if (m_DebugDrawColliders)
		{
			//drawing contour vector
			/*for each (auto pixelBody in m_PixelBodies)
			{
				for (int i = 0; i < pixelBody->m_ContourVector.size() - 1; i++)
				{
					glm::ivec2 pixelPos = { pixelBody->m_B2Body->GetPosition().x * PPU , pixelBody->m_B2Body->GetPosition().y * PPU };
					pixelPos -= pixelBody->m_Origin;
					pixelPos.y += 1;
					glm::vec2 worldPos = { pixelPos.x / (float)CHUNKSIZE, pixelPos.y / (float)CHUNKSIZE };
					glm::vec2 start = (glm::vec2(pixelBody->m_ContourVector[i].x, pixelBody->m_ContourVector[i].y) / 512.0f) + worldPos;
					glm::vec2 end = (glm::vec2(pixelBody->m_ContourVector[i + 1].x, pixelBody->m_ContourVector[i + 1].y) / 512.0f) + worldPos;
					Renderer2D::DrawLine(start, end, { 0,1,0,1 });
				}
			}*/

			auto count = m_Box2DWorld->GetBodyCount();
			for (auto body = m_Box2DWorld->GetBodyList(); body != nullptr; body = body->GetNext())
			{
				auto& T = body->GetTransform();
				for (auto fixture = body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext())
				{
					auto shape = (b2PolygonShape*)(fixture->GetShape());
					for (int i = 0; i < shape->m_count - 1; i++)
					{
						auto v = shape->m_vertices[i];
						float x1 = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
						float y1 = (T.q.s * v.x + T.q.c * v.y) + T.p.y;

						auto e = shape->m_vertices[i + 1];
						float x2 = (T.q.c * e.x - T.q.s * e.y) + T.p.x;
						float y2 = (T.q.s * e.x + T.q.c * e.y) + T.p.y;
						glm::vec2 start = glm::vec2(x1, y1) / (PPU * 2.0f);
						glm::vec2 end = glm::vec2(x2, y2) / (PPU * 2.0f);

						Renderer2D::DrawLine(start, end);
					}
					//draw the last line to connect the shape
					auto v = shape->m_vertices[shape->m_count - 1];
					float x1 = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
					float y1 = (T.q.s * v.x + T.q.c * v.y) + T.p.y;

					auto e = shape->m_vertices[0];
					float x2 = (T.q.c * e.x - T.q.s * e.y) + T.p.x;
					float y2 = (T.q.s * e.x + T.q.c * e.y) + T.p.y;
					glm::vec2 start = glm::vec2(x1, y1) / (PPU * 2.0f);
					glm::vec2 end = glm::vec2(x2, y2) / (PPU * 2.0f);

					Renderer2D::DrawLine(start, end);
				}
			}
		}

	}


	void World::ResetBox2D()
	{
		PX_TRACE("Box2D sim reset at sim tick {0}", m_SimulationTick);
		//struct to hold the data of the b2 bodies
		std::unordered_map<uint64_t, PixelBodyData> storage;
		for each (auto pair in m_PixelBodyMap)
		{
			//store the data to re-apply later
			PixelBodyData data;
			data.linearVelocity = pair.second->m_B2Body->GetLinearVelocity();
			data.angularVelocity = pair.second->m_B2Body->GetAngularVelocity();
			data.angle = pair.second->m_B2Body->GetAngle();
			data.position = pair.second->m_B2Body->GetPosition();
			//data.elementArray = pair.second->m_ElementArray;
			storage[pair.first] = data;
			//delete each b2body
			m_Box2DWorld->DestroyBody(pair.second->m_B2Body);
		}
		//in theory this should delete the bodies so the above
		//step is unnecessary but it is nicer like this
		m_Box2DWorld->~b2World();
		m_Box2DWorld = new b2World({ 0, -9.8f });
		for each (auto pair in m_PixelBodyMap)
		{
			//recreate the box2d body for each rigid body!
			pair.second->CreateB2Body(m_Box2DWorld);

			PixelBodyData& data = storage[pair.first];
			pair.second->m_B2Body->SetTransform(data.position, data.angle);
			pair.second->m_B2Body->SetLinearVelocity(data.linearVelocity);
			pair.second->m_B2Body->SetAngularVelocity(data.angularVelocity);
		}
	}

	/// <summary>
	/// Creates a pixel rigid body, but it is not placed in the world!
	/// you have to call PutPixelBodyInWorld after setting the pixel bodies
	/// position
	/// </summary>
	//PixelRigidBody* World::CreatePixelRigidBody(uint64_t uuid, const glm::ivec2& size, Element* ElementArray, b2BodyType type)
	//{
	//	PixelRigidBody* body = new PixelRigidBody();
	//	body->m_Width = size.x;
	//	body->m_Height = size.y;
	//	body->m_Origin = { body->m_Width / 2, body->m_Height / 2 };
	//	body->m_ElementArray = ElementArray;
	//	
	//	//create the base body of the whole pixel body
	//	b2BodyDef pixelBodyDef;
	//	
	//	//for the scaling of the box world and pixel bodies, every PPU pixels is 1 unit in the world space
	//	pixelBodyDef.position = {0,0};
	//	pixelBodyDef.type = type;
	//	body->m_B2Body = m_Box2DWorld->CreateBody(&pixelBodyDef);


	//	//BUG ERROR FIX TODO WRONG BROKEN
	//	// getcontour points is able to have repeating points, which is a no-no for box2d triangles / triangulation
	//	auto contour = body->GetContourPoints();
	//	if (contour.size() == 0)
	//	{
	//		m_Box2DWorld->DestroyBody(body->m_B2Body);
	//		delete body;
	//		return nullptr;
	//	}

	//	auto contourVector = body->SimplifyPoints(contour, 0, contour.size() - 1, 1.0f);
	//	//auto simplified = body->SimplifyPoints(contour);

	//	//run triangulation algorithm to create the needed triangles/fixtures
	//	std::vector<p2t::Point*> polyLine;
	//	for each (auto point in contourVector)
	//	{
	//		polyLine.push_back(new p2t::Point(point));
	//	}

	//	//create each of the triangles to comprise the body, each being a fixture
	//	p2t::CDT* cdt = new p2t::CDT(polyLine);
	//	cdt->Triangulate();
	//	auto triangles = cdt->GetTriangles();
	//	for each (auto triangle in triangles)
	//	{
	//		b2PolygonShape triangleShape;
	//		b2Vec2 points[3] = {
	//			{(float)((triangle->GetPoint(0)->x - body->m_Origin.x) / PPU), (float)((triangle->GetPoint(0)->y - body->m_Origin.y) / PPU)},
	//			{(float)((triangle->GetPoint(1)->x - body->m_Origin.x) / PPU), (float)((triangle->GetPoint(1)->y - body->m_Origin.y) / PPU)},
	//			{(float)((triangle->GetPoint(2)->x - body->m_Origin.x) / PPU), (float)((triangle->GetPoint(2)->y - body->m_Origin.y) / PPU)}
	//		};
	//		triangleShape.Set(points, 3);
	//		b2FixtureDef fixtureDef;
	//		fixtureDef.density = 1;
	//		fixtureDef.friction = 0.3f;
	//		fixtureDef.shape = &triangleShape;
	//		body->m_B2Body->CreateFixture(&fixtureDef);
	//	}
	//	PX_TRACE("Pixel body created with ID: {0}", uuid);
	//	m_PixelBodyMap.insert({ uuid, body });
	//	return body;
	//}

	void World::PutPixelBodyInWorld(PixelRigidBody& body)
	{
		glm::ivec2 pixelPosition = glm::ivec2(body.m_B2Body->GetPosition().x * PPU, body.m_B2Body->GetPosition().y * PPU);
		//PX_TRACE("pixel body put in world at: ({0},{1})", pixelPosition.x, pixelPosition.y);
		for each (auto mappedElement in body.m_Elements)
		{
			SetElement(mappedElement.second.worldPos, mappedElement.second.element);
		}
		body.m_InWorld = true;
	}


	void World::HandleTickClosure(MergedTickClosure& tc)
	{
		//PX_TRACE("tick closure {0} applied to simulation tick {1}", tc.m_Tick, m_SimulationTick);
		for (int i = 0; i < tc.m_InputActionCount; i++)
		{
			InputAction IA;
			tc >> IA;
			switch (IA)
			{
			case InputAction::Add_Player:
			{
				uint64_t ID;
				tc >> ID;

				glm::ivec2 pixelPos;
				tc >> pixelPos;

				CreatePlayer(ID, pixelPos);
				break;
			}
			case InputAction::PauseGame:
			{
				uint64_t ID;
				tc >> ID;
				m_Running = false;
				break;
			}
			case InputAction::ResumeGame:
			{
				uint64_t ID;
				tc >> ID;
				m_Running = true;
				break;
			}
			case InputAction::TransformRegionToRigidBody:
			{
				uint64_t ID;
				tc >> ID;

				glm::ivec2 maximum;
				tc >> maximum;

				glm::ivec2 minimum;
				tc >> minimum;

				b2BodyType type;
				tc >> type;

				int width = (maximum.x - minimum.x) + 1;
				int height = (maximum.y - minimum.y) + 1;
				if (width * height <= 0) break;
				glm::ivec2 newMin = maximum;
				glm::ivec2 newMax = minimum;
				//iterate over section and find the width, height, center, ect
				int mass = 0;
				for (int x = 0; x < width; x++)
				{
					for (int y = 0; y < height; y++)
					{
						glm::ivec2 pixelPos = glm::ivec2(x + minimum.x, y + minimum.y);
						auto& element = GetElement(pixelPos);
						auto& elementData = m_ElementData[element.m_ID];
						if ((elementData.cell_type == ElementType::solid || elementData.cell_type == ElementType::movableSolid) && element.m_Rigid == false)
						{
							if (pixelPos.x < newMin.x) newMin.x = pixelPos.x;
							if (pixelPos.y < newMin.y) newMin.y = pixelPos.y;
							if (pixelPos.x > newMax.x) newMax.x = pixelPos.x;
							if (pixelPos.y > newMax.y) newMax.y = pixelPos.y;
							mass++;
						}
					}
				}
				if (mass < 2) continue;//skip if we are 1 element or 0
				PX_TRACE("transforming {0} elements to a rigid body", mass);
				
				width = (newMax.x - newMin.x) + 1;
				height = (newMax.y - newMin.y) + 1;

				glm::ivec2 origin = { width / 2, height / 2 };
				std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements;
				for (int x = 0; x < width; x++)
				{
					for (int y = 0; y < height; y++)
					{
						glm::ivec2 pixelPos = { x + newMin.x, y + newMin.y };

						//loop over every element, grab it, and make it rigid if it is a movable Solid
						auto& element = GetElement(pixelPos);
						auto& elementData = m_ElementData[element.m_ID];
						if ((elementData.cell_type == ElementType::solid || elementData.cell_type == ElementType::movableSolid) && element.m_Rigid == false)
						{
							element.m_Rigid = true;
							//set the elements at the local position to be the element pulled from world
							elements[glm::ivec2(x - origin.x, y - origin.y)] = RigidBodyElement(element, pixelPos);
							SetElement(pixelPos, Element());
						}
					}
				}
				glm::ivec2 size = newMax - newMin;
				PX_TRACE("Mass is: {0}", mass);
				PixelRigidBody* body = new PixelRigidBody(ID, size, elements, type, m_Box2DWorld);
				if (body->m_B2Body == nullptr) 
				{
					PX_TRACE("Failed to create rigid body");
					continue;
				}
				else
				{
					m_PixelBodyMap[body->m_ID] = body;
					auto pixelPos = (newMin + newMax) / 2;
					if (width % 2 == 0) pixelPos.x += 1;
					if (height % 2 == 0) pixelPos.y += 1;
					body->SetPixelPosition(pixelPos);
					PutPixelBodyInWorld(*body);
				}
					
				
				break;
			}
			case Pyxis::InputAction::Input_Move:
			{
				//PX_TRACE("input action: Input_Move");
				break;
			}
			case Pyxis::InputAction::Input_Place:
			{
				//PX_TRACE("input action: Input_Place");
				uint64_t id;
				tc >> id;
				glm::ivec2 pixelPos;
				tc >> pixelPos;
				Element element;
				tc >> element;
				SetElement(pixelPos, element);
				//PX_INFO("pixel pos: ({0},{1})", pixelPos.x, pixelPos.y);
				break;
			}
			case Pyxis::InputAction::Input_StepSimulation:
			{
				PX_TRACE("input action: Input_StepSimulation");
				UpdateWorld();
				break;
			}
			case InputAction::Input_MousePosition:
			{
				glm::ivec2 mousePos;
				uint64_t ID;
				tc >> mousePos >> ID;
				HashVector hasher;
				uint32_t uintColor = hasher(mousePos);
				glm::vec4 color = { (uintColor >> 24) / 255.0f, (uintColor >> 16) / 255.0f , (uintColor >> 8) / 255.0f, 1 };
				glm::ivec2 size = glm::ivec2(2.0f / CHUNKSIZE);
				if (!m_ServerMode)
				{
					Renderer2D::DrawQuad(glm::vec3(mousePos.x / CHUNKSIZE, mousePos.y / CHUNKSIZE, 2), size, color);
				}
				break;
			}
			case InputAction::ClearWorld:
			{
				Clear();
				break;
			}
			default:
			{
				PX_TRACE("input action: default?");
				break;
			}
			}
		}
		if (m_Running)
		{
			UpdateWorld();
		}
		else
		{
			UpdateTextures();
		}
	}

	Player* World::CreatePlayer(uint64_t playerID, glm::ivec2 pixelPosition)
	{
		std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements;
		for (int x = 0; x < 10; x++)
		{
			for (int y = 0; y < 20; y++)
			{
				Element flesh = GetElementByName("flesh", x, y);
				flesh.m_Rigid = true;
				auto localPos = glm::ivec2(x - 5, y - 10);
				elements[localPos] = RigidBodyElement(flesh, localPos + pixelPosition);
			}
		}
		Player* player = new Player(playerID, glm::ivec2(10,20), elements, b2_dynamicBody, m_Box2DWorld);
		player->SetPixelPosition(pixelPosition);
		PutPixelBodyInWorld(*player);
		return player;
	}


	/// <summary>
	/// Seeds Rand() based on a few factors of the world.
	/// It is deterministic by update tick and position, so
	/// it is thread safe.
	/// </summary>
	void World::SeedRandom(int xPos, int yPos)
	{
		unsigned int seed = ((xPos * 58102) << m_SimulationTick % 5)
			+ (((yPos * 986124) * m_SimulationTick) >> 2);
		std::srand(seed);
		//PX_TRACE("Seeded rand with: {0}", seed);
	}

	const bool World::IsInBounds(int x, int y)
	{
		//having y first might actually be faster, simply because things tend to fall
		if (y < 0 || y >= CHUNKSIZE) return false;
		if (x < 0 || x >= CHUNKSIZE) return false;
		return true;
	}

	glm::ivec2 World::WorldToPixel(const glm::vec2& worldPos)
	{
		glm::ivec2 result = glm::ivec2(worldPos.x * CHUNKSIZE, worldPos.y * CHUNKSIZE);
		if (worldPos.x < 0) result.x--;
		if (worldPos.y < 0) result.y--;
		return result;
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
			result.x = result.x % CHUNKSIZE;
		}
		else result.x = pixelPos.x % CHUNKSIZE;
		if (pixelPos.y < 0)
		{
			result.y = CHUNKSIZE - (std::abs(pixelPos.y) % CHUNKSIZE);
			result.y = result.y % CHUNKSIZE;
		}
		else result.y = pixelPos.y % CHUNKSIZE;
		return result;
	}

}

