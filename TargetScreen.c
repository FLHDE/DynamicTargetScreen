// TODO: Describe plugin and mention 00679C18 patch FL.exe (214470, 00 -> 01), and E15D3 = tooltip Ids HUD targetedobject (hover flag as well?)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define STDCALL __stdcall

#define TOP_DOWN_TARGET_VIEW    (*(PBOOL) 0x679C18)

#define ADDR_GET_TARGET_STATUS_MINIMIZED        ((PBYTE) 0x4E2C25)
#define ADDR_UPDATE_SWITCH_TO_TARGET_SELECTED   ((PBYTE) 0x4E2C2D)
#define ADDR_SWITCH_TO_TARGET_BTN_MINIMIZED     ((PBYTE) 0x4E36DA)

DWORD dummy;
#define ProtectX( addr, size ) \
    VirtualProtect( addr, size, PAGE_EXECUTE_READWRITE, &dummy )

#define RELOFS( from, to ) \
    *(PDWORD)((DWORD)(from)) = (DWORD)(to) - (DWORD)(from) - 4

#define CALL( from, to ) \
    *(PBYTE)(from) = 0xE8; \
    RELOFS( (DWORD)from+1, to )

BOOL g_targetDllLoaded;

typedef struct TargetStatusHandler
{
    BYTE    x00[0x14C];
    BOOLEAN windowMinimized;        // 0x14C
    BYTE    x14D[0x177];
    BOOL    switchToTargetSelected; // 0x2C4
} TargetStatusHandler;

BOOLEAN STDCALL GetTargetStatusMinimized_Hook( TargetStatusHandler* handler )
{
    if (handler->windowMinimized &&
        handler->switchToTargetSelected != g_targetDllLoaded)
    {
        TOP_DOWN_TARGET_VIEW = !TOP_DOWN_TARGET_VIEW;
    }

    return handler->windowMinimized; // overwritten instruction
}

void Patch()
{
    ProtectX( ADDR_GET_TARGET_STATUS_MINIMIZED, 7 );
    *ADDR_GET_TARGET_STATUS_MINIMIZED = 0x57;
    CALL( ADDR_GET_TARGET_STATUS_MINIMIZED + 1, GetTargetStatusMinimized_Hook );

    if (g_targetDllLoaded = GetModuleHandle( "HudTarget.dll" ) != NULL)
    {
        ProtectX( ADDR_UPDATE_SWITCH_TO_TARGET_SELECTED, 6 );
        ProtectX( ADDR_SWITCH_TO_TARGET_BTN_MINIMIZED, 1 );

        memcpy( ADDR_UPDATE_SWITCH_TO_TARGET_SELECTED, "\x66\x0F\x1F\x44\x00\x00", 6 );
        *ADDR_SWITCH_TO_TARGET_BTN_MINIMIZED = 0x08;
    }
}

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
    UNREFERENCED_PARAMETER( lpReserved );

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls( hinstDLL );
        Patch();
    }

    return TRUE;
}
