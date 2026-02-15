#pragma once
#include <cstdint>

typedef __int64(__fastcall* TraceShapeFn)(__int64*, float*, float*, float*, __int64*, __int64*);
typedef void(__fastcall* SkyBoxObjectDrawArrayFn)(__int64 this_ptr, __int64 render, __int64 primitive, int nCount, int RenderFlag, __int64 view_info, __int64 render_stats);
typedef void(__fastcall* FrameStageFn)(void* _this, int curStage);
typedef void(__fastcall* LevelInitFn)(void* pClientModeShared, const char* szNewMap);
typedef bool(__fastcall* EquipItemInLoadoutFn)(void* thisptr, int iTeam, int iSlot, uint64_t iItemID);
void __fastcall hkSkyBoxObjectDrawArray(__int64 this_ptr, __int64 render, __int64 primitive, int nCount, int RenderFlag, __int64 view_info, __int64 render_stats);
__int64 __fastcall hkTraceShape(__int64* p1, float* p2, float* pStart, float* pEnd, __int64* p5, __int64* pTrace);
bool __fastcall hkEquipItemInLoadout(void* thisptr, int iTeam, int iSlot, uint64_t iItemID);
void __fastcall hkFrameStage(void* _this, int curStage);
void __fastcall hkLevelInit(void* pClientModeShared, const char* szNewMap);
extern LevelInitFn oLevelInit;
extern SkyBoxObjectDrawArrayFn oSkyBoxObjectDrawArray;
extern FrameStageFn oFrameStage;
extern EquipItemInLoadoutFn oEquipItemInLoadout;
extern TraceShapeFn oTraceShape;