#include "hooks.h"
#include "minhook/include/MinHook.h"
#include "core/mem/mem.h"
#include "core/globals.h"
#include "present.h"
#include "createmove.h"
#include <iostream>
#include "world.h"

bool hooks::Init() {
	if (MH_Initialize() != MH_OK) {
		return false;
	}

	if (!SetupPresentHook()) {
		return false;
	}

	if (!SetupCreateMoveHook()) {
		return false;
	}

	if (!SetupSkyBoxObjectDrawArray()) {  
		return false;
	}

	if (!SetupPenetrate()) {
		return false;
	}

	MH_EnableHook(MH_ALL_HOOKS);

	return true;
}

void hooks::Shutdown() {
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninitialize(); 
}

bool hooks::SetupPresentHook() {
	uintptr_t presentAddr = Mem::PatternScan(PRESENT_PATTERN, "GameOverlayRenderer64.dll");
	if (!presentAddr) return false;

	if (MH_CreateHook(reinterpret_cast<LPVOID>(presentAddr), &hkPresent, reinterpret_cast<LPVOID*>(&oPresent)) != MH_OK) return false;

	return true;
}

bool hooks::SetupCreateMoveHook() {
	uintptr_t createMoveAddr = Mem::PatternScan(CREATEMOVE_PATTERN, "client.dll");
	if (!createMoveAddr) return false;
	uintptr_t cmdpattern = Mem::PatternScan("48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF 48 8B F0", "client.dll");
	std::cout << "[+]CUserCmd: " + std::to_string(cmdpattern) << std::endl;
	std::cout << "[+]Createmove: " + std::to_string(createMoveAddr) << std::endl;
	if (MH_CreateHook(reinterpret_cast<LPVOID>(createMoveAddr), &hkCreateMove, reinterpret_cast<LPVOID*>(&oCreateMove)) != MH_OK) return false;

	return true;
}

bool hooks::SetupSkyBoxObjectDrawArray()
{
	uintptr_t skyboxAddr = Mem::PatternScan(SKYBOX_PATTERN, "scenesystem.dll");
	if (!skyboxAddr) return false;

	if (MH_CreateHook(reinterpret_cast<LPVOID>(skyboxAddr), &hkSkyBoxObjectDrawArray, reinterpret_cast<LPVOID*>(&oSkyBoxObjectDrawArray)) != MH_OK) return false;
	
	return true;
}

bool hooks::SetupPenetrate() {
	uintptr_t penetrateAddr = Mem::PatternScan(TRACESHAPE_PATTERN, "client.dll");
	if (!penetrateAddr) return false;

	if (MH_CreateHook(reinterpret_cast<LPVOID>(penetrateAddr), &hkTraceShape, reinterpret_cast<LPVOID*>(&oTraceShape)) != MH_OK) return false;

	return true;
}