#pragma once

#include "NetworkCommon.h"

//following guidance on asio from OLC "javix9"

namespace Pyxis
{
	namespace Network
	{
		template<typename T>
		struct MessageHeader
		{
			T id{};
			uint32_t size = 0;
		};

		template<typename T>
		struct Message
		{
			MessageHeader<T> header{};
			std::vector<uint8_t> body;

			/// <summary>
			/// returns the size of the entire message packet in bytes
			/// </summary>
			size_t size() const
			{
				return body.size();
			}

			/// <summary>
			/// override for std::cout compatibility -- produces friendly description of message
			/// </summary>
			friend std::ostream& operator << (std::ostream& os, const Message<T>& msg)
			{
				os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
				return os;
			}

			/// <summary>
			/// Pushes any "POD" like data into the message buffer
			/// </summary>
			template<typename DataType>
			friend Message<T>& operator << (Message<T>& msg, const DataType& data)
			{
				//chack that the type of the data being pushed is trivially copyable
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into message body vector");

				//cache current vector size, as this is where we will add the data
				size_t i = msg.body.size();

				//resize the vector by the size of the data being pushed
				msg.body.resize(msg.body.size() + sizeof(DataType));

				//physically copy the data into the newly allocated vector space
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

				//recalculate the message size
				msg.header.size = msg.size();

				//return message so it can be "chained"
				return msg;
			}

			template<typename DataType>
			friend Message<T>& operator >> (Message<T>& msg, DataType& data)
			{
				//chack that the data is easily copyable
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to have been in a body vector");

				//cache the location towards the end of the vector where the pulled data starts
				size_t dataSize = sizeof(DataType);
				size_t i = msg.body.size() - dataSize;

				//physically copy the memory to the DataType variable
				std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

				//shrink the vector;
				msg.body.resize(i);
				 
				//recalculate message size
				msg.header.size = msg.size();

				//return the target message so it can be "chained"
				return msg;
			}

		};

		template <typename T>
		class Connection;

		template<typename T>
		struct OwnedMessage
		{
			std::shared_ptr<Connection<T>> remote = nullptr;
			Message<T> msg;

			/// <summary>
			/// override for std::cout compatibility -- produces friendly description of message -- again for 
			/// </summary>
			friend std::ostream& operator << (std::ostream& os, const OwnedMessage<T>& msg)
			{
				os << msg.msg;
				return os;
			}
		};
	}
}