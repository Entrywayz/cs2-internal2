    #include "world.h"
    #include "core/math/math.h"
    #include "core/globals.h"
    #include "cs2/interfaces/CCPlayerInventory.h"
    #include "cs2/interfaces/entity.h"
    #include "cs2/features/skins/skins.h"
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

    TraceShapeFn oTraceShape = nullptr;
    __int64 __fastcall hkTraceShape(__int64* p1, float* p2, float* pStart, float* pEnd, __int64* p5, __int64* pTrace) {
        __int64 result = oTraceShape(p1, p2, pStart, pEnd, p5, pTrace);

        if (globals::crosshairPenetration && pTrace) {
            float fraction = *(float*)((uintptr_t)pTrace + 0xAC);

            bool hitSomething = *(char*)((uintptr_t)pTrace + 0xB7) != 0;

            bool isHit = (fraction < 1.0f) || hitSomething;

            if (isHit) {
                if (result & 1) {
                    globals::g_PenetrationStatus = 2;
                }
                else {
                    globals::g_PenetrationStatus = 1; 
                }
            }
            else {
                globals::g_PenetrationStatus = 0; 
            }

            if (globals::g_PenetrationStatus == 2) {
                globals::dotColor[0] = 0.0f; globals::dotColor[1] = 1.0f; globals::dotColor[2] = 0.0f; globals::dotColor[3] = 1.0f;
            }
            else {
                globals::dotColor[0] = 1.0f; globals::dotColor[1] = 0.0f; globals::dotColor[2] = 0.0f; globals::dotColor[3] = 1.0f;
            }
        }

        return result;
    }

    LevelInitFn oLevelInit = nullptr;
    void __fastcall hkLevelInit(void* pClientModeShared, const char* szNewMap)
    {
        printf("[H] LevelInit called for map: %s\n", szNewMap);
        I::pvs->Set(false);
        oLevelInit(pClientModeShared, szNewMap);
    }



    /*void __fastcall GetScreenAspectRatio(void* thisptr, int width, int height) {

        if (SDK::g_ScreenHeight != height || SDK::g_ScreenWidth != width) {
            SDK::g_ScreenHeight = height;
            SDK::g_ScreenWidth = width;
            printf("[D] Screen resized: width=%d, height=%d\n", width, height);
        }
        H::hkGetAsspectRatio.GetOriginal()(thisptr, width, height);

    }*/
    FrameStageFn oFrameStage = nullptr;
    void __fastcall hkFrameStage(void* _this, int curStage)
    {
        oFrameStage(_this, curStage);
    
        if (!I::engine || !I::engine->is_in_game()) return;
        if (curStage == 6) {
            std::cout << "FrameStage running..." << std::endl;


            C_CSPlayerPawn* local_pawn = C_CSPlayerPawn::GetLocalPawn();
            if (!local_pawn) return;
            CCSPlayerInventory* inventory = CCSInventoryManager::GetInstance()->GetLocalInventory();

            C_CS2HudModelWeapon* viewModel = local_pawn->GetViewModel();
            S::set_agent(curStage);
            S::ApplyKnifeSkins(local_pawn, inventory, viewModel);
            S::ApplyWeaponSkins(local_pawn, inventory, viewModel);
            S::ApplyGloves();
        }
    }

    EquipItemInLoadoutFn oEquipItemInLoadout = nullptr;
    bool __fastcall hkEquipItemInLoadout(void* thisptr, int iTeam, int iSlot, uint64_t iItemID) {

        bool result = oEquipItemInLoadout(thisptr, iTeam, iSlot, iItemID);
        S::onEquipItemInLoadout(thisptr, iTeam, iSlot, iItemID);

        return result;
    }