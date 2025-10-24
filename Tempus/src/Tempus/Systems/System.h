// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include "IUpdateable.h"
#include <bitset>
#include <set>

namespace Tempus
{
    using ComponentSignature = std::bitset<MAX_COMPONENTS>;
    
    class TEMPUS_API System : public IUpdateable
    {
    public:

        virtual ~System() override = default;
        virtual void OnInit(class Scene* ownerScene) { m_OwnerScene = ownerScene; }
        bool IsUpdating() const override { return true; }
        virtual void OnUpdate(float DeltaTime) override {}
        
        ComponentSignature GetComponentSignature() const { return m_Signature; }

    protected:

        ComponentSignature m_Signature;
        std::set<uint32_t> m_Entities;
        Scene* m_OwnerScene = nullptr;
    };

}
