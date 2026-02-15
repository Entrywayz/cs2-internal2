#include "core/math/math.h"
#include "cs2/classes/CCSGOInput.h"
#include "aim.h"
#include <vector>
#include "cs2/offsets/client_dll.hpp"
#include "cs2/offsets/offsets.hpp"
#include "core/mem/mem.h"
#include <algorithm>
#include "core/globals.h"

namespace Mem {
    class PtrValidator {
    private:
        struct CacheEntry {
            void* base = nullptr;
            size_t region_size = 0;
            DWORD state = 0;
            DWORD protect = 0;
            DWORD time = 0;
        };

        static const size_t CACHE_SIZE = 256; // Размер кэша можно настроить
        static CacheEntry cache[256]; // Используем CACHE_SIZE
        static DWORD last_check;

        static bool CheckAndCache(void* ptr) {
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0)
                return false;

            size_t index = (reinterpret_cast<uintptr_t>(ptr) >> 12) % CACHE_SIZE;
            cache[index] = {
                mbi.BaseAddress,
                mbi.RegionSize,
                mbi.State,
                mbi.Protect,
                GetTickCount()
            };

            return (mbi.State == MEM_COMMIT) && ((mbi.Protect & PAGE_NOACCESS) == 0);
        }

    public:
        static bool IsValidPtr(void* ptr) {
            if (!ptr) return false;

            size_t index = (reinterpret_cast<uintptr_t>(ptr) >> 12) % CACHE_SIZE;
            auto& entry = cache[index];

            DWORD current_time = GetTickCount();
            if (entry.base &&
                current_time - entry.time < 1000 &&
                reinterpret_cast<uintptr_t>(ptr) >= reinterpret_cast<uintptr_t>(entry.base) &&
                reinterpret_cast<uintptr_t>(ptr) < reinterpret_cast<uintptr_t>(entry.base) + entry.region_size) {

                return (entry.state == MEM_COMMIT) && ((entry.protect & PAGE_NOACCESS) == 0);
            }

            return CheckAndCache(ptr);
        }

        // Для отладки/очистки при необходимости
        static void ClearCache() {
            memset(cache, 0, sizeof(cache));
            last_check = 0;
        }
    };


}
// Определение статических членов
Mem::PtrValidator::CacheEntry Mem::PtrValidator::cache[Mem::PtrValidator::CACHE_SIZE];
DWORD Mem::PtrValidator::last_check = 0;

