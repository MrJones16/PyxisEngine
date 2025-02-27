#pragma once
#include <deque>

namespace Pyxis
{
	namespace Network
	{

		template<typename T>
		class ThreadSafeQueue
		{
		public:
			ThreadSafeQueue() = default;
			ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete;

			virtual ~ThreadSafeQueue() { clear(); }

			inline const T& front()
			{
				std::scoped_lock lock(muxQueue);
				return deqQueue.front();
			}

			inline const T& back()
			{
				std::scoped_lock lock(muxQueue);
				return deqQueue.back();
			}

			inline void push_back(const T& item)
			{
				std::scoped_lock lock(muxQueue);
				deqQueue.emplace_back(std::move(item));

				//signal the wait that there has been a message
				//std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
			}

			inline void push_front(const T& item)
			{
				std::scoped_lock lock(muxQueue);
				deqQueue.emplace_front(std::move(item));

				//signal the wait that there has been a message
				//std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
			}

			//returns true if Queue has no items
			inline bool empty()
			{
				std::scoped_lock lock(muxQueue);
				return deqQueue.empty();
			}

			//returns number of items in Queue
			inline size_t size()
			{
				std::scoped_lock lock(muxQueue);
				return deqQueue.size();
			}

			inline void clear()
			{
				std::scoped_lock lock(muxQueue);
				deqQueue.clear();
			}

			inline T pop_front()
			{
				std::scoped_lock lock(muxQueue);
				auto t = std::move(deqQueue.front());
				deqQueue.pop_front();
				return t;
			}

			inline T pop_back()
			{
				std::scoped_lock lock(muxQueue);
				auto t = std::move(deqQueue.back());
				deqQueue.pop_back();
				return t;
			}

			inline void wait()
			{
				std::unique_lock<std::mutex> ul(muxBlocking);
				while (empty())
				{
					cvBlocking.wait(ul);
				}
			}

		protected:
			std::mutex muxQueue;
			std::deque<T> deqQueue;

			std::condition_variable cvBlocking;
			std::mutex muxBlocking;
		};
	}
}