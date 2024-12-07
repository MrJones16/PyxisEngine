#pragma once

#include <Pyxis/Network/NetworkMessage.h>
/// <summary>
/// Classes / Enums for serializing game data & logic
/// to be sent using the client & server interfaces.
/// </summary>

namespace Pyxis
{
	struct ClientData
	{
		char m_Name[64] = "PyxisEnjoyer";
		glm::ivec2 m_CurosrPixelPosition = { 0,0 };
		glm::vec4 m_Color = { 1,1,1,1 };


		ClientData()
		{
			std::vector<glm::vec4> colorOptions =
			{
				glm::vec4(1,0,0,1),//red
				glm::vec4(1,0.5f,0,1),//orange
				glm::vec4(1,1,0,1),//yellow
				glm::vec4(0,1,0,1),//green
				glm::vec4(0,0,1,1),//blue
				glm::vec4(1,0,0.2f,1),//indigo
				glm::vec4(1,0,0.8f,1),//violet
			};
			m_Color = colorOptions[std::rand() % colorOptions.size()];
		}

	};

	enum class GameMessage : uint32_t
	{
		String,
		StringToAll,

		Client_ClientData,
		Client_RequestMergedTick,
		Client_RequestAllClientData,
		Client_RequestGameData,

		Server_ClientData,
		Server_AllClientData,
		Server_ClientDisconnected,
		Server_GameData,

		Server_Message,
		Message_All,

		Game_ResetBox2D,
		Game_Loaded,
		Game_TickToEnter,
		Game_TickClosure,
		Game_MergedTickClosure,

	};

	enum class InputAction : uint32_t
	{
		Add_Player,
		Remove_Player,
		PauseGame, ResumeGame,
		ClearWorld,
		TransformRegionToRigidBody,
		Input_Move,
		Input_Place,
		Input_StepSimulation,
		Input_MousePosition,

	};

	enum BrushType : uint16_t {
		circle = 0, square = 1, end
	};

	//class InputActionData : 

	class TickClosure
	{
	public:
		//stored first, all the data for an action, then what kind of action it is
		std::vector<uint8_t> m_Data;
		//the number of input actions to apply
		uint32_t m_InputActionCount = 0;

	public:
		void AddInputAction(InputAction action)
		{
			*(this) << action;
			m_InputActionCount++;
		}

		template<typename... Arguments>
		void AddInputAction(InputAction action, Arguments ... args)
		{
			AddInputActionData(args...);
			*(this) << action;
			m_InputActionCount++;
		}

		/// <summary>
		/// Pushes any "POD" like data into the message buffer
		/// </summary>
		template<typename DataType>
		friend TickClosure& operator << (TickClosure& tc, const DataType& data)
		{
			//check that the type of the data being pushed is trivially copyable
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into message body vector");

			//cache current vector size, as this is where we will add the data
			size_t i = tc.m_Data.size();

			//resize the vector by the size of the data being pushed
			tc.m_Data.resize(tc.m_Data.size() + sizeof(DataType));

			//physically copy the data into the newly allocated vector space
			std::memcpy(tc.m_Data.data() + i, &data, sizeof(DataType));

			//return message so it can be "chained"
			return tc;
		}

		template<typename DataType>
		friend TickClosure& operator >> (TickClosure& tc, DataType& data)
		{
			//check that the data is easily copyable
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to have been in a body vector");

			//cache the location towards the end of the vector where the pulled data starts
			size_t dataSize = sizeof(DataType);
			size_t i = tc.m_Data.size() - dataSize;

			//physically copy the memory to the DataType variable
			std::memcpy(&data, tc.m_Data.data() + i, sizeof(DataType));

			//shrink the vector;
			tc.m_Data.resize(i);

			//return the target message so it can be "chained"
			return tc;
		}

	private:
		template<typename DataType>
		void AddInputActionData(DataType data)
		{
			*(this) << data;
		}

		template<typename DataType, typename... Arguments>
		void AddInputActionData(DataType data, Arguments ... args)
		{
			*(this) << data;
			AddInputActionData(args...);
		}

	};

	class MergedTickClosure : public TickClosure
	{
	public:
		//the "heartbeat" tick this action should occur
		uint64_t m_Tick = 0;

		std::unordered_set<uint64_t> m_Clients;
		uint32_t m_ClientCount = 0;
		inline void AddTickClosure(TickClosure& tickClosure, HSteamNetConnection clientID)
		{
			
			size_t i = m_Data.size(); // get current end point of data
			m_Data.resize(m_Data.size() + tickClosure.m_Data.size());
			memcpy(m_Data.data() + i, tickClosure.m_Data.data(), tickClosure.m_Data.size());
			*(this) << tickClosure.m_InputActionCount;
			*(this) << clientID;
			//keep track of clients / closure count for later
			m_ClientCount++;
			m_Clients.insert(clientID);
		}
	};
}