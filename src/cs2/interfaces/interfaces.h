#pragma once
#include <cstdint>
#include <windows.h>
#include "core/math/math.h"

#define SCHEMA_FIELD(type, name, className, fieldName) \
    type& name() { \
        static int offset = offsets::GetOffset(className, fieldName); \
        return *(type*)((uintptr_t)this + offset); \
    }

class C_BaseEntity {
public:
    SCHEMA_FIELD(int, m_iHealth, "C_BaseEntity", "m_iHealth");
    SCHEMA_FIELD(uint8_t, m_iTeamNum, "C_BaseEntity", "m_iTeamNum");
    SCHEMA_FIELD(CGameSceneNode*, m_pGameSceneNode, "C_BaseEntity", "m_pGameSceneNode");

    virtual bool IsPlayer() = 0;
    virtual bool IsAlive() { return m_iHealth() > 0; }
};

class CGameSceneNode {
public:
    SCHEMA_FIELD(Vector, m_vecAbsOrigin, "CGameSceneNode", "m_vecAbsOrigin");
};