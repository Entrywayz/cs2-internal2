#include "math.h"
#include <cstdint>
#include "cs2/features/visuals/visuals.h"
#include "cs2/offsets/client_dll.hpp"
#include "cs2/offsets/offsets.hpp"
#include "imgui/imgui.h"

bool WorldToScreen(Vec3 pos, Vec2& screen, float matrix[4][4]) {
    float w = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

    if (w < 0.001f) 
        return false;

    float x = matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3];
    float y = matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3];

    float invw = 1.0f / w;
    x *= invw;
    y *= invw;

    int width = (int)ImGui::GetIO().DisplaySize.x;
    int height = (int)ImGui::GetIO().DisplaySize.y;

    float x_res = width / 2.0f;
    float y_res = height / 2.0f;

    x_res += 0.5f * x * width + 0.5f;
    y_res -= 0.5f * y * height + 0.5f;

    screen.x = x_res;
    screen.y = y_res;

    return true;
}

