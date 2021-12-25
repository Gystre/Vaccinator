#include "pch.h"
#include "Hook.h"

/*

In here there are several things we need to do.
1. hook the functions that load and call the vac modules themselves to stop them
2. if any modules do make it through, make them load via loadlibrary so it exposes it's kernel32.dll
3. hook the IAT for kernel32.dll to inhibit GetProcAddress and GetSystemInfo as well as various other stuff
4. ????
5. Profit

*/

DWORD __stdcall start() {
    //wait for these guys to load
    while (!GetModuleHandleA("tier0_s.dll"))
        std::this_thread::sleep_for(std::chrono::milliseconds(25));

    while (!GetModuleHandleA("steamservice.dll"))
        std::this_thread::sleep_for(std::chrono::milliseconds(25));

    //init global and start da stuff
    SSDestroyer::hook = std::make_unique<Hook>();
    if (!SSDestroyer::hook->start())
        exit(EXIT_FAILURE);

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        _beginthreadex(nullptr, 0, reinterpret_cast<unsigned(__stdcall*)(void*)>(start), nullptr, 0, nullptr);
    }

    return TRUE;
}

