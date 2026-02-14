#pragma once
#include "core/math/math.h"
#include "../ext/imgui/imgui.h"
#include <string>

namespace globals {
	inline bool showMenu = true;
	inline bool espEnabled = false;
	inline bool bhopEnabled = false;
	inline bool autostrafeEnabled = false;
	inline bool glowEnabled = false;
	inline bool skyBoxChange = false;
	inline Vec3 skyColor = { 1.f, 1.f, 1.f };
	inline float glowColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	inline bool bAntiAim = false;
	inline int iBaseYawType = 0; // 0 = none, 1 = backwards, 2 = forwards
	inline int iPitchType = 0;   // 0 = none, 1 = down, 2 = up, 3 = zero
	inline bool thirdPersonEnabled = false;
	inline float thirdPersonDistance = 100.0f;
	inline float aimbotFov = 10.0f;
	inline bool aimEnabled = false;
	inline bool silentEnabled = false;
	inline bool chamsEnabled = false;
	inline float chamsColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	inline int hitboxIndex[6] = {6, 5, 3, 2, 0};
	inline const char* hitboxNames[6] = {"Head", "Neck", "Chest", "Lower chest", "Stomach", "Pelvis"};
	inline int g_PenetrationStatus = 0;
	inline bool crosshairPenetration = false;
	inline float dotColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	inline std::string addrCCSGO = "";
	inline float flTestPitch = 0.0f;
	inline float flTestYaw = 0.0f;
	inline bool en = false;
}