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
			/// 
			/// Not Volatile
			/// </summary>
			template<typename DataType>
			friend Message<T>& operator << (Message<T>& msg, const DataType& data)
			{
				//check that the type of the data being pushed is trivially copyable
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

			/// <summary>
			/// Pushes any "POD" storing vector into the message in reverse
			/// </summary>
			template<typename DataType>
			friend Message<T>& operator << (Message<T>& msg, const std::vector<DataType>& data)
			{
				//check that the data is easily copyable
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into message body vector");

				for (auto it = data.rbegin(); it != data.rend(); it++)
				{
					msg << *it;
				}
				msg << uint32_t(data.size());

				return msg;
			}


			/// <summary>
			/// Pulls data out of the message. Volatile.
			/// </summary>
			template<typename DataType>
			friend Message<T>& operator >> (Message<T>& msg, DataType& data)
			{
				//check that the data is easily copyable
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

			/// <summary>
			/// Pulls vector data out of the message. Volatile.
			/// </summary>
			template<typename DataType>
			friend Message<T>& operator >> (Message<T>& msg, std::vector<DataType>& data)
			{
				//check that the data is easily copyable
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to have been in a body vector");

				//make sure we empty the array first, as we are supposed to be replacing it... or maybe not?
				data.clear();

				//get the amount of items to extract
				uint32_t itemCount = 0;
				msg >> itemCount;

				//loop over that many elements and pull them out
				for (int i = 0; i < itemCount; i++)
				{
					DataType dt;
					msg >> dt;
					data.push_back(dt);
				}

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