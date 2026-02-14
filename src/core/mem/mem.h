#pragma once
#include <cstdint>
#include <string>
#include <Windows.h>

namespace Mem {
    uintptr_t PatternScan(const std::string& pattern, const std::string& modStr);

    class PtrValidator {
    private:
        static constexpr size_t CACHE_SIZE = 4096;

        struct CacheEntry {
            void* base;
            size_t region_size;
            DWORD state;
            DWORD protect;
            DWORD time;
        };
        static CacheEntry cache[CACHE_SIZE];
        static DWORD last_check;

        static bool CheckAndCache(void* ptr);

    public:
        static bool IsValidPtr(void* ptr);
    };
}