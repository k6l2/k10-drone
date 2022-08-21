#include "korl-interface-game.h"
#include "korl-interface-game-memory.h"
#include "korl-interface-platform.h"
KORL_INTERFACE_PLATFORM_API_DECLARE;
#define KORL_DEFINED_INTERFACE_PLATFORM_API
korl_global_variable GameMemory* fctd_memory;
KORL_GAME_API KORL_GAME_ON_RELOAD(korl_game_onReload)
{
    KORL_INTERFACE_PLATFORM_API_GET(korlApi);
    fctd_memory = memory;
}
KORL_GAME_API KORL_GAME_INITIALIZE(korl_game_initialize)
{
    korl_memory_zero(fctd_memory, sizeof(*fctd_memory));
    fctd_memory->continueProgramExecution = true;
    korl_gui_setFontAsset(L"submodules/korl/c/test-assets/source-sans/SourceSans3-Semibold.otf");
}
KORL_GAME_API KORL_GAME_ON_KEYBOARD_EVENT(korl_game_onKeyboardEvent)
{
}
KORL_GAME_API KORL_GAME_ON_MOUSE_EVENT(korl_game_onMouseEvent)
{
}
KORL_GAME_API KORL_GAME_ON_GAMEPAD_EVENT(korl_game_onGamepadEvent)
{
}
KORL_GAME_API KORL_GAME_UPDATE(korl_game_update)
{
    korl_gui_windowBegin(L"Connect Flight Controller Bluetooth", NULL, KORL_GUI_WINDOW_STYLE_FLAGS_DEFAULT);
        if(korl_gui_widgetButtonFormat(L"Query Paired Bluetooth Devices"))
        {
            ///@TODO: ???
        }
    korl_gui_windowEnd();
    return fctd_memory->continueProgramExecution;
}
KORL_GAME_API KORL_GAME_ON_ASSET_RELOADED(korl_game_onAssetReloaded)
{
}
