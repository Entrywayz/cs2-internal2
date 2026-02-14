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



    if (globals::aimEnabled) {
        if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) {
            pCmd->SetSubTickAngle(aimbot::aimbot());
        }
    }
}