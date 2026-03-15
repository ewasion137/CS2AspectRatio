#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define VIEW_MATRIX_OFFSET 0x230EF20
HMODULE hModule; // Храним хендл для выгрузки
bool is_stretching = false; // Состояние чита

// Функция, которая постоянно "долбит" память
void ApplyStretch() {
    uintptr_t client_base = (uintptr_t)GetModuleHandleA("client.dll");
    if (!client_base) return;

    float* matrix = (float*)(client_base + VIEW_MATRIX_OFFSET);

    // Защита памяти
    DWORD old_protect;
    VirtualProtect(matrix, 64, PAGE_EXECUTE_READWRITE, &old_protect);

    // Если включено - пишем 1.5f, если выключено - 1.0f (стандарт)
    matrix[0] = is_stretching ? 1.5f : 1.0f; 

    VirtualProtect(matrix, 64, old_protect, &old_protect);
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    printf("Cheat Loaded! Press INSERT to Toggle, DELETE to Unload.\n");

    while (1) {
        // Toggle (Вкл/Выкл)
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            is_stretching = !is_stretching;
            printf("Stretching: %s\n", is_stretching ? "ON" : "OFF");
        }

        // Выгрузка
        if (GetAsyncKeyState(VK_DELETE) & 1) {
            printf("Unloading...\n");
            // Возвращаем матрицу в нормальное состояние перед выходом
            is_stretching = false;
            ApplyStretch();
            break; // Выходим из цикла
        }

        // Постоянно применяем патч, если включено
        if (is_stretching) {
            ApplyStretch();
        }

        Sleep(10); // Спим чуть-чуть, чтобы не грузить проц на 100%
    }

    // Правильная выгрузка
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hDllModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        hModule = hDllModule; // Сохраняем хендл
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, NULL, 0, NULL);
    }
    return TRUE;
}