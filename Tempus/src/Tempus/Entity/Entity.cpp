// Copyright Levi Spevakow (C) 2025

#include "Entity.h"


Tempus::Entity::Entity(uint32_t id) :  m_Id(id)
{
}

Tempus::Entity::Entity(const Entity& other) : m_Id(other.m_Id)
{
    this->bActive = other.bActive;
    this->m_OwnerScene = other.m_OwnerScene;
}


