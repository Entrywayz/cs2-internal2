#include <Windows.h>
#include <thread>
#include <iostream> 
#include <cstdio>  
#include "cs2/hooks/hooks.h"

void CreateConsole() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    SetConsoleTitleA("CS2 Internal Debug Console");
}

void ReleaseConsole() {
    fclose(stdout);
    FreeConsole();
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    CreateConsole();

    std::cout << "[+] DLL Injected successfully!" << std::endl;

    hooks::Init();

    std::cout << "[+] Hooks initialized." << std::endl;

    while (!GetAsyncKeyState(VK_DELETE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    hooks::Shutdown();

    std::cout << "[-] Shutting down..." << std::endl;

    ReleaseConsole();

    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}