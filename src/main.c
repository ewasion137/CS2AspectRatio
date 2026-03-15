#include <windows.h>
#include <stdint.h>

#define VIEW_MATRIX_OFFSET 0x230EF20

// Функция для изменения матрицы
void ModifyViewMatrix() {
    uintptr_t client_base = (uintptr_t)GetModuleHandleA("client.dll");
    if (!client_base) return;

    float* matrix = (float*)(client_base + VIEW_MATRIX_OFFSET);

    DWORD old_protect;
    VirtualProtect(matrix, 64, PAGE_EXECUTE_READWRITE, &old_protect);

    matrix[0] = 1.5f; // Растягиваем X

    VirtualProtect(matrix, 64, old_protect, &old_protect);
}

// Поток, который будет ждать нажатия клавиши
DWORD WINAPI MainThread(LPVOID lpParam) {
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
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Создаем поток, чтобы не блокировать игру при загрузке
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, NULL, 0, NULL);
        break;
        
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}