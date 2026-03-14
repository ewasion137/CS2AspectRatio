#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Твой код здесь: создаем поток или патчим память
        MessageBoxA(NULL, "DLL Loaded in C!", "Success", MB_OK);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}