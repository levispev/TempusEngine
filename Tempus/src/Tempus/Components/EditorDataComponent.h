// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core/Core.h"
#include "Component.h"
#include "Utils/EnumClassFlagUtils.h"

namespace Tempus
{
    enum class EditorEntityDataFlags : uint8_t
    {
        None = 0,
        // Disallows deletion of the entity in the editor
        NoDelete = BIT(0),
        // Prevents the entity from being serialized in a scene file
        NoSerialize = BIT(1),
        // Hides the entity from the outliner in the editor
        HideInOutliner = BIT(2),
        // Disallows additional components being added to this entity in the editor
        NoAddComponent = BIT(3),
        // Disallows removal of components from this entity in the editor
        NoRemoveComponent = BIT(4)
    };
    ENUM_CLASS_FLAGS(EditorEntityDataFlags);

    // @TODO In the future this component will have special metadata that prevents it from being manually added in the editor
    class TEMPUS_API EditorDataComponent : public Component
    {
        DECLARE_COMPONENT(EditorDataComponent, 3, ComponentMetaData::NoEditorAdd | ComponentMetaData::NoSerialize)
        TPS_DEBUG_NAME("Editor Data Component")
        
    public:

        EditorDataComponent() = default;
        EditorDataComponent(EditorEntityDataFlags inFlags) : flags(inFlags) {}

        EditorEntityDataFlags flags = EditorEntityDataFlags::None;
        
    };
    
}