float distance(const Vec3& a, const Vec3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

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

uintptr_t aimbot::client = (uintptr_t)(GetModuleHandleA("client.dll"));
uintptr_t aimbot::engine2 = (uintptr_t)(GetModuleHandleA("engine2.dll"));

static Vec3 punchAngleOld{ 0, 0, 0 };

QAngle_t aimbot::aimbot() {

    float closestDist = FLT_MAX;
    float closestFOV = FLT_MAX;
    Vec3 bestTargetPos = { 0, 0, 0 };
    bool foundTarget = false;

    const float MAX_DISTANCE = 10000.0f; 

    if (!client) return { 0, 0, 0 };

    uintptr_t vmAddr = client + cs2_dumper::offsets::client_dll::dwViewMatrix;
    if (!Mem::PtrValidator::IsValidPtr((void*)vmAddr) || !vmAddr) return { 0, 0, 0 };
    float (*ViewMatrix)[4][4] = (float(*)[4][4])vmAddr;
    Vec3* viewAngles = reinterpret_cast<Vec3*>(client + cs2_dumper::offsets::client_dll::dwViewAngles);
    uintptr_t localPawn = *(uintptr_t*)(client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
    if (!localPawn || !Mem::PtrValidator::IsValidPtr((void*)localPawn)) return { 0, 0, 0 };
    uintptr_t entityList = *(uintptr_t*)(client + cs2_dumper::offsets::client_dll::dwEntityList);
    if (!Mem::PtrValidator::IsValidPtr((void*)entityList)) return { 0, 0, 0 };
    uint8_t localTeam = *(uint8_t*)(localPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
    Vec3 enemyPos;
    if (!localPawn || !entityList) return { 0, 0, 0 };

    //get local eye position
    uintptr_t localNode = *(uintptr_t*)(localPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
    Vec3 localEyePos = *(Vec3*)(localNode + cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecOrigin) +
        *(Vec3*)(localPawn + cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset);

    int shotsFired = *(int*)(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired);

    Vec3 aimPunch = *(Vec3*)(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchAngle);

    for (int i = 1; i < 64; i++) {
        uintptr_t list_entry = *(uintptr_t*)(entityList + (8 * (i >> 9) + 16));
        if (!list_entry) continue;

        uintptr_t controller_identity = list_entry + (112 * (i & 0x1FF));
        if (!controller_identity) continue;

        uintptr_t controller = *(uintptr_t*)(controller_identity);
        if (controller <= 0x1000) continue;

        uintptr_t namePtr = *(uintptr_t*)(controller + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName);
        const char* playerName = (namePtr > 0x1000) ? (const char*)namePtr : "?";

        uint32_t pawnHandle = *(uint32_t*)(controller + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn);
        if (pawnHandle == 0xFFFFFFFF) continue;

        int pawnIndex = pawnHandle & 0x7FFF;
        uintptr_t p_chunk_ptr = *(uintptr_t*)(entityList + (8 * (pawnIndex >> 9) + 16));
        if (!p_chunk_ptr) continue;

        uintptr_t pawn_identity = p_chunk_ptr + (112 * (pawnIndex & 0x1FF));
        if (!pawn_identity) continue;

        uintptr_t current_pawn = *(uintptr_t*)(pawn_identity);
        if (current_pawn <= 0x1000 || current_pawn == localPawn) continue;

        uint8_t currentTeamNum = *(uint8_t*)(current_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
        if (!currentTeamNum) continue;

        int health = *(int*)(current_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
        if (health <= 0 || health > 100) continue;

        if (currentTeamNum != localTeam) {
            uintptr_t pSceneNode = *(uintptr_t*)(current_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
            if (!pSceneNode) continue;

            uintptr_t boneArray = *(uintptr_t*)(pSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80);
            if (!boneArray) continue;

            int BoneIndex = 6;

            Vec3 enemyHead = *(Vec3*)(boneArray + 6 * 32);

            float currentDist = distance(localEyePos, enemyHead);

            if (currentDist > MAX_DISTANCE) continue;

            float currentFOV = CalculateFOV(localEyePos, enemyHead, *viewAngles);

            if (currentFOV > globals::aimbotFov) continue;

            if (currentFOV < closestFOV ||
                (fabs(currentFOV - closestFOV) < 0.1f && currentDist < closestDist)) {
                closestFOV = currentFOV;
                closestDist = currentDist;
                bestTargetPos = enemyHead;
                foundTarget = true;
            }
        }
    }

    Vec3 finalAngles;

    if (foundTarget) {
        Vec3 angleToTarget = (bestTargetPos - localEyePos);

        if (shotsFired > 0) {
            angleToTarget.x -= aimPunch.x * 2.0f;
            angleToTarget.y -= aimPunch.y * 2.0f;
        }

        return angleToTarget.ToQAngle_t();
    }

    if (shotsFired > 1) {
        Vec3 delta = {
            (aimPunch.x - punchAngleOld.x) * 2.0f,
            (aimPunch.y - punchAngleOld.y) * 2.0f,
            0.0f
        };

        QAngle_t compensated;
        compensated.pitch = viewAngles->x - delta.x;
        compensated.yaw = viewAngles->y - delta.y;
        compensated.roll = 0.0f;

        punchAngleOld = aimPunch;
        return compensated;
    }

    punchAngleOld = { 0, 0, 0 };
    return viewAngles ? QAngle_t(viewAngles->x, viewAngles->y, 0.0f) : QAngle_t{ 0,0,0 };
}