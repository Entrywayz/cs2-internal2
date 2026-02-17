#include "aim.h"
#include "cs2/classes/EntityManager.h"
#include "cs2/classes/Utils.h"
#include "core/globals.h"
#include <algorithm>
#include <cmath>
#include <iostream>
static Vec3 punchAngleOld{ 0, 0, 0 };
float CalculateFOV(const Vec3& source, const Vec3& target, const Vec3& viewAngles) {
    Vec3 direction = (target - source).Normalized();
    Vec3 viewDir;
    float pitch = viewAngles.x * (M_PI / 180.0f);
    float yaw = viewAngles.y * (M_PI / 180.0f);
    viewDir.x = cos(pitch) * cos(yaw);
    viewDir.y = cos(pitch) * sin(yaw);
    viewDir.z = -sin(pitch);
    float dot = direction.x * viewDir.x + direction.y * viewDir.y + direction.z * viewDir.z;
    dot = std::clamp(dot, -1.0f, 1.0f);
    return acos(dot) * (180.0f / M_PI);
}
QAngle_t aimbot::aimbot() {
    float closestDist = FLT_MAX;
    float closestFOV = FLT_MAX;
    Vec3 bestTargetPos = { 0, 0, 0 };
    bool foundTarget = false;
    const float MAX_DISTANCE = 10000.0f;
    C_CSPlayerPawn* localPawn = EntityManager::Get().GetLocalPawn();
    if (!localPawn || !localPawn->IsAlive())
        return QAngle_t{ 0, 0, 0 };
    Vec3 viewAngles = globals::viewAngles;
    uintptr_t localNode = localPawn->m_pGameSceneNode();
    if (!localNode)
        return QAngle_t{ 0, 0, 0 };
    CGameSceneNode* sceneNode = reinterpret_cast<CGameSceneNode*>(localNode);
    Vec3 localOrigin = sceneNode->m_vecOrigin();
    Vec3 viewOffset = localPawn->m_vecViewOffset();
    Vec3 localEyePos = localOrigin + viewOffset;
    int shotsFired = localPawn->m_iShotsFired();
    Vec3 aimPunch = localPawn->m_aimPunchAngle();
    const auto& entities = EntityManager::Get().GetEntities();
    for (const auto& ent : entities) {
        C_CSPlayerPawn* pawn = ent.pawn;
        if (!pawn || !pawn->IsAlive())
            continue;
        if (pawn->m_iTeamNum() == localPawn->m_iTeamNum())
            continue;
        Vec3 enemyHead = Utils::GetBonePos(pawn, BoneID::Head);
        if (enemyHead.IsZero())
            continue;
        float currentDist = localEyePos.distanceTo(enemyHead);
        if (currentDist > MAX_DISTANCE)
            continue;
        float currentFOV = CalculateFOV(localEyePos, enemyHead, viewAngles);
        if (currentFOV > globals::aimbotFov)
            continue;

        if (currentFOV < closestFOV ||
            (fabs(currentFOV - closestFOV) < 0.1f && currentDist < closestDist)) {
            closestFOV = currentFOV;
            closestDist = currentDist;
            bestTargetPos = enemyHead;
            foundTarget = true;
        }
    }
    if (foundTarget) {
        Vec3 angleToTarget = (bestTargetPos - localEyePos);

        if (shotsFired > 0) {
            angleToTarget.x -= aimPunch.x * 2;
            angleToTarget.y -= aimPunch.y * 2;
        }

        return angleToTarget.ToQAngle_t();
    }
    if (shotsFired > 1) {
        Vec3 delta = {
            (aimPunch.x - punchAngleOld.x) * 2,
            (aimPunch.y - punchAngleOld.y) * 2,
            0.0f
        };
        QAngle_t compensated;
        compensated.pitch = viewAngles.x - delta.x;
        compensated.yaw = viewAngles.y - delta.y;
        compensated.roll = 0.0f;

        punchAngleOld = aimPunch;
        return compensated;
    }
    punchAngleOld = { 0, 0, 0 };
    return QAngle_t(viewAngles.x, viewAngles.y, 0.0f);
}