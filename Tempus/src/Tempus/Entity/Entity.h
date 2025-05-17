// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"
#include <bitset>
#include <string>
#include "Scene.h"

using ComponentSignature = std::bitset<MAX_COMPONENTS>;

namespace Tempus {

    class TEMPUS_API Entity
    {
    private:

        friend class Scene;

        const uint32_t m_Id;
        bool bActive = true;

        Scene* m_OwnerScene = nullptr;
        
        Entity(uint32_t id);
    
    public:

        ~Entity();

        template<typename T, typename ...Args>
        requires std::derived_from<T, Component>
        void AddComponent(Args... arguments)
        {
            STATIC_ASSERT_HAS_COMPONENT_ID(T);
            static_assert(T::GetId() > 0, "Cannot add invalid component!");
            if (m_OwnerScene)
            {
                m_OwnerScene->AddComponent<T>(m_Id, arguments...);
            }
        }

        template<typename T>
        requires std::derived_from<T, Component>
        bool HasComponent() const
        {
            if (m_OwnerScene)
            {
                return m_OwnerScene->HasComponent<T>(m_Id);
            }
            TPS_CORE_WARN("Entity does not belong to a scene!");
            return false;
        }

    };
    
}
