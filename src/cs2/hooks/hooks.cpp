#include "hooks.h"
#include "minhook/include/MinHook.h"
#include "cs2/interfaces/interfaces.h"
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

	if (!SetupFrameStage()) {
		return false;
	}

	if (!SetupLevelInit()) {
		return false;
	}

	if (!SetupItemInLoadout()) {
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
	uintptr_t presentAddr = Scanner::PatternScan(PRESENT_PATTERN, "GameOverlayRenderer64.dll");
	if (!presentAddr) return false;

	if (MH_CreateHook(reinterpret_cast<LPVOID>(presentAddr), &hkPresent, reinterpret_cast<LPVOID*>(&oPresent)) != MH_OK) return false;

	return true;
}

bool hooks::SetupCreateMoveHook() {
	uintptr_t createMoveAddr = Scanner::PatternScan(CREATEMOVE_PATTERN, "client.dll");
	if (!createMoveAddr) return false;
	if (MH_CreateHook(reinterpret_cast<LPVOID>(createMoveAddr), &hkCreateMove, reinterpret_cast<LPVOID*>(&oCreateMove)) != MH_OK) return false;

	return true;
}

bool hooks::SetupSkyBoxObjectDrawArray()
{
	uintptr_t skyboxAddr = Scanner::PatternScan(SKYBOX_PATTERN, "scenesystem.dll");
	if (!skyboxAddr) return false;

	if (MH_CreateHook(reinterpret_cast<LPVOID>(skyboxAddr), &hkSkyBoxObjectDrawArray, reinterpret_cast<LPVOID*>(&oSkyBoxObjectDrawArray)) != MH_OK) return false;
	
	return true;
}

bool hooks::SetupPenetrate() {
	uintptr_t penetrateAddr = Scanner::PatternScan(TRACESHAPE_PATTERN, "client.dll");
	if (!penetrateAddr) return false;

	if (MH_CreateHook(reinterpret_cast<LPVOID>(penetrateAddr), &hkTraceShape, reinterpret_cast<LPVOID*>(&oTraceShape)) != MH_OK) return false;

	return true;
}

bool hooks::SetupItemInLoadout() {
	uintptr_t itemAddr = Scanner::PatternScan("48 89 5c 24 ? 48 89 6c 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 ec ? 0f b7 fa", "client.dll");
	std::cout << "[+]ItemAddr: " << itemAddr << std::endl;
	if (!itemAddr) return false;

	if (MH_CreateHook(reinterpret_cast<LPVOID>(itemAddr), &hkEquipItemInLoadout, reinterpret_cast<LPVOID*>(&oEquipItemInLoadout)) != MH_OK)
		return false;

	return true; 
}

bool hooks::SetupLevelInit() {

	uintptr_t levelAddr = Scanner::PatternScan("40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48", "client.dll");
	std::cout << "[+]LevelAddr: " << levelAddr << std::endl;
	if (!levelAddr) return false;

	if (MH_CreateHook(reinterpret_cast<LPVOID>(levelAddr), &hkLevelInit, reinterpret_cast<LPVOID*>(&oLevelInit)) != MH_OK)
		return false;

	return true; 
}
	
bool hooks::SetupFrameStage() {
	uintptr_t frameAddr = Scanner::PatternScan("48 89 5C 24 ? 48 89 6C 24 ? 57 48 83 EC 40 48 8B F9", "client.dll");
	std::cout << "[+]FrameAddr: " << frameAddr << std::endl;
	if (!frameAddr) return false;
	if (MH_CreateHook(reinterpret_cast<LPVOID>(frameAddr), &hkFrameStage, reinterpret_cast<LPVOID*>(&oFrameStage)) != MH_OK)
		return false;

	return true; 
}
