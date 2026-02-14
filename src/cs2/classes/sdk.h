#pragma once
#include <cstdint>  
#include <windows.h> 
#include "core/math/math.h"
#include "cs2/offsets/client_dll.hpp"
#include "cs2/offsets/offsets.hpp"

#define SCHEMA_FIELD(type, name, offset) inline type& name() { \
    return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); \
}

#define SCHEMA_FIELD_PTR(type, name, offset) inline type* name() { \
    return reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); \
}

class CGameSceneNode;
class Vec3; 
class CGlowProperty;
class C_BaseEntity {
public:
    inline int32_t& m_iHealth() {
        return *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(this) + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
    };
    SCHEMA_FIELD(uint8_t, m_iTeamNum, cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
    SCHEMA_FIELD_PTR(CGameSceneNode, m_pGameSceneNode, cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
};

class CCSPlayerController {
public:
    SCHEMA_FIELD(uint32_t, m_hPlayerPawn, cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn);

    SCHEMA_FIELD_PTR(const char, m_sSanitizedPlayerName, cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName);

    const char* GetPlayerName() {
        return m_sSanitizedPlayerName();
    }
};

class CGameSceneNode {
public:
    SCHEMA_FIELD(Vec3, m_vecAbsOrigin, cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin);
};

class CGlowProperty {
public:
    SCHEMA_FIELD(bool, m_bGlowing, cs2_dumper::schemas::client_dll::CGlowProperty::m_bGlowing);
    SCHEMA_FIELD(uint32_t, m_glowColorOverride, cs2_dumper::schemas::client_dll::CGlowProperty::m_glowColorOverride);
};

class C_BaseModelEntity : public C_BaseEntity {
public:
    CGlowProperty* GetGlow() {
        return reinterpret_cast<CGlowProperty*>(
            reinterpret_cast<uintptr_t>(this) +
            cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_Glow
            );
    }
};