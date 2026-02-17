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
#include "cs2/classes/EntityManager.h"
#include "core/mem/mem.h"
#include "cs2/classes/Utils.h"

static std::mutex glowMutex;

void glow(C_CSPlayerPawn* pawn) {
    if (!pawn || !Mem::PtrValidator::IsValidPtr(pawn)) return;

    std::lock_guard<std::mutex> lock(glowMutex); 

    uint64_t current = reinterpret_cast<uint64_t>(pawn);
    uint64_t glowBase = current + cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_Glow;
    if (!Mem::PtrValidator::IsValidPtr((void*)glowBase)) return;

    uint64_t glowColorOverride = glowBase + cs2_dumper::schemas::client_dll::CGlowProperty::m_glowColorOverride;
    uint64_t glowEnable = glowBase + cs2_dumper::schemas::client_dll::CGlowProperty::m_bGlowing;

    if (!Mem::PtrValidator::IsValidPtr((void*)glowColorOverride) || !Mem::PtrValidator::IsValidPtr((void*)glowEnable)) return;

    uint8_t r = static_cast<uint8_t>(globals::glowColor[0] * 255.0f);
    uint8_t g = static_cast<uint8_t>(globals::glowColor[1] * 255.0f);
    uint8_t b = static_cast<uint8_t>(globals::glowColor[2] * 255.0f);
    uint8_t a = static_cast<uint8_t>(globals::glowColor[3] * 255.0f);

    *(uint32_t*)glowColorOverride = (r << 24) | (g << 16) | (b << 8) | a;
    *(bool*)glowEnable = true;
}
void visuals::PlayerESP() {
    if (!globals::espEnabled)
        return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const float sw = ImGui::GetIO().DisplaySize.x;
    const float sh = ImGui::GetIO().DisplaySize.y;

    const auto& entities = EntityManager::Get().GetEntities();
    C_CSPlayerPawn* localPawn = EntityManager::Get().GetLocalPawn();
    if (!localPawn)
        return;

    for (const auto& ent : entities)
    {
        C_CSPlayerPawn* pawn = ent.pawn;
        if (!pawn || !pawn->IsAlive())
            continue;

        if (pawn->m_iTeamNum() == localPawn->m_iTeamNum())
            continue;

        Vec3 feet = pawn->m_vOldOrigin();
        Vec3 head = Utils::GetBonePos(pawn, BoneID::Head);
        if (head.IsZero())
            continue;

        head.z += 8.2f;

        Vec3 sFeet, sHead;
        if (!Utils::WorldToScreen(feet, sFeet, (float*)globals::ViewMatrix, sw, sh) ||
            !Utils::WorldToScreen(head, sHead, (float*)globals::ViewMatrix, sw, sh))
            continue;

        float h = sFeet.y - sHead.y + 1;
        if (h < 5.f)
            continue;

        float w = h * 0.5f;
        float x = sHead.x - w * 0.5f;
        float y = sHead.y;

        if (globals::glowEnabled) {
            glow(pawn);
        }

        if (globals::boxEnabled)
        {
            dl->AddRect({ x, y }, { x + w, y + h }, ImGui::ColorConvertFloat4ToU32(ImVec4(
                globals::boxCol[0],
                globals::boxCol[1],
                globals::boxCol[2],
                globals::boxCol[3]
            )), 0.f, 0, 2);
            dl->AddRect({ x - 1, y - 1 }, { x + w + 1, y + h + 1 }, IM_COL32(0, 0, 0, 220));
        }

        if (globals::healthEnabled)
        {
            int hp = pawn->m_iHealth();
            float hpFrac = std::clamp(hp / 100.f, 0.f, 1.f);
            float hpH = h * hpFrac;

            dl->AddRectFilled({ x - 6, y - 1 }, { x - 2, y + h + 1 }, IM_COL32(0, 0, 0, 150));
            dl->AddRectFilled({ x - 5, y + h - hpH }, { x - 3, y + h }, IM_COL32(0, 255, 0, 255));
        }

        if (globals::nameEnabled && ent.controller)
        {
            char nameBuf[64]{};
            uintptr_t namePtr = reinterpret_cast<uintptr_t>(ent.controller->m_sSanitizedPlayerName());
            if (Utils::SafeReadString(namePtr, nameBuf))
            {
                ImVec2 ts = ImGui::CalcTextSize(nameBuf);
                dl->AddText({ x + (w - ts.x) * 0.5f, y - ts.y - 2 }, ImGui::ColorConvertFloat4ToU32(ImVec4(
                    globals::nameCol[0],
                    globals::nameCol[1],
                    globals::nameCol[2],
                    globals::nameCol[3]
                )), nameBuf);
            }
        }

        if (globals::skeleton)
        {
            const float thick = globals::skelThick;

            for (const auto& conn : Bones::connections)
            {
                Vec3 b1 = Utils::GetBonePos(pawn, conn.bone1);
                Vec3 b2 = Utils::GetBonePos(pawn, conn.bone2);
                if (b1.IsZero() || b2.IsZero())
                    continue;

                Vec3 sb1, sb2;
                if (Utils::WorldToScreen(b1, sb1, (float*)globals::ViewMatrix, sw, sh) &&
                    Utils::WorldToScreen(b2, sb2, (float*)globals::ViewMatrix, sw, sh))
                {
                    dl->AddLine({ sb1.x, sb1.y }, { sb2.x, sb2.y }, ImGui::ColorConvertFloat4ToU32(ImVec4(
                        globals::skelCol[0],
                        globals::skelCol[1],
                        globals::skelCol[2],
                        globals::skelCol[3]
                    )), thick);
                }
            }
        }
    }
}
