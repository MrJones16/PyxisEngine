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
				: m_AsioAcceptor(m_AsioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{

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
									m_AsioContext, std::move(socket), m_QueueMessagesIn);

							//give the user server a chance to deny connection
							if (OnClientConnect(newConn))
							{
								//connection allowed, so add to container of new connections
								m_DeqConnections.push_back(std::move(newConn));

								m_DeqConnections.back()->ConnectToClient(this, nIDCounter++);

								PX_CORE_INFO("[SERVER] Connection [{0}] Approved", m_DeqConnections.back()->GetID());
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
					if (client && client->IsConnected())
					{
						if (client != ignoreClient)
							client->Send(msg);
					}
					else
					{
						if (client != ignoreClient)
							OnClientDisconnect(client);
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

			//container of active validated connections
			std::deque<std::shared_ptr<Connection<T>>> m_DeqConnections;

			//order of declaration is important - it is also the order of initializition
			asio::io_context m_AsioContext;
			std::thread m_ContextThread;

			//these things need an asio context
			asio::ip::tcp::acceptor m_AsioAcceptor;

			// clients will be identified in the "wider system" via an ID
			uint64_t nIDCounter = 10000;


		};

	}
}