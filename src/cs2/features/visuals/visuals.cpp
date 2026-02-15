#include "visuals.h"
#include <cstdint>
#include <windows.h>
#include <algorithm>
#include "cs2/offsets/client_dll.hpp"
#include "cs2/classes/CCSGOInput.h"
#include "cs2/offsets/offsets.hpp"
#include "core/math/math.h"
#include "imgui/imgui.h"
#include "core/globals.h"
#include <chrono>
#include "cs2/classes/sdk.h"
#include "core/mem/mem.h"

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

uintptr_t visuals::client = (uintptr_t)(GetModuleHandleA("client.dll"));
uintptr_t visuals::engine2 = (uintptr_t)(GetModuleHandleA("engine2.dll"));

bool IsInGame() {
    uintptr_t networkClient = *(uintptr_t*)(visuals::engine2 + cs2_dumper::offsets::engine2_dll::dwNetworkGameClient);
    if (!Mem::PtrValidator::IsValidPtr((void*)networkClient))
        return false;

    int signonState = *(int*)(networkClient + cs2_dumper::offsets::engine2_dll::dwNetworkGameClient_signOnState);

    return signonState == 6;
}

void glow(uint64_t current) {
    if (!Mem::PtrValidator::IsValidPtr((void*)current)) return;

    uint64_t glowBase = current + cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_Glow;
    if (!Mem::PtrValidator::IsValidPtr((void*)glowBase)) return;

    uint64_t glowColorOverride = glowBase + cs2_dumper::schemas::client_dll::CGlowProperty::m_glowColorOverride;
    uint64_t glowEnable = glowBase + cs2_dumper::schemas::client_dll::CGlowProperty::m_bGlowing;

    if (!Mem::PtrValidator::IsValidPtr((void*)glowColorOverride) || !Mem::PtrValidator::IsValidPtr((void*)glowEnable)) return;

    uint8_t r = static_cast<uint8_t>(globals::glowColor[0] * 255.0f);
    uint8_t g = static_cast<uint8_t>(globals::glowColor[1] * 255.0f);
    uint8_t b = static_cast<uint8_t>(globals::glowColor[2] * 255.0f);
    uint8_t a = static_cast<uint8_t>(globals::glowColor[3] * 255.0f);   

    *(uint32_t*)glowColorOverride = (a << 24) | (r << 16) | (g << 8) | b;
    *(bool*)glowEnable = true;
}

void visuals::PlayerESP() {
    if (!client || !engine2) return;
    if (!IsInGame()) return;
    uintptr_t vmAddr = client + cs2_dumper::offsets::client_dll::dwViewMatrix;
    if (!Mem::PtrValidator::IsValidPtr((void*)vmAddr)) return;
    float (*ViewMatrix)[4][4] = (float(*)[4][4])vmAddr;

    uintptr_t localPawn = *(uintptr_t*)(client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
    if (!localPawn || !Mem::PtrValidator::IsValidPtr((void*)localPawn)) return;
    uintptr_t entityList = *(uintptr_t*)(client + cs2_dumper::offsets::client_dll::dwEntityList);
    if (!Mem::PtrValidator::IsValidPtr((void*)entityList)) return;
    uint8_t localTeam = *(uint8_t*)(localPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);

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
            if (globals::glowEnabled) {
                glow(current_pawn);
            }

            uintptr_t sceneNode = *(uintptr_t*)(current_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
            Vec3 feetPos = *(Vec3*)(sceneNode + cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin);
            Vec3 headPos = { feetPos.x, feetPos.y, feetPos.z + 65.0f };

            Vec2 feet, head;
            if (WorldToScreen(feetPos, feet, *ViewMatrix) && WorldToScreen(headPos, head, *ViewMatrix)) {
                float h = feet.y - head.y + 2;
                float w = h / 1.7f;
                float x = feet.x - w / 2.0f;
                float y = head.y;

                auto draw = ImGui::GetBackgroundDrawList();
                float healthBarW = 2.0f;
                float healthPercentage = (float)health / 100.0f;
                float healthHeight = h * healthPercentage;

                if (playerName) {
                    ImVec2 textSize = ImGui::CalcTextSize(playerName);
                    draw->AddText({ x + w / 2.0f - textSize.x / 2.0f, y - textSize.y - 2.0f },
                        ImColor(255, 255, 255),
                        playerName);
                }

                draw->AddRect({ x, head.y }, { x + w, feet.y }, ImColor(255, 255, 255));

                draw->AddRectFilled({ x - 6, y }, { x - 6 + healthBarW, y + h }, ImColor(0, 0, 0, 200));

                ImColor healthColor = ImColor((int)(255 * (1 - healthPercentage)), (int)(255 * healthPercentage), 0);
                draw->AddRectFilled({ x - 6, y + (h - healthHeight) }, { x - 6 + healthBarW, y + h }, healthColor);
            }
        }
    }
}

//005cea90
//0065b788
//4F 6E 41 64 64 65 64 54 6F 53 63 65 6E 65 53 79 73 74 65 6D