// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"
#include "Components/Component.h"
#include "Scene.h"

namespace Tempus
{
    class TEMPUS_API Entity
    {
        friend class Scene;

        Entity(uint32_t id);
        
        const uint32_t m_Id;
        bool bActive = true;
        Scene* m_OwnerScene = nullptr;
    
    public:
        
        uint32_t GetId() const { return m_Id; }

        template<ValidComponent T, typename ...Args>
        void AddComponent(Args&&... arguments)
        {
            if (m_OwnerScene)
            {
                m_OwnerScene->AddComponent<T>(m_Id, std::forward<Args>(arguments)...);
            }
            else
            {
                TPS_ERROR("Entity [{0}] does not belong to a scene!", m_Id);
            }
        }

        template<ValidComponent T>
        T* GetComponent()
        {
            if(m_OwnerScene)
            {
                return m_OwnerScene->GetComponent<T>(m_Id);
            }
            TPS_ERROR("Entity [{0}] does not belong to a scene!", m_Id);
            return nullptr;
        }

        template<ValidComponent T>
        bool HasComponent() const
        {
            if (m_OwnerScene)
            {
                return m_OwnerScene->HasComponent<T>(m_Id);
            }
            TPS_ERROR("Entity [{0}] does not belong to a scene!", m_Id);
            return false;
        }

        template<ValidComponent T>
        void RemoveComponent()
        {
            if (m_OwnerScene)
            {
                m_OwnerScene->RemoveComponent<T>(m_Id);
            }
            else
            {
                TPS_ERROR("Entity [{0}] does not belong to a scene!", m_Id);
            }
        }
    };
}
