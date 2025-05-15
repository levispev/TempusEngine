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
        void AddComponent(Args... arguments)
        {
            if (m_OwnerScene)
            {
                m_OwnerScene->AddComponent<T>(m_Id, arguments...);
            }
        }

    };
    
}
