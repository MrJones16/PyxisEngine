#pragma once

#include "NetworkCommon.h"
#include "NetworkMessage.h"
#include "NetworkThreadSafeQueue.h"

namespace Pyxis
{
	namespace Network
	{
		//forward declare server pointer

		template<typename T>
		class ServerInterface;

		template<typename T>
		class Connection : public std::enable_shared_from_this<Connection<T>>
		{
		public:
			enum class Owner
			{
				server, client,
			};
			Connection(Owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, ThreadSafeQueue<OwnedMessage<T>>& qIn)
				: m_AsioContext(asioContext), m_Socket(std::move(socket)), m_QueueMessagesIn(qIn)
			{
				m_OwnerType = parent;
				if (m_OwnerType == Owner::server)
				{
					//connection is Server -> Client, construct random data for the client
					// to transform and send back for validation
					m_HandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

					//precalc the result for checking when the client responds
					m_HandshakeCheck = scramble(m_HandshakeOut);
				}
				else
				{
					//owner is Clienter -> Server, so we have nothing to define
					m_HandshakeIn = 0;
					m_HandshakeOut = 0;
				}
			}

			~Connection() 
			{

			}

			uint64_t GetID()
			{
				return m_ID;
			}

		public:

			inline void ConnectToClient(Pyxis::Network::ServerInterface<T>* server, uint64_t uid = 0)
			{
				if (m_OwnerType == Owner::server)
				{
					if (m_Socket.is_open())
					{
						m_ID = uid;
						//was: ReadHeader();

						//A client has attempted to connect to the server, but we need
						//the client to first valdate itself, so first write out the
						//handshake data to be validated
						WriteValidation();

						//next, issue a tast to sit and wait asynchronously for
						//precisely the validation data send back from client
						ReadValidation(server);
					}
				}
			}

			inline void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
			{
				//only clients can connect to servers
				if (m_OwnerType == Owner::client)
				{
					//request asio attempts to connect to an endpoint
					asio::async_connect(m_Socket, endpoints,
						[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
						{
							if (!ec)
							{
								ReadValidation();
							}
							else
							{
								PX_CORE_ERROR("Client Error: {0}", ec.message());
							}
						});
				}
			}
			inline void Disconnect() 
			{
				if (IsConnected())
				{
					asio::post(m_AsioContext, [this]() {m_Socket.close(); });
				}
			}
			inline bool IsConnected()
			{
				return m_Socket.is_open();
			}

			inline void Send(const Message<T>& msg) {
				asio::post(m_AsioContext,
					[this, msg]()
					{
						bool WritingMessage = !m_QueueMessagesOut.empty();
						m_QueueMessagesOut.push_back(msg);
						if (!WritingMessage)
						{
							WriteHeader();
						}
					});
			}

		private:
			//ASYNC - prime context ready to read a message header
			void ReadHeader()
			{  
				asio::async_read(m_Socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (m_msgTemporaryIn.header.size > 0)
							{
								m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
								ReadBody();
							}
							else
							{
								AddToIncomingMessageQueue();
							}
						}
						else
						{
							PX_CORE_ERROR("[{0}] Read Header Fail: {1}", m_ID, ec.message());
							m_Socket.close();
						}
					});
			}

			//ASYNC - prime context ready to read a message body
			void ReadBody()
			{
				asio::async_read(m_Socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							AddToIncomingMessageQueue();
						}
						else
						{
							//as above
							PX_CORE_ERROR("[{0}] Read Body Fail: {1}", m_ID, ec.message());
							m_Socket.close();
						}
					});
			}

			//ASYNC - prime context ready to write a message header
			void WriteHeader()
			{
				asio::async_write(m_Socket, asio::buffer(&m_QueueMessagesOut.front().header, sizeof(MessageHeader<T>)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (m_QueueMessagesOut.front().body.size() > 0)
							{
								WriteBody();
							}
							else
							{
								m_QueueMessagesOut.pop_front();
								if (!m_QueueMessagesOut.empty())
								{
									WriteHeader();
								}
							}
						}
						else
						{
							PX_CORE_ERROR("[{0}] Write Header Fail.", m_ID);
							m_Socket.close();
						}
					});
			}

			//ASYNC - prime context ready to write a message body
			void WriteBody()
			{
				asio::async_write(m_Socket, asio::buffer(m_QueueMessagesOut.front().body.data(), m_QueueMessagesOut.front().body.size()),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							m_QueueMessagesOut.pop_front();

							if (!m_QueueMessagesOut.empty())
							{
								WriteHeader();
							}
						}
						else
						{
							PX_CORE_ERROR("[{0}] Write Body Fail.", m_ID);
							m_Socket.close();
						}
					});
			}

			inline void AddToIncomingMessageQueue()
			{
				if (m_OwnerType == Owner::server)
					m_QueueMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
				else
					m_QueueMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

				ReadHeader();
			}

			inline void WriteValidation()
			{
				asio::async_write(m_Socket, asio::buffer(&m_HandshakeOut, sizeof(uint64_t)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							//validation data sent, clients should sit and wait for a response
							if (m_OwnerType == Owner::client)
							{
								ReadHeader();
							}
							//no else needed, because the next command
							//for server is handled in ConnectToClient
						}
						else
						{
							m_Socket.close();
						}
					});
			}

			inline void ReadValidation(Pyxis::Network::ServerInterface<T>* server = nullptr)
			{
				asio::async_read(m_Socket, asio::buffer(&m_HandshakeIn, sizeof(uint64_t)),
					[this, server](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (m_OwnerType == Owner::server)
							{
								if (m_HandshakeIn == m_HandshakeCheck)
								{
									PX_CORE_INFO("Client Validated");
									server->OnClientValidated(this->shared_from_this());

									//now sit and recieve data
									ReadHeader();
								}
								else
								{
									PX_CORE_WARN("Client Disconnected (Failed Validation)");
									m_Socket.close();
								}
							}
							else
							{
								//connection is a client, so solve puzzle
								m_HandshakeOut = scramble(m_HandshakeIn);

								//write the validation
								WriteValidation();
							}
						}
						else
						{
							PX_CORE_ERROR("[{0}] Read Header Fail: {1}", m_ID, ec.message());
							m_Socket.close();
						}
					});
			}

			inline uint64_t scramble(uint64_t input)
			{
				uint64_t out = input * 0x000000012167F051;
				out = out ^ 0xFEEDDADADEADBEEF;
				out = (out & 0xF0F0F0F0F0F0F0F0) >> 3 | (out & 0x0F0F0F0F0F0F0F0F) << 5;
				return out ^ 0xDEADABBABEEFFACE;
			}

		protected:
			//each connection has a unique socket to a remote
			asio::ip::tcp::socket m_Socket;

			//this context is shared with the whole asio instance
			asio::io_context& m_AsioContext;

			//this queue holds all messages to be sent to the remote side
			//of this connection
			ThreadSafeQueue<Message<T>> m_QueueMessagesOut;

			//This queue holds all messages that have been recieved from
			//the remote side of this connection. Note is is a reference
			//as the "owner" of this connection is expected to provide a queue
			ThreadSafeQueue<OwnedMessage<T>>& m_QueueMessagesIn;
			Message<T> m_msgTemporaryIn;

			// the "owner" decides how some of the context behaves
			Owner m_OwnerType = Owner::server;

			//the unique identifier of the connection
			uint64_t m_ID = 0;

			//handshake validation
			uint64_t m_HandshakeOut = 0;
			uint64_t m_HandshakeIn = 0;
			uint64_t m_HandshakeCheck = 0;

		};
	}
}