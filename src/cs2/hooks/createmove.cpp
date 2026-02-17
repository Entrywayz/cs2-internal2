#include "createmove.h"
#include <iostream>
#include <string>
#include "cs2/classes/CCSGOInput.h"
#include "cs2/offsets/client_dll.hpp"
#include "cs2/offsets/offsets.hpp"
#include "cs2/features/antiaim/antiaim.h"
#include "cs2/features/aim/aim.h"
#include "core/globals.h"
#include "cs2/classes/CCSGOInput.h"

CreateMoveFn oCreateMove = nullptr;

void __fastcall hkCreateMove(CCSGOInput* pThis, unsigned int slot, CUserCmd* pCmd)
{
    oCreateMove(pThis, slot, pCmd);

    if (!pCmd) return;


    if (globals::aimEnabled && !globals::silentEnabled) {
        if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) { 
            if (pCmd->csgoUserCmd.pBaseCmd && pCmd->csgoUserCmd.pBaseCmd->pViewAngles) {
                pCmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue = aimbot::aimbot();
                pCmd->csgoUserCmd.pBaseCmd->nHasBits |= EBaseCmdBits::BASE_BITS_VIEWANGLES;
            }
        }
    }


    if (globals::aimEnabled && globals::silentEnabled) {
        if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) {
            pCmd->SetSubTickAngle(aimbot::aimbot());
        }
    }
}