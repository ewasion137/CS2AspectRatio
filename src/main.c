#include <windows.h>
#include <stdint.h>
#include <stdio.h>

#define VIEW_MATRIX_OFFSET 0x230EF20

// Функция для изменения матрицы
void ModifyViewMatrix() {
    uintptr_t client_base = (uintptr_t)GetModuleHandleA("client.dll");
    if (!client_base) return;

    float* matrix = (float*)(client_base + VIEW_MATRIX_OFFSET);

    // Добавь проверку на валидность адреса
    if (IsBadReadPtr(matrix, 64)) return; 

    DWORD old_protect;
    VirtualProtect(matrix, 64, PAGE_EXECUTE_READWRITE, &old_protect);

    matrix[0] = 1.5f; // Пытаемся принудительно держать 1.5

    VirtualProtect(matrix, 64, old_protect, &old_protect);
}

// Поток, который будет ждать нажатия клавиши
DWORD WINAPI MainThread(LPVOID lpParam) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    printf("Debug: Client Base = %p\n", (void*)GetModuleHandleA("client.dll"));
    printf("Debug: Matrix Address = %p\n", (void*)((uintptr_t)GetModuleHandleA("client.dll") + VIEW_MATRIX_OFFSET));
    while (1) {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
             ModifyViewMatrix();
        }
        Sleep(100); 
    }
    return 0;
}

// ТОЧКА ВХОДА (Вот где должен быть твой switch)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        MessageBoxA(NULL, "DLL Injected!", "Status", MB_OK); // ЭТОТ МЕССЕЙДЖ ДОЛЖЕН ПОЯВИТЬСЯ
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, NULL, 0, NULL);
    }
    return TRUE;
}