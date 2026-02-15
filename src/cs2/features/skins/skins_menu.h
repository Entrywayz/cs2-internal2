#pragma once
#include <string>
#include <vector>
#include <functional>

namespace SkinsMenu {
    // Инициализация системы скинов
    void Initialize();

    // Отрисовка вкладки скинов в меню
    void RenderTab();

    // Завершение работы
    void Shutdown();

    // Статус дампа скинов
    bool IsDumped();
}