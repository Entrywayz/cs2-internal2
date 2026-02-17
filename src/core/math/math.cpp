#include "math.h"
#include <cstdint>
#include "cs2/features/visuals/visuals.h"
#include "cs2/offsets/client_dll.hpp"
#include "cs2/offsets/offsets.hpp"
#include "imgui/imgui.h"

bool WorldToScreen(
    const Vec3& world,
    Vec2& screen,
    const float* matrix,
    float width,
    float height)
{
    float x = matrix[0] * world.x + matrix[4] * world.y + matrix[8] * world.z + matrix[12];
    float y = matrix[1] * world.x + matrix[5] * world.y + matrix[9] * world.z + matrix[13];
    float z = matrix[2] * world.x + matrix[6] * world.y + matrix[10] * world.z + matrix[14];
    float w = matrix[3] * world.x + matrix[7] * world.y + matrix[11] * world.z + matrix[15];

    if (w < 0.001f)
        return false;

    float inv_w = 1.0f / w;
    float x_ndc = x * inv_w;
    float y_ndc = y * inv_w;

    screen.x = (width * 0.5f) * (1.0f + x_ndc);
    screen.y = (height * 0.5f) * (1.0f - y_ndc); 

    return true;
}