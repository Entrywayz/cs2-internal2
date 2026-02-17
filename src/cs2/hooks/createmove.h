#pragma once
#include "cs2/classes/CCSGOInput.h"

typedef void(__fastcall* CreateMoveFn)(CCSGOInput* pThis, unsigned int slot, CUserCmd* pCmd);

extern CreateMoveFn oCreateMove; 

void __fastcall hkCreateMove(CCSGOInput* pThis, unsigned int slot, CUserCmd* pCmd);