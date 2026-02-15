#include "present.h"
#include "core/globals.h"
#include "cs2/features/visuals/visuals.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "cs2/features/aim/aim.h"
#include "cs2/features/skins/inventory.h"
#include "cs2/features/skins/skins.h"
#include "cs2/features/skins/inventory_p.h"
extern void RenderInventoryWindow();
extern void RenderSettingsWindow();
namespace W { void RenderSkinManagerWindow(); }
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

            INVENTORY::Dump(); // Собрать данные о предметах
            InventoryPersistence::LoadAndRecreate("added_items.json"); // Загрузить кастомные скины
            InventoryPersistence::Equip(); // Надеть те, что были надеты

            Initialized = true;

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
#include "imgui/imgui.h"
#include <string>
#include <vector>
#include <algorithm>

// Предположим, что эти данные доступны из ваших прошлых файлов:
// S::m_dumped_items — вектор структур c_dumped_item (результат INVENTORY::Dump())
// InventoryPersistence — ваш неймспейс для работы с JSON и CEconItem

#include "imgui/imgui.h"
#include <string>
#include <algorithm>
#include <vector>

void RenderInventoryWindow() {
    static char skinSearch[128] = "";

    // Поле поиска
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##search", "Поиск оружия или скина...", skinSearch, IM_ARRAYSIZE(skinSearch));

    ImGui::Spacing();

    // Область со списком
    if (ImGui::BeginChild("##inventory_scroll", ImVec2(0, 0), true)) {

        std::string filter = skinSearch;
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

        // 1. Итерируемся по предметам (оружию)
        for (auto& item : S::m_dumped_items) {

            // 2. Итерируемся по скинам, которые привязаны к этому оружию
            for (auto& skin : item.m_dumped_skins) {

                // Преобразуем const char* в std::string для поиска
                std::string itemName = item.m_name;
                std::string skinName = skin.m_name;

                std::string combined = itemName + " " + skinName;
                std::transform(combined.begin(), combined.end(), combined.begin(), ::tolower);

                // Фильтрация поиска
                if (!filter.empty() && combined.find(filter) == std::string::npos)
                    continue;

                // Уникальный ID для ImGui (индекс оружия + ID скина)
                ImGui::PushID(item.m_def_index ^ skin.m_id);

                // Текст в списке: "AK-47 | Vulcan"
                char label[256];
                snprintf(label, sizeof(label), "%s | %s", item.m_name, skin.m_name);

                if (ImGui::Selectable(label)) {
                    // Создаем структуру для записи в JSON
                    PersistedEconItem rec;
                    rec.itemID = 0; // Назначится автоматически в LoadAndRecreate
                    rec.defIndex = item.m_def_index;
                    rec.paintKit = (float)skin.m_id; // В вашем классе m_id — это PaintKit ID
                    rec.rarity = skin.m_rarity;
                    rec.quality = 4; // Genuine/Normal
                    rec.paintSeed = (float)(rand() % 1000);
                    rec.paintWear = 0.0001f; // Factory New
                    rec.legacy = true;

                    // Сохраняем названия для отображения в менеджере
                    rec.weaponTag = item.m_name;
                    rec.skinTag = skin.m_name;

                    // Путь к иконке (можете оставить пустым, если не используете картинки в меню)
                    rec.imagePath = "";

                    // Сохраняем в файл added_items.json
                    InventoryPersistence::Append(rec, "added_items.json");

                    // Мгновенно обновляем инвентарь в игре
                    InventoryPersistence::LoadAndRecreate("added_items.json");
                }

                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    }
}

HRESULT hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT flags)
{
    if (!Initialized) initHkPresent(pSwapChain, SyncInterval, flags);

    // --- Логика применения скинов (Каждый кадр) ---
    if (I::engine && I::engine->is_in_game()) {
        auto pLocalPawn = C_CSPlayerPawn::GetLocalPawn();
        auto pInventory = CCSPlayerInventory::GetInstance();

        if (pLocalPawn && pInventory) {
            S::ApplyGloves(); // Применяем перчатки
            S::ApplyWeaponSkins(pLocalPawn, pInventory, nullptr); // Применяем скины на оружие
            // S::set_agent(6); // Если нужен чейнджер агентов
        }
    }
    
    if (ImGui::BeginTabItem("Skins Search")) {
        RenderInventoryWindow(); // Поиск и добавление новых скинов
        ImGui::EndTabItem();
    }

    globals::g_PenetrationStatus = 0;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

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

    if (globals::showMenu) {
        ImGui::Begin("Menu");

        if (ImGui::BeginTabBar("MainTabs")) {

            if (ImGui::BeginTabItem("Rage")) {
                ImGui::Checkbox("AIM", &globals::aimEnabled);
                ImGui::SliderFloat("FOV", &globals::aimbotFov, 0.0f, 180.0f);
                ImGui::Checkbox("BHOP", &globals::bhopEnabled);
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
                ImGui::Checkbox("Glow", &globals::glowEnabled);

                if (globals::glowEnabled) {
                    ImGui::SameLine();
                    ImGui::ColorEdit4("Glow Color", (float*)&globals::glowColor,
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

                ImGui::Checkbox("Chams", &globals::chamsEnabled);
                if (globals::chamsEnabled) {
                    ImGui::SameLine();
                    ImGui::ColorEdit4("Glow Color", (float*)&globals::glowColor,
                        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                }

                ImGui::Checkbox("Third Person", &globals::thirdPersonEnabled);
                if (globals::thirdPersonEnabled) {
                    ImGui::SliderFloat("Distance", &globals::thirdPersonDistance, 0.0f, 300.0f);
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