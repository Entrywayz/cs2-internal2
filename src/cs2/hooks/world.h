#pragma once

typedef __int64(__fastcall* TraceShapeFn)(__int64*, float*, float*, float*, __int64*, __int64*);
typedef void(__fastcall* SkyBoxObjectDrawArrayFn)(__int64 this_ptr, __int64 render, __int64 primitive, int nCount, int RenderFlag, __int64 view_info, __int64 render_stats);
extern SkyBoxObjectDrawArrayFn oSkyBoxObjectDrawArray;

void __fastcall hkSkyBoxObjectDrawArray(__int64 this_ptr, __int64 render, __int64 primitive, int nCount, int RenderFlag, __int64 view_info, __int64 render_stats);
__int64 __fastcall hkTraceShape(__int64* p1, float* p2, float* pStart, float* pEnd, __int64* p5, __int64* pTrace);

typedef void (__cdecl* ViewModelFOVfn)(void);
extern ViewModelFOVfn oViewModelFOV;

void __cdecl hkViewModelFOV(void);


extern TraceShapeFn oTraceShape;