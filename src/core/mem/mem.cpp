#include "mem.h"

#include <Windows.h>
#include <Psapi.h>
#include <vector>

std::vector<std::pair<uint8_t, bool>> PatternToBytes(const std::string& pattern) {
    std::vector<std::pair<uint8_t, bool>> patternBytes;
    const char* start = pattern.c_str();
    const char* end = start + pattern.size();

    for (const char* current = start; current < end; ++current) {
        if (*current == ' ') continue;
        if (*current == '?') {
            patternBytes.emplace_back(0, false);
            if (*(current + 1) == '?') ++current;
        }
        else {
            patternBytes.emplace_back(strtoul(current, nullptr, 16), true);
            if (*(current + 1) != ' ') ++current;
        }
    }

    return patternBytes;
}

uintptr_t Mem::PatternScan(const std::string& pattern, const std::string& modStr) {
    if (modStr.empty())
        return 0;

    auto mod = GetModuleHandle(modStr.c_str());
    if (!mod)
        return 0;

    MODULEINFO modInfo;
    GetModuleInformation(GetCurrentProcess(), mod, &modInfo, sizeof(MODULEINFO));

    size_t moduleSize = modInfo.SizeOfImage;
    uintptr_t moduleBase = (uintptr_t)modInfo.lpBaseOfDll;

    std::vector<std::pair<uint8_t, bool>> patternBytes = PatternToBytes(pattern);
    size_t patternLength = patternBytes.size();

    for (size_t i = 0; i < moduleSize - patternLength; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternLength; ++j) {
            if (patternBytes[j].second && patternBytes[j].first != *reinterpret_cast<uint8_t*>(moduleBase + i + j)) {
                found = false;
                break;
            }
        }
        if (found) {
            return moduleBase + i;
        }
    }

    return 0;
}

Mem::PtrValidator::CacheEntry Mem::PtrValidator::cache[Mem::PtrValidator::CACHE_SIZE];
DWORD Mem::PtrValidator::last_check = 0;

bool Mem::PtrValidator::CheckAndCache(void* ptr) {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0)
        return false;

    size_t index = (reinterpret_cast<uintptr_t>(ptr) >> 12) % CACHE_SIZE;
    cache[index] = {
        mbi.BaseAddress,
        mbi.RegionSize,
        mbi.State,
        mbi.Protect,
        GetTickCount()
    };

    return (mbi.State == MEM_COMMIT) && ((mbi.Protect & PAGE_NOACCESS) == 0);
}

bool Mem::PtrValidator::IsValidPtr(void* ptr) {
    if (!ptr) return false;

    size_t index = (reinterpret_cast<uintptr_t>(ptr) >> 12) % CACHE_SIZE;
    auto& entry = cache[index];

    DWORD current_time = GetTickCount();
    if (entry.base &&
        current_time - entry.time < 1000 &&
        reinterpret_cast<uintptr_t>(ptr) >= reinterpret_cast<uintptr_t>(entry.base) &&
        reinterpret_cast<uintptr_t>(ptr) < reinterpret_cast<uintptr_t>(entry.base) + entry.region_size) {

        return (entry.state == MEM_COMMIT) && ((entry.protect & PAGE_NOACCESS) == 0);
    }

    return CheckAndCache(ptr);
}