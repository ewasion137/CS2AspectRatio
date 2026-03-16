#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "MinHook.h"

// Переменная для настройки (можешь потом вынести в конфиг)
float my_aspect_ratio = 35.0f;

// Оригинальная функция, которую мы перехватим
typedef float (__fastcall* GetAspectRatio_t)(void* rcx, int width, int height);
GetAspectRatio_t oGetAspectRatio = NULL;

// Наш Хук-заменитель
float __fastcall Hooked_GetAspectRatio(void* rcx, int width, int height) {
    // Если мы хотим выключить чит, можем вернуть оригинал:
    // return oGetAspectRatio(rcx, width, height);

    return my_aspect_ratio; // Возвращаем наше кастомное значение
}

// Надежный сканер паттернов
uintptr_t FindPattern(const char* module, const char* pattern, const char* mask) {
    HMODULE hMod = GetModuleHandleA(module);
    if (!hMod) return 0;

    MODULEINFO info;
    GetModuleInformation(GetCurrentProcess(), hMod, &info, sizeof(info));
    uintptr_t start = (uintptr_t)info.lpBaseOfDll;
    uintptr_t size = (uintptr_t)info.SizeOfImage;

    size_t pattern_len = strlen(mask);
    for (uintptr_t i = 0; i < size - pattern_len; i++) {
        bool found = true;
        for (size_t j = 0; j < pattern_len; j++) {
            if (mask[j] != '?' && ((uint8_t*)(start + i))[j] != ((uint8_t*)pattern)[j]) {
                found = false;
                break;
            }
        }
        if (found) return start + i;
    }
    return 0;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    printf("[*] CS2 Aspect Ratio - Elite Refactor\n");
    printf("[*] Initializing MinHook...\n");

    if (MH_Initialize() != MH_OK) {
        printf("[!] Failed to initialize MinHook\n");
        return 0;
    }

    // Сигнатура функции GetAspectRatio в engine2.dll
    // 48 89 5C 24 ? 57 48 83 EC ? 8B FA 48 8D 0D
    const char* pattern = "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x8B\xFA\x48\x8D\x0D";
    const char* mask    = "xxxx?xxxx?xxxxx";

    uintptr_t target_addr = FindPattern("engine2.dll", pattern, mask);

    if (target_addr) {
        printf("[+] Found target at: %p\n", (void*)target_addr);

        if (MH_CreateHook((void*)target_addr, &Hooked_GetAspectRatio, (void**)&oGetAspectRatio) == MH_OK) {
            MH_EnableHook((void*)target_addr);
            printf("[SUCCESS] Aspect Ratio Hook enabled!\n");
        } else {
            printf("[!] Failed to create hook.\n");
        }
    } else {
        printf("[ERROR] Could not find signature in engine2.dll!\n");
        printf("[TIP] Check if the game updated or search for signature manually in CE.\n");
    }

    printf("[*] Press DELETE to unload.\n");

    while (!(GetAsyncKeyState(VK_DELETE) & 1)) {
        // Здесь можно добавить управление на стрелочки:
        if (GetAsyncKeyState(VK_UP) & 1) {
            my_aspect_ratio += 0.1f;
            printf("Current AR: %.2f\n", my_aspect_ratio);
        }
        if (GetAsyncKeyState(VK_DOWN) & 1) {
            my_aspect_ratio -= 0.1f;
            printf("Current AR: %.2f\n", my_aspect_ratio);
        }
        Sleep(100);
    }

    printf("[*] Unloading...\n");
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    FreeConsole();
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID res) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hMod);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hMod, 0, 0);
    }
    return 1;
}