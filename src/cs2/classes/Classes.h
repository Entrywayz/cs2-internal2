#pragma once
#include <cstdint>
#include "cs2/offsets/client_dll.hpp"
#include "cs2/offsets/offsets.hpp"
#include "core/math/math.h"

#define SCHEMA(type, name, offset) \
    type name() const { \
        return *reinterpret_cast<const type*>(reinterpret_cast<uintptr_t>(this) + offset); \
    }

class CGameSceneNode
{
public:
    SCHEMA(uintptr_t, m_modelState, cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState);
    SCHEMA(Vec3, m_vecOrigin, cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecOrigin);
};

class C_Player_ObserverServices {
public:
    SCHEMA(uint32_t, m_hObserverTarget, cs2_dumper::schemas::client_dll::CPlayer_ObserverServices::m_hObserverTarget);
};

class C_BaseEntity
{
public:
    SCHEMA(int, m_iHealth, cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
    SCHEMA(int, m_iTeamNum, cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
    SCHEMA(Vec3, m_vOldOrigin, cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
    SCHEMA(uintptr_t, m_pGameSceneNode, cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);

    bool IsAlive() const { return m_iHealth() > 0; }
};

class C_CSPlayerPawn : public C_BaseEntity
{
public:
    SCHEMA(Vec3, m_vecViewOffset, cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset);
    SCHEMA(int, m_iShotsFired, cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired);
    SCHEMA(Vec3, m_aimPunchAngle, cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchAngle);
    SCHEMA(uintptr_t, m_pObserverServices, cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_pObserverServices);
};

class C_CSPlayerController : public C_BaseEntity
{
public:
    SCHEMA(uint32_t, m_hPlayerPawn, cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn);
    SCHEMA(const char*, m_sSanitizedPlayerName, cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName);
    SCHEMA(bool, m_bPawnIsAlive, cs2_dumper::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive);
};

