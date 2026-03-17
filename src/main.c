#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "MinHook.h" // Убедись, что путь верный

// Значение для растяга. 
// 1.333f — это 4:3 (будет жирно на 16:9 мониторе)
// 1.0f — будет ОЧЕНЬ жирно
// 1.777f — стандарт 16:9
float my_aspect_ratio = 1.333333f;

// Структура рендера на основе данных с UnknownCheats
struct CViewRender {
    uint8_t  pad0[0x528];    // Смещение до AspectRatio
    float    flAspectRatio;  // 0x528
    uint8_t  pad1[0x71];     // Смещение до флагов
    uint8_t  nSomeFlags;     // 0x59D (0x528 + 4 + 0x71)
};

// Прототип оригинальной функции
typedef void* (__fastcall* CreateViewRender_t)(struct CViewRender* pViewRender);
CreateViewRender_t oCreateViewRender = NULL;

// Наш НОВЫЙ хук для растяга (Stretched)
void* __fastcall Hooked_CreateViewRender(struct CViewRender* pViewRender) {
    // 1. Вызываем оригинал, чтобы игра заполнила структуру своими данными
    void* ret = oCreateViewRender(pViewRender);

    if (pViewRender != NULL) {
        // 2. Устанавливаем наше значение
        pViewRender->flAspectRatio = my_aspect_ratio;

        // 3. САМОЕ ВАЖНОЕ: Ставим флаг "использовать кастомный Aspect Ratio"
        // Без этого бита игра просто проигнорирует наше значение
        pViewRender->nSomeFlags |= 2; 
    }

    return ret;
}

// Проверенный сканер паттернов
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

    printf("[*] CS2 FAT MODELS (True Stretched) - Loaded\n");

    if (MH_Initialize() != MH_OK) {
        printf("[!] MinHook failed to init!\n");
        return 0;
    }

    // 1. САМАЯ НОВАЯ сигнатура (Актуальная после последних апдейтов)
    // IDA: 48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 48
    const char* pattern1 = "\x48\x8B\xC4\x48\x89\x58\x00\x48\x89\x68\x00\x48\x89\x70\x00\x48\x89\x48";
    const char* mask1    = "xxxxxx?xxx?xxx?xxx";

    // 2. АЛЬТЕРНАТИВНАЯ сигнатура (Которая была до этого)
    // IDA: 48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B F1
    const char* pattern2 = "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x56\x57\x41\x56\x48\x83\xEC\x00\x48\x8B\xF1";
    const char* mask2    = "xxxx?xxxx?xxxxxxx?xxx";

    // 3. ТВОЯ СТАРАЯ сигнатура (на случай отката)
    const char* pattern3 = "\x48\x89\x5C\x24\x10\x48\x89\x6C\x24\x18\x56\x57\x41\x56\x48\x83\xEC\x00\x4C\x8B\xF1\x48\x8D\x94\x24\x90\x00\x00\x00";
    const char* mask3    = "xxxxxxxxxxxxxxxxx?xxxxxxxxxxx";

    // Ищем функцию перебирая актуальные сигнатуры
    uintptr_t target_addr = FindPattern("client.dll", pattern1, mask1);
    
    if (!target_addr) {
        target_addr = FindPattern("client.dll", pattern2, mask2);
    }
    if (!target_addr) {
        target_addr = FindPattern("client.dll", pattern3, mask3);
    }

    if (target_addr) {
        printf("[+] Found CreateViewRender at: %p\n", (void*)target_addr);

        if (MH_CreateHook((void*)target_addr, &Hooked_CreateViewRender, (void**)&oCreateViewRender) == MH_OK) {
            MH_EnableHook((void*)target_addr);
            printf("[SUCCESS] STRETCH HOOK ENABLED!\n");
        } else {
            printf("[ERROR] Failed to hook!\n");
        }
    } else {
        printf("[ERROR] Could not find signature in client.dll!\n");
    }

    printf("[*] Keys: UP/DOWN to change Stretch, DELETE to Unload.\n");

    while (!(GetAsyncKeyState(VK_DELETE) & 1)) {
        // Добавлено простенькое сглаживание кнопок (убрал залипание)
        if (GetAsyncKeyState(VK_UP) & 1) {
            my_aspect_ratio += 0.05f;
            printf("Target AspectRatio: %.3f\n", my_aspect_ratio);
        }
        if (GetAsyncKeyState(VK_DOWN) & 1) {
            my_aspect_ratio -= 0.05f;
            printf("Target AspectRatio: %.3f\n", my_aspect_ratio);
        }
        Sleep(10); // Снизил слип для лучшей реакции на клавиши
    }

    printf("[*] Unloading...\n");
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    
    // Даем потокам игры завершить вызовы оригинальной функции перед выгрузкой
    Sleep(200); 
    
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