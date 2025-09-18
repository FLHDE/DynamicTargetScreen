// This is a reimplementation of the Dynamic Target Screen plugin by Venemon.
// The plugin toggles the top-down view of the target wireframe when pressing the "Switch to Target" button.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define STDCALL __stdcall

#define TOP_DOWN_TARGET_VIEW                    (*(PBOOL) 0x679C18)

#define ADDR_GET_TARGET_STATUS_MINIMIZED        ((PBYTE) 0x4E2C25)
#define ADDR_UPDATE_SWITCH_TO_TARGET_SELECTED   ((PBYTE) 0x4E2C2D)
#define ADDR_SWITCH_TO_TARGET_BTN_HIDDEN        ((PBYTE) 0x4E36DA)

DWORD dummy;
#define ProtectX( addr, size ) \
    VirtualProtect( addr, size, PAGE_EXECUTE_READWRITE, &dummy )

#define RELOFS( from, to ) \
    *(PDWORD)((DWORD)(from)) = (DWORD)(to) - (DWORD)(from) - 4

#define CALL( from, to ) \
    *(PBYTE)(from) = 0xE8; \
    RELOFS( (DWORD)from+1, to )

BOOL g_targetDllLoaded;

typedef enum TARGETSTATUSVIEW
{
    STATUSVIEW_CONTACTLIST    = 0,
    STATUSVIEW_TARGET         = 1,
    STATUSVIEW_FORMATION      = 2,
    STATUSVIEW_FORCE_DWORD    = 0x7FFFFFFF
} TARGETSTATUSVIEW;

typedef struct TargetStatusHandler
{
    BYTE                x00[0x14C];
    BOOLEAN             windowOpen;     // 0x14C
    BYTE                x14D[0x177];
    TARGETSTATUSVIEW    selectedView;   // 0x2C4
} TargetStatusHandler;

BOOLEAN STDCALL GetTargetStatusMinimized_Hook( TargetStatusHandler* pHandler )
{
    // When the Target Status window is minimized, the wireframe is not visible, so don't toggle the top-down Target View.
    // If the window is open and the Target DLL is loaded, the target wireframe is always visible, so allow it to toggle.
    // Finally, if the window is open and the DLL is not loaded, only toggle the view if the target wireframe is visible.
    if (pHandler->windowOpen &&
        (g_targetDllLoaded || pHandler->selectedView == STATUSVIEW_TARGET))
    {
        TOP_DOWN_TARGET_VIEW = !TOP_DOWN_TARGET_VIEW;
    }

    return pHandler->windowOpen; // overwritten instruction
}

void Patch()
{
    ProtectX( ADDR_GET_TARGET_STATUS_MINIMIZED, 7 );
    *ADDR_GET_TARGET_STATUS_MINIMIZED = 0x57; // push edi (TargetStatusHandler*)
    CALL( ADDR_GET_TARGET_STATUS_MINIMIZED + 1, GetTargetStatusMinimized_Hook );

    if (g_targetDllLoaded = GetModuleHandle( "HudTarget.dll" ) != NULL)
    {
        // The Adv Wide HUD plugin hides the Switch To Target button, so make it visible again.
        ProtectX( ADDR_SWITCH_TO_TARGET_BTN_HIDDEN, 1 );
        *ADDR_SWITCH_TO_TARGET_BTN_HIDDEN = 0x08; // xor -> or

        // When the Switch To Target button is visible, make sure FL doesn't do anything when the user presses it.
        ProtectX( ADDR_UPDATE_SWITCH_TO_TARGET_SELECTED, 6 );
        memcpy( ADDR_UPDATE_SWITCH_TO_TARGET_SELECTED, "\x66\x0F\x1F\x44\x00\x00", 6 ); // nop
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
