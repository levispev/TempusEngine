// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"

namespace Tempus
{
    class TEMPUS_API Component
    {

    private:

        friend class Scene;
        
        Component();
        ~Component();
        
    public:

        inline uint8_t GetId() const { return m_Id; }

    protected:

        const uint8_t m_Id;
        
    };
    
}
