#include "present.h"
#include "core/globals.h"
#include "core/mem/mem.h"
#include "cs2/features/visuals/visuals.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "cs2/features/aim/aim.h"
#include "cs2/classes/EntityManager.h"
#include "cs2/offsets/offsets.hpp"

PresentFn oPresent = nullptr;
bool Initialized = false;
ImFont* mainFont = nullptr;
HWND window = nullptr;
WNDPROC oWndProc = nullptr;
ID3D11Device* pDevice = nullptr;
ID3D11DeviceContext* pContext = nullptr;
ID3D11RenderTargetView* mainRenderTargetView = nullptr;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN) {
        if (wParam == VK_INSERT) {
            globals::showMenu = !globals::showMenu;

            ImGuiIO& io = ImGui::GetIO();
            if (!globals::showMenu)
                io.MousePos = ImVec2(-1, -1);
        }
    }

    if (globals::showMenu) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

        
        switch (uMsg) {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
            return true;
        }
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}
void InitImGui() {
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    mainFont = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\arial.ttf",
        16.0f,
        nullptr,
        io.Fonts->GetGlyphRangesCyrillic()
    );
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);
}
HRESULT initHkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT flags) {
    if (!Initialized) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            InitImGui();
            Initialized = true;
        }
        else {
            return oPresent(pSwapChain, SyncInterval, flags);
        }
    }
}

double deg2rad(double degrees) {
    return degrees * M_PI / 180.0;
}

float skyColorArray[4];
HRESULT hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT flags)
{


    if (!Initialized) {
        initHkPresent(pSwapChain, SyncInterval, flags);
    }

    globals::g_PenetrationStatus = 0;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    EntityManager::Get().Update();
    if (globals::espEnabled) {
        visuals::PlayerESP();
    }

    if (globals::crosshairPenetration) {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(globals::dotColor[0], globals::dotColor[1], globals::dotColor[2], globals::dotColor[3]));

        ImGui::GetBackgroundDrawList()->AddCircleFilled(center, 2.0f, col);
    }

    if (globals::aimEnabled && globals::aimbotFov > 0.0f) {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImVec2 screenCenter = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);

        float radiusInPixels = ImGui::GetIO().DisplaySize.x * 0.5f * tanf(deg2rad(globals::aimbotFov));

        drawList->AddCircle(
            screenCenter,           
            radiusInPixels,
            IM_COL32(255, 255, 255, 128), 
            64,                   
            1.0f                   
        );
    }

    uintptr_t client = Mem::GetModuleBase("client.dll");
    if (client) {
        uintptr_t matrixAddr = client + cs2_dumper::offsets::client_dll::dwViewMatrix;
        if (Mem::PtrValidator::IsValidPtr((void*)matrixAddr)) {
            memcpy(&globals::ViewMatrix, (void*)matrixAddr, sizeof(globals::ViewMatrix));
        }
        uintptr_t anglesAddr = client + cs2_dumper::offsets::client_dll::dwViewAngles;
        if (Mem::PtrValidator::IsValidPtr((void*)anglesAddr)) {
            memcpy(&globals::viewAngles, (void*)anglesAddr, sizeof(globals::viewAngles));
        }
    }

    if (globals::showMenu) {
        ImGui::Begin("Menu");

        if (ImGui::BeginTabBar("MainTabs")) {

            if (ImGui::BeginTabItem("Rage")) {
                ImGui::Checkbox("Aim", &globals::aimEnabled);
                ImGui::Checkbox("Silent", &globals::silentEnabled);
                ImGui::SliderFloat("FOV", &globals::aimbotFov, 0.0f, 180.0f);
                ImGui::Checkbox("Bhop", &globals::bhopEnabled);

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Anti-Aim")) {
                ImGui::Checkbox("Enable Anti-Aim", &globals::bAntiAim);

                if (globals::bAntiAim) {
                    ImGui::SeparatorText("Pitch Settings");
                    const char* pitchTypes[] = { "None", "Down", "Up", "Zero" };
                    ImGui::Combo("Pitch Type", &globals::iPitchType, pitchTypes, IM_ARRAYSIZE(pitchTypes));

                    ImGui::SeparatorText("Yaw Settings");
                    const char* yawTypes[] = { "None", "Backwards", "Forwards" };
                    ImGui::Combo("Yaw Type", &globals::iBaseYawType, yawTypes, IM_ARRAYSIZE(yawTypes));
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Visuals")) {
                ImGui::Checkbox("ESP", &globals::espEnabled);
                ImGui::Checkbox("Box", &globals::boxEnabled);
                if (globals::boxEnabled) {
                    ImGui::SameLine();
                    ImGui::ColorEdit4("Box Color", (float*)&globals::boxCol,
                        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                }
                ImGui::Checkbox("Glow", &globals::glowEnabled);
                if (globals::glowEnabled) {
                    ImGui::SameLine();
                    ImGui::ColorEdit4("Glow Color", (float*)&globals::glowColor,
                        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                }
                ImGui::Checkbox("Skeleton", &globals::skeleton);
                if (globals::skeleton) {
                    ImGui::SameLine();
                    ImGui::ColorEdit4("Skeleton Color", (float*)&globals::skelCol,
                        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                    ImGui::SliderFloat("Thickness", &globals::skelThick, 0, 10);
                }
                ImGui::Checkbox("Health Bar", &globals::healthEnabled);
                ImGui::Checkbox("Name", &globals::nameEnabled);
                if (globals::nameEnabled) {
                    ImGui::SameLine();
                    ImGui::ColorEdit4("Name Color", (float*)&globals::nameCol,
                        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                }
                ImGui::Checkbox("Crosshair Penetration", &globals::crosshairPenetration);
                ImGui::Checkbox("Sky Color", &globals::skyBoxChange);

                if (globals::skyBoxChange) {
                    ImGui::SameLine();

                    skyColorArray[0] = globals::skyColor.x;
                    skyColorArray[1] = globals::skyColor.y;
                    skyColorArray[2] = globals::skyColor.z;
                    skyColorArray[3] = 1.0f;

                    if (ImGui::ColorEdit3("##SkyColor", skyColorArray,
                        ImGuiColorEditFlags_NoInputs |
                        ImGuiColorEditFlags_NoLabel |
                        ImGuiColorEditFlags_NoTooltip)) {

                        globals::skyColor.x = skyColorArray[0];
                        globals::skyColor.y = skyColorArray[1];
                        globals::skyColor.z = skyColorArray[2];
                    }
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    ImGui::Render();
    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return oPresent(pSwapChain, SyncInterval, flags);
}