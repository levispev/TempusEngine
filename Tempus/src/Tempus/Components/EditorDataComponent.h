// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include "Component.h"
#include "Utils/EnumClassFlagUtils.h"

namespace Tempus
{
    enum class EditorEntityDataFlags : uint8_t
    {
        None = 0,
        NoDelete = BIT(0),
        NoSerialize = BIT(1),
        HideInOutliner = BIT(2),
        NoAddComponent = BIT(3)
    };

    ENUM_CLASS_FLAGS(EditorEntityDataFlags);

    // @TODO In the future this component will have special metadata that prevents it from being manually added in the editor
    class TEMPUS_API EditorDataComponent : public Component
    {
        DECLARE_COMPONENT(EditorDataComponent, 3)
        TPS_DEBUG_NAME("Editor Data Component")
        
    public:

        EditorDataComponent() = default;
        EditorDataComponent(EditorEntityDataFlags inFlags) : flags(inFlags) {}

        EditorEntityDataFlags flags = EditorEntityDataFlags::None;
        
    };
    
}
