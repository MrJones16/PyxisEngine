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
			Connection(Owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, std::shared_ptr<asio::ip::udp::socket> udpSocket, ThreadSafeQueue<OwnedMessage<T>>& qIn)
				: m_AsioContext(asioContext), m_Socket(std::move(socket)), m_QueueMessagesIn(qIn),
				m_UDPSocket(udpSocket)
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

			void SetID(uint64_t id)
			{
				m_ID = id;
			}

			uint64_t GetID()
			{
				return m_ID;
			}

			void SetUDPEndpoint(asio::ip::udp::endpoint ep)
			{
				m_RemoteUDPEndpoint = ep;
			}

		public:

			inline void ConnectToClient(Pyxis::Network::ServerInterface<T>* server, uint64_t uid = 0)
			{
				if (m_OwnerType == Owner::server)
				{
					if (m_Socket.is_open())
					{
						//point created udp port to the client
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

			/// <summary>
			/// 
			/// </summary>
			/// <param name="endpoints"></param>
			/// <param name="endpointsUDP"></param>
			/// <param name="Connecting">If there is an error, this will be made to false</param>
			inline void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints, const asio::ip::udp::resolver::results_type& endpointsUDP)
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
								//create a UDP socket listening to the same port used by TCP.
								m_RemoteUDPEndpoint = asio::ip::udp::endpoint(m_Socket.remote_endpoint().address(), m_Socket.remote_endpoint().port());
								m_UDPSocket = std::make_shared<asio::ip::udp::socket>(m_AsioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), m_Socket.local_endpoint().port()));
								PX_TRACE("Created UDP Socket, listening to port {0}", m_Socket.local_endpoint().port());
								ReadValidation();
							}
							else
							{
								PX_CORE_ERROR("Client Error: {0}", ec.message());
								m_Socket.close();
							}
						});
					
					//request asio attempts to connect to an endpoint for UDP as well
					/*asio::async_connect(*m_UDPSocket, endpointsUDP,
						[this](std::error_code ec, asio::ip::udp::endpoint endpoint)
						{
							if (!ec)
							{
								ReadUDPMessage();
							}
							else
							{
								PX_CORE_ERROR("Client UDP Error: {0}", ec.message());
							}
						});*/
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
				//PX_TRACE("Sending message, size:{0}", msg.header.size);
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

			//client and server can both use this to send messages using UDP
			inline void SendUDP(const Message<T>& msg) {
				//clients will send their ID, so the message can be tied to them
				WriteUDPMessage(msg);
				//WriteHeaderUDP(msg);
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
							if (m_OwnerType == Owner::client)
								m_UDPSocket->close();
						}
					});
			}


			void ReadUDPMessage()
			{
				m_UDPSocket->async_receive_from(asio::buffer(m_ReceiveBuffer, 1024),
					m_RecvEndpoint,
					[this](std::error_code ec, std::size_t length)
					{
						//PX_TRACE("Recieved UDP Header, of length {0}", length);
						PX_WARN("Recieved UDP, Endpoint: {0}:{1}", m_RecvEndpoint.address(), m_RecvEndpoint.port());
						if (!ec)
						{
							if (length < sizeof(MessageHeader<T>))
							{
								PX_ERROR("didn't recieve enough information for header");
							}
							else
							{
								//extract header from data
								memcpy(&m_msgTemporaryInUDP.header, m_ReceiveBuffer, sizeof(MessageHeader<T>));

								//make sure we are only pulling a max amount out of the body so we don't
								//access unknown memory
								if (m_msgTemporaryInUDP.header.size < (1024 - sizeof(MessageHeader<T>)))
								{
									//extract the length of data into the body
									m_msgTemporaryInUDP.body.resize(m_msgTemporaryInUDP.header.size);
									memcpy(m_msgTemporaryInUDP.body.data(), m_ReceiveBuffer + sizeof(MessageHeader<T>), m_msgTemporaryInUDP.header.size);
									//add it to the message queue
									m_QueueMessagesIn.push_back({ nullptr, m_msgTemporaryInUDP });
								}
							}
						}
						else
						{
							PX_CORE_ERROR("Read Header UDP Fail: {0}", ec.message());

							m_UDPSocket->close();
						}

						//continue reading UDP messages
						ReadUDPMessage();
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
							if (m_OwnerType == Owner::client)
								m_UDPSocket->close();
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

			//ASYNC - prime context ready to write a message header on UDP
			void WriteUDPMessage(const Message<T>& msg)
			{
				std::vector<uint8_t> data(sizeof(MessageHeader<T>) + msg.body.size());
				memcpy(data.data(), &msg.header, sizeof(MessageHeader<T>));
				memcpy(data.data() + sizeof(MessageHeader<T>), msg.body.data(), msg.body.size());
				PX_TRACE("Sending UDP message to address: {0}:{1}", m_RemoteUDPEndpoint.address(), m_RemoteUDPEndpoint.port());
				//the sending sends to the same address as TCP
				m_UDPSocket->send_to(asio::buffer(data.data(), data.size()), m_RemoteUDPEndpoint);

			}

			//ASYNC - prime context ready to write a message header on UDP
			void WriteHeaderUDP(Message<T>& msg)
			{
				m_UDPSocket->send_to(asio::buffer(&msg.header, sizeof(MessageHeader<T>)), m_RemoteUDPEndpoint);

				WriteBodyUDP(msg);
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
							if (m_OwnerType == Owner::client)
								m_UDPSocket->close();
						}
					});
			}

			//ASYNC - prime context ready to write a message body on UDP
			void WriteBodyUDP(Message<T>& msg)
			{
				m_UDPSocket->send_to(asio::buffer(msg.body.data(), msg.body.size()),
					asio::ip::udp::endpoint(m_Socket.remote_endpoint().address(), m_Socket.remote_endpoint().port()));
			}

			inline void AddToIncomingMessageQueue()
			{
				if (m_OwnerType == Owner::server)
					m_QueueMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
				else
					m_QueueMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

				ReadHeader();
			}

			inline void AddToIncomingMessageQueueUDP()
			{
				if (m_OwnerType == Owner::server)
					m_QueueMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryInUDP });
				else
					m_QueueMessagesIn.push_back({ nullptr, m_msgTemporaryInUDP });

				ReadHeaderUDP();
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
								ReadID();
								//ReadHeader();
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

			//ASYNC - prime context ready to read a message header
			void ReadID()
			{
				asio::async_read(m_Socket, asio::buffer(&m_ID, sizeof(uint64_t)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							PX_TRACE("ID obtained from server");
							//we have obtained the ID from the server, so lets send that udp validation
							WriteUDPValidation();

							//now sit and listen to any tcp messages
							ReadHeader();
						}
						else
						{
							PX_CORE_ERROR("[{0}] Read ID Fail: {1}", m_ID, ec.message());

							m_Socket.close();
							if (m_OwnerType == Owner::client)
								m_UDPSocket->close();
						}
					});
			}

			/// <summary>
			/// Sends the validation to the server using UDP, so the server can setup the client connection's udp address
			/// </summary>
			inline void WriteUDPValidation()
			{
				std::pair<uint64_t, uint64_t> UDPEstablishing(uint64_t(-1), m_ID);
				m_UDPSocket->send_to(asio::buffer(&UDPEstablishing, sizeof(std::pair<uint64_t, uint64_t>)), m_RemoteUDPEndpoint);
				ReadUDPMessage();
			}

			inline void FinalizeConnection()
			{
				//if we are owned by the server, tell the client it's ID and have it send a UDP message
				if (m_OwnerType == Owner::server)
				{
					asio::async_write(m_Socket, asio::buffer(&m_ID, sizeof(uint64_t)),
						[this](std::error_code ec, std::size_t length)
						{
							if (!ec)
							{
								//sent the client it's id, so we wait for a udp response to finalize the connection
							}
							else
							{
								m_Socket.close();
							}
						});
				}
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
									PX_CORE_INFO("Client Validated, Establishing UDP");

									//server->OnClientValidated(this->shared_from_this());

									//send the client it's ID, and have it respond with a UDP message to set up the correct address
									FinalizeConnection();
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

								//Send a UDP validation as well to establish the servers udp connection to this device
								//WriteUDPValidation();
							}
						}
						else
						{
							PX_CORE_ERROR("[{0}] Read Header Fail: {1}", m_ID, ec.message());
							m_Socket.close();
							if (m_OwnerType == Owner::client)
								m_UDPSocket->close();
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
			// each connection has a unique socket to a remote
			asio::ip::tcp::socket m_Socket;

			// accompanied with a UDP socket, created based on the TCP socket
			std::shared_ptr<asio::ip::udp::socket> m_UDPSocket;

			//this endpoint either points to the server if you are a client, or client if a server
			//it is seperate from the udp socket since the server uses a single socket.
			asio::ip::udp::endpoint m_RemoteUDPEndpoint;

			asio::ip::udp::endpoint m_RecvEndpoint;

			//this context is shared with the whole asio instance
			asio::io_context& m_AsioContext;

			//this queue holds all messages to be sent to the remote side
			//of this connection
			ThreadSafeQueue<Message<T>> m_QueueMessagesOut;

			//this queue holds all UDP messages to be sent to the remote side
			ThreadSafeQueue<Message<T>> m_QueueMessagesOutUDP;

			//This queue holds all messages that have been recieved from
			//the remote side of this connection. Note is is a reference
			//as the "owner" of this connection is expected to provide a queue
			ThreadSafeQueue<OwnedMessage<T>>& m_QueueMessagesIn;
			Message<T> m_msgTemporaryIn;

			uint8_t m_ReceiveBuffer[1024];
			Message<T> m_msgTemporaryInUDP;

			// the "owner" decides how some of the context behaves
			Owner m_OwnerType = Owner::server;

			//the unique identifier of the connection, same as the network client
			uint64_t m_ID = 0;

			//handshake validation
			uint64_t m_HandshakeOut = 0;
			uint64_t m_HandshakeIn = 0;
			uint64_t m_HandshakeCheck = 0;
			bool m_UDPValidated = false;

		};
	}
}