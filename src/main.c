#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Отключаем ожидание потоков, чтобы не блокировать инжект
        DisableThreadLibraryCalls(hModule);
        
        // Показываем MessageBox, чтобы знать, что DLL загрузилась
        MessageBoxA(NULL, "DLL Loaded successfully!", "CS2 AspectRatio", MB_OK | MB_ICONINFORMATION);
        break;
        
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}