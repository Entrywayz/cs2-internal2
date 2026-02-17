#include "world.h"
#include "core/math/math.h"
#include "core/globals.h"
struct CSkyBoxObjectDesc

{


    char  pad1[0xE8];

    Vec3  skyColor;      // 0x100

    float intensity;     // 0x108

    int   skyType;       // 0x134


};

SkyBoxObjectDrawArrayFn oSkyBoxObjectDrawArray = nullptr;

void __fastcall hkSkyBoxObjectDrawArray(__int64 this_ptr, __int64 render, __int64 primitive, int nCount, int RenderFlag, __int64 view_info, __int64 render_stats) {
    if (nCount > 0) {
        uintptr_t skybox_data_addr = 0x68 * nCount + primitive - 0x50;
        auto SkyBoxData = *(CSkyBoxObjectDesc**)(skybox_data_addr);

        if (SkyBoxData) {
            if (globals::skyBoxChange) {
                auto& c = globals::skyColor;
                Vec3 skycol = Vec3(c.x, c.y, c.z);
                *(Vec3*)((__int64)SkyBoxData + 0x100) = skycol;
            }
            else {
                *(Vec3*)((__int64)SkyBoxData + 0x100) = { 1.f, 1.f, 1.f };
            }
        }

        oSkyBoxObjectDrawArray(this_ptr, render, primitive, nCount, RenderFlag, view_info, render_stats);
    }
}
//0xb7
TraceShapeFn oTraceShape = nullptr;
__int64 __fastcall hkTraceShape(__int64* p1, float* p2, float* pStart, float* pEnd, __int64* p5, __int64* pTrace) {
    __int64 result = oTraceShape(p1, p2, pStart, pEnd, p5, pTrace);

    if (globals::crosshairPenetration && pTrace) {
        // Оффсет 0xAC - это fraction (float)
        float fraction = *(float*)((uintptr_t)pTrace + 0xAC);

        // Оффсет 0xB7 - это некий флаг столкновения (char в Ghidra)
        bool hitSomething = *(char*)((uintptr_t)pTrace + 0xB7) != 0;

        // В декомпиляторе условие bVar3 (попадание):
        // (fraction <= 1.0 && fraction != 1.0) || hitSomething
        bool isHit = (fraction < 1.0f) || hitSomething;

        if (isHit) {
            // Чтобы отличить прострел от обычной стены в CS2, 
            // обычно проверяют биты в результате самой функции.
            // В твоем декомпиляторе это младший бит результата: result & 1

            // Если (result & 1) истинно, значит пуля прошла или попала в валидную цель.
            // Давай попробуем использовать это для смены цвета.
            if (result & 1) {
                globals::g_PenetrationStatus = 2; // Зеленый (прострел/попадание)
            }
            else {
                globals::g_PenetrationStatus = 1; // Красный (не простреливается)
            }
        }
        else {
            globals::g_PenetrationStatus = 0; // Никуда не попали
        }

        // Установка цветов
        if (globals::g_PenetrationStatus == 2) {
            globals::dotColor[0] = 0.0f; globals::dotColor[1] = 1.0f; globals::dotColor[2] = 0.0f; globals::dotColor[3] = 1.0f;
        }
        else {
            globals::dotColor[0] = 1.0f; globals::dotColor[1] = 0.0f; globals::dotColor[2] = 0.0f; globals::dotColor[3] = 1.0f;
        }
    }

    return result;
}