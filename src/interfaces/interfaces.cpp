#include "interfaces.h"
#include "CGameEntitySystem/CGameEntitySystem.h"
#include "../features/movement/archilix.hpp"
#include "..\..\archilix\utils\memory\Interface\Interface.h"
#include "../i_csgo_input.hpp"
#include "xor.hpp"
bool I::Interfaces::init()
{
    const HMODULE tier0_base = GetModuleHandleA("tier0.dll");
    if (!(tier0_base)) {
        printf("[ERROR] Failed to get tier0.dll module handle\n");
        return false;
    }
    bool success = true;
    InputSys = I::Get<IInputSystem>(("inputsystem.dll"), "InputSystemVersion00");
    if (!InputSys) {
        printf("[WARNING] Failed to get InputSystemVersion00\n");
        success = false;
    }
    EngineClient = I::Get<IEngineClient>(("engine2.dll"), "Source2EngineToClient00");
    if (!EngineClient) {
        printf("[WARNING] Failed to get Source2EngineToClient00\n");
        success = false;
    }
    GameEntity = I::Get<IGameResourceService>(("engine2.dll"), "GameResourceServiceClientV00");
    if (!GameEntity) {
        printf("[WARNING] Failed to get GameResourceServiceClientV00\n");
        success = false;
    } else {
        if (GameEntity->Instance) {
            int maxEntities = GameEntity->Instance->GetHighestEntityIndex();
            } else {
            }
    }
    ConstructUtlBuffer = reinterpret_cast<decltype(ConstructUtlBuffer)>(GetProcAddress(tier0_base, "??0CUtlBuffer@@QEAA@HHW4BufferFlags_t@0@@Z"));
    if (!ConstructUtlBuffer) printf("[WARNING] Failed to get ConstructUtlBuffer\n");
    EnsureCapacityBuffer = reinterpret_cast<decltype(EnsureCapacityBuffer)>(GetProcAddress(tier0_base, "?EnsureCapacity@CUtlBuffer@@QEAAXH@Z"));
    if (!EnsureCapacityBuffer) printf("[WARNING] Failed to get EnsureCapacityBuffer\n");
    PutUtlString = reinterpret_cast<decltype(PutUtlString)>(GetProcAddress(tier0_base, "?PutString@CUtlBuffer@@QEAAXPEBD@Z"));
    if (!PutUtlString) printf("[WARNING] Failed to get PutUtlString\n");
    CreateMaterial = reinterpret_cast<decltype(CreateMaterial)>(M::FindPattern("materialsystem2.dll", "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05"));
    if (!CreateMaterial) printf("[WARNING] Failed to find CreateMaterial pattern\n");
    LoadKeyValues = reinterpret_cast<decltype(LoadKeyValues)>(GetProcAddress(tier0_base, "?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEAVCUtlBuffer@@AEBUKV3ID_t@@PEBDI@Z"));
    if (!LoadKeyValues) printf("[WARNING] Failed to get LoadKeyValues\n");
    ConMsg = reinterpret_cast<decltype(ConMsg)>(GetProcAddress(tier0_base, "?ConMsg@@YAXPEBDZZ"));
    if (!ConMsg) printf("[WARNING] Failed to get ConMsg\n");
    ConColorMsg = reinterpret_cast<decltype(ConColorMsg)>(GetProcAddress(tier0_base, "?ConColorMsg@@YAXAEBVColor@@PEBDZZ"));
    if (!ConColorMsg) printf("[WARNING] Failed to get ConColorMsg\n");
    I::EntitySystem = *reinterpret_cast<CGameEntitySystem**>(
        M::FindPattern("client.dll", "48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B D3") + 3
        );
    printf("InputSystemVersion00: 0x%p\n", reinterpret_cast<void*>(InputSys));
    printf("Source2EngineToClient00: 0x%p\n", reinterpret_cast<void*>(EngineClient));
    printf("GameResourceServiceClientV00: 0x%p\n", reinterpret_cast<void*>(GameEntity));
    printf("CreateMaterial: 0x%p\n", reinterpret_cast<void*>(CreateMaterial));
    return success;
}
