#pragma once

#include <Pyxis/Core/Core.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


#define REGISTER_SERIALIZABLE_TYPE(T) \
namespace { \
    struct T##_Registrar { \
        T##_Registrar() { \
            TypeRegistry::getInstance().registerType(#T, []() -> Ref<Serializable> { return CreateRef<T>(); }); \
        } \
    }; \
    static T##_Registrar global_##T##_registrar; \
}

namespace Pyxis
{

    /// <summary>
    /// Derivable class that you can override Serialize() and Deserialze().
	/// use the macro REGISTER_SERIALIZABLE_TYPE(T) after your class definition
	/// to 
    /// </summary>
    class Serializable
    {
	public:
		~Serializable() = default;

		virtual void Serialize(json& j) = 0;
		virtual void Deserialize(json& j) = 0;
    };

	// Factory function type
	using FactoryFunction = std::function<Ref<Serializable>()>;

	class TypeRegistry {
	public:
		static TypeRegistry& getInstance() {
			static TypeRegistry instance;
			return instance;
		}

		void registerType(const std::string& typeName, FactoryFunction factory) {
			registry[typeName] = factory;
		}

		Ref<Serializable> createInstance(const std::string& typeName) {
			if (registry.find(typeName) != registry.end()) {
				return registry[typeName]();
			}
			return nullptr;
		}
	private:
		std::unordered_map<std::string, FactoryFunction> registry;
	};

}
