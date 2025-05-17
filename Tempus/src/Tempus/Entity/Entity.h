// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"
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

        template<ValidComponent T, typename ...Args>
        void AddComponent(Args... arguments)
        {
            if (m_OwnerScene)
            {
                m_OwnerScene->AddComponent<T>(m_Id, arguments...);
            }
            else
            {
                TPS_CORE_ERROR("Entity [{0}] does not belong to a scene!", m_Id);
            }
        }

        template<ValidComponent T>
        T* GetComponent()
        {
            if(m_OwnerScene)
            {
                return m_OwnerScene->GetComponent<T>(m_Id);
            }
            TPS_CORE_ERROR("Entity [{0}] does not belong to a scene!", m_Id);
            return nullptr;
        }

        template<ValidComponent T>
        bool HasComponent() const
        {
            if (m_OwnerScene)
            {
                return m_OwnerScene->HasComponent<T>(m_Id);
            }
            TPS_CORE_ERROR("Entity [{0}] does not belong to a scene!", m_Id);
            return false;
        }

    };
    
}
