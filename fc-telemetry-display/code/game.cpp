#include "korl-interface-game.h"
#include "korl-interface-game-memory.h"
#include "korl-math.h"
KORL_INTERFACE_PLATFORM_API_DECLARE;
#define KORL_DEFINED_INTERFACE_PLATFORM_API
using V3f32 = Korl_Math_V3f32;
using Batch = Korl_Gfx_Batch;
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
    fctd_memory->camera                   = korl_gfx_createCameraFov(90, 1, 1'000, {20,20,20}, KORL_MATH_V3F32_ZERO);
    fctd_memory->allocatorStack           = korl_memory_allocator_create(KORL_MEMORY_ALLOCATOR_TYPE_LINEAR, korl_math_megabytes(8), L"fctd-stack", KORL_MEMORY_ALLOCATOR_FLAG_EMPTY_EVERY_FRAME, NULL/*auto-select address*/);
    korl_gui_setFontAsset(L"submodules/korl/c/test-assets/source-sans/SourceSans3-Semibold.otf");
}
KORL_GAME_API KORL_GAME_ON_KEYBOARD_EVENT(korl_game_onKeyboardEvent)
{
    if(keyCode == KORL_KEY_ESCAPE && isDown && !isRepeat)
        fctd_memory->continueProgramExecution = false;
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
    korl_gfx_useCamera(fctd_memory->camera);
    Batch*const batchOrigin = korl_gfx_createBatchLines(fctd_memory->allocatorStack, 3);
    korl_gfx_batchSetLine(batchOrigin, 0, KORL_MATH_V3F32_ZERO, {1,0,0}, {255,  0,  0,255});
    korl_gfx_batchSetLine(batchOrigin, 1, KORL_MATH_V3F32_ZERO, {0,1,0}, {  0,255,  0,255});
    korl_gfx_batchSetLine(batchOrigin, 2, KORL_MATH_V3F32_ZERO, {0,0,1}, {  0,  0,255,255});
    korl_gfx_batchSetScale(batchOrigin, {25,25,25});
    korl_gfx_batch(batchOrigin, KORL_GFX_BATCH_FLAGS_NONE);
    return fctd_memory->continueProgramExecution;
}
KORL_GAME_API KORL_GAME_ON_ASSET_RELOADED(korl_game_onAssetReloaded)
{
}
#include "korl-math.c"
#include "korl-checkCast.c"
