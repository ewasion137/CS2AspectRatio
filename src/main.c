#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "MinHook.h" // Убедись, что путь к инклудам правильный
#include <psapi.h>

// Структура рендера (из твоего источника)
struct CViewRender {
    unsigned char pad0[0x528];
    float flAspectRatio;
    unsigned char pad1[0x71];
    unsigned char nSomeFlags;
};

// Указатель на оригинал
void* (*oCreateViewRender)(struct CViewRender* pViewRender);

// Наш хук
void* __fastcall CreateViewRender_Hook(struct CViewRender* pViewRender) {
    // Выводим раз в 100 кадров, чтобы не убить производительность
    static int counter = 0;
    if (counter++ % 100 == 0) {
        printf("Hook Hit! AspectRatio currently: %f\n", pViewRender->flAspectRatio);
    }
    
    // Делаем изменения
    pViewRender->flAspectRatio = 2.5f; // Поставим 2.5, чтобы было ОЧЕНЬ заметно
    pViewRender->nSomeFlags |= 2;
    
    return oCreateViewRender(pViewRender);
}

// Простой сканер сигнатуры (ищет адрес функции по байтам)
uintptr_t FindPattern(const char* module_name, const char* pattern, const char* mask) {
    HMODULE hModule = GetModuleHandleA(module_name);
    MODULEINFO modInfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(MODULEINFO));
    
    uintptr_t start = (uintptr_t)modInfo.lpBaseOfDll;
    uintptr_t end = start + modInfo.SizeOfImage;

    for (uintptr_t i = start; i < end; i++) {
        bool found = true;
        for (size_t j = 0; j < strlen(mask); j++) {
            if (mask[j] != '?' && ((unsigned char*)i)[j] != ((unsigned char*)pattern)[j]) {
                found = false;
                break;
            }
        }
        if (found) return i;
    }
    return 0;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    // 1. Инициализация MinHook
    if (MH_Initialize() != MH_OK) return 0;

    // 2. Поиск адреса функции (используем сигнатуру из твоего UC)
    // "48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC"
    const char* sig = "\x48\x89\x5C\x24\x10\x48\x89\x6C\x24\x18\x56\x57\x41\x56\x48\x83\xEC";
    const char* mask = "?????????????????";
    uintptr_t addr = FindPattern("client.dll", sig, mask);

    if (addr) {
        printf("Found function at: %p\n", (void*)addr);
        
        MH_STATUS status = MH_CreateHook((void*)addr, &CreateViewRender_Hook, (void**)&oCreateViewRender);
        if (status != MH_OK) {
            printf("MH_CreateHook failed! Code: %d\n", status);
        }
        
        status = MH_EnableHook((void*)addr);
        if (status != MH_OK) {
            printf("MH_EnableHook failed! Code: %d\n", status);
        } else {
            printf("Hook enabled successfully!\n");
        }
    } else {
        printf("Failed to find pattern!\n");
    }

    // Ожидание DEL для выгрузки
    while (!(GetAsyncKeyState(VK_DELETE) & 1)) {
        Sleep(100);
    }

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    FreeConsole();
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, NULL);
    }
    return TRUE;
}