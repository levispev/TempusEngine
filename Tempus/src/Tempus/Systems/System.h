// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include "IUpdateable.h"
#include <bitset>

namespace Tempus
{
    using ComponentSignature = std::bitset<MAX_COMPONENTS>;
    
    class TEMPUS_API System : public IUpdateable
    {
    public:

        virtual ~System() override = default;
        virtual void OnInit() {}
        bool IsUpdating() const override { return true; }
        virtual void OnUpdate(float DeltaTime) override {}
        
        ComponentSignature GetComponentSignature() const { return m_Signature; }

    protected:

        ComponentSignature m_Signature;
        //std::vector<Entity> m_Entities;
    };

}
