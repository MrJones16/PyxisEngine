#pragma once
#include <forward_list>

namespace Pyxis
{
	
	
	//signal system, basically events, but a new name and with the observer design pattern

	//Signal<void(int)> m_Signal;
	//
	//Reciever<void(int)> m_Reciever;
	//
	//m_Reciever(std::bind(&MenuLayer::TestFunction, this, std::placeholders::_1))
	//OR
	//m_Reciever(this, &MenuLayer::TestFunction)


	template<typename FunctionLayout>
	struct Callable {
	public:
		std::function<FunctionLayout> m_Function;

		explicit Callable(std::function<FunctionLayout> func)
			: m_Function(std::move(func)) {}
	};

	template<typename FunctionLayout>
	class Reciever {
	public:
		explicit Reciever(std::function<FunctionLayout> function) {
			m_Callable = std::make_shared<Callable<FunctionLayout>>(std::move(function));
		}

		// Constructor for an object and a member function
		template<typename ClassType>
		explicit Reciever(ClassType* object, FunctionLayout ClassType::* memberFunction) {
			m_Callable = std::make_shared<Callable<FunctionLayout>>(
				[object, memberFunction](auto&&... args) {
					(object->*memberFunction)(std::forward<decltype(args)>(args)...);
				});
		}

		std::shared_ptr<Callable<FunctionLayout>> m_Callable;
	};

	template<typename FunctionLayout>
	class Signal {
	private:

	public:
		std::forward_list<std::weak_ptr<Callable<FunctionLayout>>> m_Callbacks;
		// Add a listener to the event
		void AddReciever(const Reciever<FunctionLayout>& listener) {
			m_Callbacks.push_front(listener.m_Callable);
		}

		// Overload the () operator to invoke all listeners
		template<typename... Args>
		void operator()(Args&&... args) {
			auto it = m_Callbacks.before_begin(); // Iterator to track previous element
			auto current = m_Callbacks.begin();  // Current element

			while (current != m_Callbacks.end()) {
				if (auto sharedFunc = current->lock()) {
					// Valid listener; invoke it
					sharedFunc->m_Function(std::forward<Args>(args)...);
					it = current++; // Move to the next element
				}
				else {
					// Expired listener; remove it
					current = m_Callbacks.erase_after(it);
				}
			}
		}
	};
	
	
}