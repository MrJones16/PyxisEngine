#pragma once

#include "NetworkCommon.h"
#include "NetworkThreadSafeQueue.h"
#include "NetworkMessage.h"
#include "NetworkConnection.h"


namespace Pyxis
{
	namespace Network
	{

		template<typename T>
		class ServerInterface
		{
		public:
			ServerInterface(uint16_t port)
				: m_AsioAcceptor(m_AsioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
				m_UDPSocket(std::make_shared<asio::ip::udp::socket>(m_AsioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)))
			{
				/*m_UDPSocket = std::make_shared<*/
			}

			inline virtual ~ServerInterface()
			{
				Stop();
			}

			inline bool Start()
			{
				try
				{
					//give it something to do
					WaitForClientConnection();

					ReadUDPMessage();
					//ReadHeaderUDP();

					//then start the context, so it doesnt stop
					m_ContextThread = std::thread([this]() {m_AsioContext.run(); });
				}
				catch (std::exception& e)
				{
					//something prohibited the server from listening
					PX_CORE_ERROR("[SERVER] Exception: {0}", e.what());
					return false;
				}

				PX_CORE_INFO("[SERVER] Started!");
				return true;
			}

			inline void Stop()
			{
				//request the context to close
				m_AsioContext.stop();

				//tidy up the context thread
				if (m_ContextThread.joinable()) m_ContextThread.join();

				//inform someone, anybody, if they care...
				PX_CORE_INFO("[SERVER] Stopped!");

			}

			//ASYNC - Tell asio to wait for connection
			inline void WaitForClientConnection()
			{
				m_AsioAcceptor.async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket)
					{
						if (!ec)
						{
							PX_CORE_INFO("[SERVER] New Connection: {0}", socket.remote_endpoint());
							
							std::shared_ptr<Connection<T>> newConn =
								std::make_shared<Connection<T>>(Connection<T>::Owner::server,
									m_AsioContext, std::move(socket), m_UDPSocket, m_QueueMessagesIn);
							

							//give the user server a chance to deny connection
							if (OnClientConnect(newConn))
							{
								//connection allowed, so add to container of new connections
								m_DeqNewConnections.push_back(std::move(newConn));

								m_DeqNewConnections.back()->ConnectToClient(this, nIDCounter++);

								m_ClientMap[m_DeqNewConnections.back()->GetID()] = m_DeqNewConnections.back();

								PX_CORE_INFO("[SERVER] Connection [{0}] Approved", m_DeqNewConnections.back()->GetID());
							}
							else
							{
								PX_CORE_INFO("[SERVER] Connection Denied");
							}
						}
						else
						{
							PX_CORE_ERROR("[SERVER] New Connection Error: {0}", ec.message());
						}

						//prime the asio context with more work - again simply wait for another conneciton
						WaitForClientConnection();
					}
				);
			}

			// send a message to a specific client
			inline void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
			{
				if (client && client->IsConnected())
				{
					client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					m_ClientMap.erase(client->GetID());
					m_DeqConnections.erase(
						std::remove(m_DeqConnections.begin(), m_DeqConnections.end(), client), m_DeqConnections.end());

				}
			}

