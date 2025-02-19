#pragma once

#include <glm/glm.hpp>
#include "Pyxis/Renderer/Texture.h"
#include "Pyxis/Core/Timestep.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
#include <queue>
#include <tinyxml2.h>
#include <random>

//I would use the new Pyxis/Core/Serialize.h, but since Node's set their ID in the node map
//upon construction, the constructor requires a parameter, and that's not something
//serialize can handle right now.
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#define REGISTER_SERIALIZABLE_NODE(T) \
namespace { \
    struct T##_Registrar { \
        T##_Registrar() { \
            NodeRegistry::getInstance().registerType(#T, [](UUID id) -> Ref<Node> { return Instantiate<T>(id); }); \
        } \
    }; \
    static T##_Registrar global_##T##_registrar; \
}

//override json conversion for glm objects
namespace glm {
	// Serialize glm::vec2
	inline void to_json(json& j, const glm::vec2& vec) {
		j = json{ {"x", vec.x}, {"y", vec.y} };
	}
	// Deserialize glm::vec2
	inline void from_json(const json& j, glm::vec2& vec) {
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
	}

	// Serialize glm::ivec2
	inline void to_json(json& j, const glm::ivec2& vec) {
		j = json{ {"x", vec.x}, {"y", vec.y} };
	}
	// Deserialize glm::ivec2
	inline void from_json(const json& j, glm::ivec2& vec) {
		vec.x = j.at("x").get<int>();
		vec.y = j.at("y").get<int>();
	}

	// Serialize glm::vec3
	inline void to_json(json& j, const glm::vec3& vec) {
		j = json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z} };
	}
	// Deserialize glm::vec3
	inline void from_json(const json& j, glm::vec3& vec) {
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
		vec.z = j.at("z").get<float>();
	}

	// Serialize glm::ivec3
	inline void to_json(json& j, const glm::ivec3& vec) {
		j = json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z} };
	}
	// Deserialize glm::ivec3
	inline void from_json(const json& j, glm::ivec3& vec) {
		vec.x = j.at("x").get<int>();
		vec.y = j.at("y").get<int>();
		vec.z = j.at("z").get<int>();
	}

	// Serialize glm::vec4
	inline void to_json(json& j, const glm::vec4& vec) {
		j = json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z}, {"w", vec.w}};
	}
	// Deserialize glm::vec4
	inline void from_json(const json& j, glm::vec4& vec) {
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
		vec.z = j.at("z").get<float>();
		vec.w = j.at("w").get<float>();
	}

	// Serialize glm::ivec4
	inline void to_json(json& j, const glm::ivec4& vec) {
		j = json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z}, {"w", vec.w} };
	}
	// Deserialize glm::ivec4
	inline void from_json(const json& j, glm::ivec4& vec) {
		vec.x = j.at("x").get<int>();
		vec.y = j.at("y").get<int>();
		vec.z = j.at("z").get<int>();
		vec.w = j.at("w").get<int>();
	}

}


namespace Pyxis
{


	using UUID = uint32_t;
	//Node is the base entity class that lives in a scene heirarchy.
	//Nodes is a collection of all living nodes
	// s_HoveredNodeID is the ID of the node with a mouse hovering over it, set by the scene layer.
	// It belongs to Node{} because it helps to be accessible from nodes, to know if they are hovered / wanting to be 
	// interacted with. 
	//
	class Node : public std::enable_shared_from_this<Node>
	{
	public://static Node things
		inline static std::unordered_map<UUID, Ref<Node>> Nodes;
		inline static std::queue<UUID> NodesToDestroyQueue;
		inline static UUID s_HoveredNodeID = 0;
		inline static UUID GenerateUUID() {
			static std::random_device rd;  // Non-deterministic random seed
			static std::mt19937 gen(rd()); // 64-bit Mersenne Twister
			static std::uniform_int_distribution<uint32_t> dist;
			UUID result = dist(gen);
			while (Node::Nodes.contains(result))
			{
				result = dist(gen);
			}
			return result;
		}
		inline static UUID UseExistingUUID(UUID id) {
			if ((!Node::Nodes.contains(id)) && (id != (UUID)0))
			{
				return id;
			}
			static std::random_device rd;  // Non-deterministic random seed
			static std::mt19937 gen(rd()); // 64-bit Mersenne Twister
			static std::uniform_int_distribution<uint32_t> dist;
			UUID result = dist(gen);
			while (Node::Nodes.contains(result) || (result == 0))
			{
				result = dist(gen);
			}
			return result;
		}

		//takes in a json, and creates a node ref if successful,
		//otherwise returns nullptr.
		static Ref<Node> DeserializeNode(json& j);


		//vars
	protected:
		const UUID m_UUID;

	public:
		std::string m_Name = "Node";
		Node* m_Parent = nullptr;
		std::vector<Node*> m_Children;

		bool m_Enabled = true;

	public:
		//for creating new nodes
		Node(const std::string& name = "Node");
		//for creating nodes to be deserialized
		Node(UUID id);
		virtual ~Node();

		/// <summary>
		/// Removes the node from the node map, its parent, and it's children's parent
		/// 
		/// Since its removed from the node map, the main shared_ptr reference is lost, 
		/// and the node will be deleted unless there is another one held somewhere.
		/// </summary>
		virtual void QueueFree();
		

		/// <summary>
		/// Frees this object, and all of it's children.
		/// </summary>
		virtual void QueueFreeHierarchy();

		virtual void OnUpdate(Timestep ts);
		virtual void OnFixedUpdate();
		virtual void OnRender();
		virtual void OnImGuiRender() {};
		virtual void OnInspectorRender();

		UUID GetUUID() { return m_UUID; }

		virtual void AddChild(const Ref<Node>& child);
		virtual void RemoveChild(const Ref<Node>& child);
		

		//Serialization
		virtual void Serialize(json& j);
		virtual void Deserialize(json& j);

		std::vector<uint8_t> SerializeToMessagePack(const json& j) {
			return json::to_msgpack(j);
		}

		std::vector<uint8_t> SerializeToCBOR(const json& j) {
			return json::to_cbor(j);
		}

		std::vector<uint8_t> SerializeToUBJSON(const json& j) {
			return json::to_ubjson(j);
		}

		//Compressed Binary Serialization
		std::vector<uint8_t> SerializeBinary();

		void DeserializeBinary(std::vector<uint8_t> msgpack);
	
	};

	template<typename T, typename ... Args>
	constexpr Ref<T> Instantiate(Args&& ... args)
	{
		Ref<T> result = CreateRef<T>(std::forward<Args>(args)...);
		Node::Nodes[result->GetUUID()] = result;
		return result;
	}


	// Factory function type
	using NodeFactoryFunction = std::function<Ref<Node>(UUID)>;

	class NodeRegistry {
	public:
		static NodeRegistry& getInstance() {
			static NodeRegistry instance;
			return instance;
		}

		void registerType(const std::string& typeName, NodeFactoryFunction factory) {
			registry[typeName] = factory;
		}

		Ref<Node> createInstance(const std::string& typeName, UUID id) {
			if (registry.find(typeName) != registry.end()) {
				return registry[typeName](id);
			}
			return nullptr;
		}
	private:
		std::unordered_map<std::string, NodeFactoryFunction> registry;
	};

	REGISTER_SERIALIZABLE_NODE(Node);

}