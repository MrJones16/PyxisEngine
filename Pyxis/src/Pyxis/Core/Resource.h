#pragma once

#include <memory>
#include <type_traits>
#include <Pyxis/Core/Log.h>

namespace Pyxis
{
	
    /// <summary>
    /// A resource is something that is loaded using a filepath, and subsequent calls to
	/// load the same resource will return the same instance of the resource.
    /// </summary>
    class Resource {
    public:
        
        Resource(std::string filePath) :
            m_FilePath(filePath)
        {

        }
        virtual ~Resource() = default;

        std::string& GetPath() { return m_FilePath; };

    private:
        std::string m_FilePath = "";
        friend class ResourceManager;
    };

    // Concept to ensure T is derived from Resource
    template<typename T>
    concept ResourceType = std::is_base_of_v<Resource, T>;

    // ResourceManager class
    class ResourceManager {
    public:
        template<ResourceType T>
        static Ref<T> Load(std::string filePath)
        {
            if (m_Resources.contains(filePath))
            {
                if (Ref<T> res = std::dynamic_pointer_cast<T>(m_Resources[filePath]))
                {
                    
                    return res;
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
                
                auto ref = CreateRef<T>(filePath);
                m_Resources[filePath] = ref;
                return ref;
            }
        }

    private:
        inline static std::unordered_map<std::string, Ref<Resource>> m_Resources;
    };
}