			// send a message to a specific client
			inline void MessageClientUDP(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
			{
				if (client && client->IsConnected())
				{
					client->SendUDP(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					m_DeqConnections.erase(
						std::remove(m_DeqConnections.begin(), m_DeqConnections.end(), client), m_DeqConnections.end());

				}
			}

			//
			inline void MessageAllClients(const Message<T>& msg, std::shared_ptr<Connection<T>> ignoreClient = nullptr)
			{
				bool InvalidClientExists = false;
				//using a bool to not change the deq while iterating
				for (auto& client : m_DeqConnections)
				{
					if (client != ignoreClient)
					{
						if (client && client->IsConnected())
						{

							client->Send(msg);
						}
						else
						{

							OnClientDisconnect(client);
							m_ClientMap.erase(client->GetID());
							client.reset();
							InvalidClientExists = true;
						}
					}
				}
				 
				if (InvalidClientExists)
				{
					m_DeqConnections.erase(
						std::remove(m_DeqConnections.begin(), m_DeqConnections.end(), nullptr), m_DeqConnections.end());
				}

			}

			inline void MessageAllClientsUDP(const Message<T>& msg, std::shared_ptr<Connection<T>> ignoreClient = nullptr)
			{
				bool InvalidClientExists = false;
				//using a bool to not change the deq while iterating
				for (auto& client : m_DeqConnections)
				{
					if (client != ignoreClient)
					if (client && client->IsConnected())
					{	
						client->SendUDP(msg);
					}
					else
					{
						OnClientDisconnect(client);
						m_ClientMap.erase(client->GetID());
						client.reset();
						InvalidClientExists = true;
					}
				}

				if (InvalidClientExists)
				{
					m_DeqConnections.erase(
						std::remove(m_DeqConnections.begin(), m_DeqConnections.end(), nullptr), m_DeqConnections.end());
				}

			}

			inline void Update(size_t maxMessages = -1, bool waitForMessages = false)
			{
				if (waitForMessages) m_QueueMessagesIn.wait();
				size_t messageCount = 0;
				while (messageCount < maxMessages && !m_QueueMessagesIn.empty())
				{
					//grab the front message
					auto msg = m_QueueMessagesIn.pop_front();

					//pass to message handler 
					OnMessage(msg.remote, msg.msg);

					messageCount++;
				}
			}

		protected:

			void ReadUDPMessage()
			{
				m_UDPSocket->async_receive_from(asio::buffer(m_ReceiveBuffer, 1024),
					m_RemoteEndpoint,
					[this](std::error_code ec, std::size_t length)
					{
						/*PX_CORE_WARN("Recieved UDP Header, of length {0}", length);
						PX_CORE_WARN("Endpoint: {0}:{1}", m_RemoteEndpoint.address(), m_RemoteEndpoint.port());*/
						if (!ec)
						{
							//have to do a check to see if this is a udp message to setup a udp connection
							if (length == std::size_t(16))
							{
								uint64_t UDPHandShake;
								memcpy(&UDPHandShake, m_ReceiveBuffer, sizeof(uint64_t));
								if (UDPHandShake == uint64_t(-1))
								{
									PX_CORE_TRACE("Recieved UDP Handshake");
									uint64_t ID;
									memcpy(&ID, m_ReceiveBuffer + sizeof(uint64_t), sizeof(uint64_t));
									for (auto & conn : m_DeqNewConnections)
									{
										if (conn->GetID() == ID)
										{
											//we have recieved a udp handshake from this client, so lets finalize them.
											//set the connections endpoint so udp works
											conn->SetUDPEndpoint(m_RemoteEndpoint);
											//add the connection to validated deq
											m_DeqConnections.push_back(m_ClientMap[ID]);
											//remove it from new connections deq
											m_DeqNewConnections.erase(
												std::remove(m_DeqNewConnections.begin(), m_DeqNewConnections.end(), conn), m_DeqNewConnections.end());
											//validation call
											OnClientValidated(m_ClientMap[ID]);

											//continue reading messages
											ReadUDPMessage();
											return;
										}
									}
								}
							}
							if (length < sizeof(MessageHeader<T>))
							{
								PX_CORE_WARN("didn't recieve enough information for header");
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
									//pull the client ID from the body, and push the message with the correct client
									uint64_t clientID;
									m_msgTemporaryInUDP >> clientID;
									m_QueueMessagesIn.push_back({ m_ClientMap[clientID], m_msgTemporaryInUDP });
								}
								else
								{
									PX_CORE_WARN("Recieved too large of a message, ignoring it.");
								}
							}
						}
						else
						{
							PX_CORE_ERROR("[SERVER] Read Header UDP Fail: {0}", ec.message());
						}

						//continue reading UDP messages
						ReadUDPMessage();
					});
			}

			//called when a client connects to the server
			inline virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client)
			{
				return false;
			}

			//called when a client appears to have disconnected
			inline virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client)
			{
				
			}

			//called when a message arrives
			virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg)
			{

			}

		public:

			virtual void OnClientValidated(std::shared_ptr<Connection<T>> client)
			{

			}

		protected:
			//thread safe queue for incoming message packets
			ThreadSafeQueue<OwnedMessage<T>> m_QueueMessagesIn;

			//container of new connections that need to be validated
			std::deque<std::shared_ptr<Connection<T>>> m_DeqNewConnections;

			//container of active validated connections
			std::deque<std::shared_ptr<Connection<T>>> m_DeqConnections;

			//map of ID's to clients, for linking UDP messages
			std::unordered_map<uint64_t, std::shared_ptr<Connection<T>>> m_ClientMap;

			//order of declaration is important - it is also the order of initializition
			asio::io_context m_AsioContext;
			std::thread m_ContextThread;

			//these things need an asio context
			asio::ip::tcp::acceptor m_AsioAcceptor;

			std::shared_ptr<asio::ip::udp::socket> m_UDPSocket;
			asio::ip::udp::endpoint m_RemoteEndpoint;
			uint8_t m_ReceiveBuffer[1024];
			Message<T> m_msgTemporaryInUDP;

			// clients will be identified in the "wider system" via an ID
			uint64_t nIDCounter = 10000;


		};

	}
}