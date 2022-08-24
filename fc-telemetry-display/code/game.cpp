#include "korl-interface-game.h"
#include "korl-interface-game-memory.h"
#include "korl-math.h"
KORL_INTERFACE_PLATFORM_API_DECLARE;
#define KORL_DEFINED_INTERFACE_PLATFORM_API
#include "korl-stb-ds.h"
using V3f32 = Korl_Math_V3f32;
using Quat  = Korl_Math_Quaternion;
using Batch = Korl_Gfx_Batch;
korl_global_variable GameMemory* fctd_memory;
korl_global_const f32 CAMERA_DISTANCE = 32;
#ifdef _LOCAL_STRING_POOL_POINTER
    #undef  _LOCAL_STRING_POOL_POINTER
#endif
#define _LOCAL_STRING_POOL_POINTER (&(fctd_memory->stringPool))
KORL_GAME_API KORL_GAME_ON_RELOAD(korl_game_onReload)
{
    KORL_INTERFACE_PLATFORM_API_GET(korlApi);
    fctd_memory = memory;
}
KORL_GAME_API KORL_GAME_INITIALIZE(korl_game_initialize)
{
    korl_memory_zero(fctd_memory, sizeof(*fctd_memory));
    fctd_memory->continueProgramExecution = true;
    fctd_memory->camera                   = korl_gfx_createCameraFov(90, 1, 1'000, {CAMERA_DISTANCE,0,0}, KORL_MATH_V3F32_ZERO);
    fctd_memory->allocatorStack           = korl_memory_allocator_create(KORL_MEMORY_ALLOCATOR_TYPE_LINEAR , korl_math_megabytes(8), L"fctd-stack", KORL_MEMORY_ALLOCATOR_FLAG_EMPTY_EVERY_FRAME   , NULL/*auto-select address*/);
    fctd_memory->allocatorHeap            = korl_memory_allocator_create(KORL_MEMORY_ALLOCATOR_TYPE_GENERAL, korl_math_megabytes(8), L"fctd-heap" , KORL_MEMORY_ALLOCATOR_FLAG_SERIALIZE_SAVE_STATE, NULL/*auto-select address*/);
    fctd_memory->stringPool               = korl_stringPool_create(fctd_memory->allocatorHeap);
    korl_gui_setFontAsset(L"submodules/korl/c/test-assets/source-sans/SourceSans3-Semibold.otf");
}
KORL_GAME_API KORL_GAME_ON_KEYBOARD_EVENT(korl_game_onKeyboardEvent)
{
    if(keyCode == KORL_KEY_ESCAPE && isDown && !isRepeat)
        fctd_memory->continueProgramExecution = false;
}
KORL_GAME_API KORL_GAME_ON_MOUSE_EVENT(korl_game_onMouseEvent)
{
    switch(mouseEvent.type)
    {
    case KORL_MOUSE_EVENT_BUTTON_DOWN:{
        switch(mouseEvent.which.button)
        {
        case KORL_MOUSE_BUTTON_RIGHT:{
            fctd_memory->mouseRotateCamera = true;
            break;}
        default:{break;}
        }
        break;}
    case KORL_MOUSE_EVENT_BUTTON_UP:{
        switch(mouseEvent.which.button)
        {
        case KORL_MOUSE_BUTTON_RIGHT:{
            fctd_memory->mouseRotateCamera = false;
            break;}
        default:{break;}
        }
        break;}
    case KORL_MOUSE_EVENT_WHEEL:{
        /* (de/in)crease camera FoV based on mouse scroll wheel amount */
        fctd_memory->camera.subCamera.perspective.fovHorizonDegrees += -0.1f*KORL_C_CAST(f32, mouseEvent.which.wheel);
        KORL_MATH_ASSIGN_CLAMP(fctd_memory->camera.subCamera.perspective.fovHorizonDegrees, 45, 125);
        break;}
    case KORL_MOUSE_EVENT_HWHEEL:{
        break;}
    case KORL_MOUSE_EVENT_MOVE:{
        break;}
    case KORL_MOUSE_EVENT_MOVE_RAW:{
        if(fctd_memory->mouseRotateCamera)
        {
            /* rotate the camera position around the +Z axis by the mouse X amount */
            fctd_memory->camera.position *= korl_math_quaternion_fromAxisRadians(KORL_MATH_V3F32_Z, -0.01f*KORL_C_CAST(f32, mouseEvent.x), true);
            /* rotate the camera position around the `+Z.cross(cameraPosition)` axis by the mouse Y amount */
            const V3f32 yawAxis = korl_math_v3f32_normal(korl_math_v3f32_cross(&KORL_MATH_V3F32_Z, &fctd_memory->camera.position));
            korl_assert(!korl_math_isNearlyZero(korl_math_v3f32_magnitudeSquared(&yawAxis)));
            // calculate the "forward" normal vector
            V3f32 forward = korl_math_v3f32_normal({ fctd_memory->camera.position.x
                                                   , fctd_memory->camera.position.y
                                                   , 0});
            korl_assert(!korl_math_isNearlyZero(korl_math_v3f32_magnitudeSquared(&forward)));
            // find the angle between the forward vector & the cam position => current pitch
            f32 pitchRadians = korl_math_v3f32_radiansBetween(forward, fctd_memory->camera.position);
            pitchRadians *= fctd_memory->camera.position.z >= 0 ? -1.f : 1.f;
            // modify pitch using raw mouse Y
            pitchRadians += -0.01f*KORL_C_CAST(f32, mouseEvent.y);
            // clamp the pitch to a reasonable range
            KORL_MATH_ASSIGN_CLAMP(pitchRadians, -KORL_PI32*0.45f, KORL_PI32*0.45f);
            // calculate the new camera position by pitching the forward vector around the yawAxis
            fctd_memory->camera.position = korl_math_quaternion_fromAxisRadians(yawAxis, pitchRadians, true)
                                         * (CAMERA_DISTANCE * forward);
        }
        break;}
    }
}
KORL_GAME_API KORL_GAME_ON_GAMEPAD_EVENT(korl_game_onGamepadEvent)
{
}
KORL_GAME_API KORL_GAME_UPDATE(korl_game_update)
{
#if KORL_DEBUG
    korl_gui_widgetTextFormat(L"camera.fov=%f", fctd_memory->camera.subCamera.perspective.fovHorizonDegrees);
    korl_gui_widgetTextFormat(L"mouseRotateCamera=%hs", fctd_memory->mouseRotateCamera ? "ON" : "OFF");
    korl_gui_widgetTextFormat(L"cameraPositionDistance=%f", korl_math_v3f32_magnitude(&fctd_memory->camera.position));
#endif
    if(fctd_memory->bluetoothSocket)
    {
        KORL_ZERO_STACK(au8, bluetoothData);
        Korl_Bluetooth_ReadResult resultRead;
        switch(resultRead = korl_bluetooth_read(fctd_memory->bluetoothSocket, fctd_memory->allocatorStack, &bluetoothData))
        {
        case KORL_BLUETOOTH_READ_SUCCESS:{
            if(!bluetoothData.size)
                break;
            Korl_StringPool_StringHandle tempString = string_newEmptyUtf16(0);
            for(u$ i = 0; i < bluetoothData.size; i++)
                string_appendFormat(tempString, L"0x%02X ", bluetoothData.data[i]);
            korl_log(VERBOSE, "bluetooth data: {%ws}", string_getRawUtf16(tempString));
            string_free(tempString);
            break;}
        case KORL_BLUETOOTH_READ_DISCONNECT:{
            fctd_memory->bluetoothSocket = NULL;
            break;}
        default:{
            korl_log(ERROR, "korl_bluetooth_read failed; result=%i", resultRead);
            break;}
        }
        bool flagConnected = true;
        korl_gui_windowBegin(L"Flight Controller Bluetooth Connection", &flagConnected, KORL_GUI_WINDOW_STYLE_FLAGS_DEFAULT);
            ///@TODO: add gui to send text data to the flight controller bluetooth modem (textEdit field)
        korl_gui_windowEnd();
        if(!flagConnected)
        {
            korl_bluetooth_disconnect(fctd_memory->bluetoothSocket);
            fctd_memory->bluetoothSocket = 0;
        }
    }
    else
    {
        korl_gui_windowBegin(L"Connect Flight Controller Bluetooth", NULL, KORL_GUI_WINDOW_STYLE_FLAGS_DEFAULT);
            if(korl_gui_widgetButtonFormat(L"Query Paired Bluetooth Devices"))
            {
                if(fctd_memory->stbDaLastBluetoothQuery)
                    mcarrfree(KORL_C_CAST(void*, fctd_memory->allocatorHeap), fctd_memory->stbDaLastBluetoothQuery);
                fctd_memory->stbDaLastBluetoothQuery = korl_bluetooth_query(fctd_memory->allocatorHeap);
            }
            for(u$ b = 0; b < arrlenu(fctd_memory->stbDaLastBluetoothQuery); b++)
            {
                if(korl_gui_widgetButtonFormat(fctd_memory->stbDaLastBluetoothQuery[b].name))
                    fctd_memory->bluetoothSocket = korl_bluetooth_connect(&fctd_memory->stbDaLastBluetoothQuery[b]);
                if(fctd_memory->bluetoothSocket)
                    break;
            }
        korl_gui_windowEnd();
    }
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
#undef _LOCAL_STRING_POOL_POINTER
#include "korl-math.c"
#include "korl-checkCast.c"
#define STB_DS_IMPLEMENTATION
#define STBDS_UNIT_TESTS // for the sake of detecting any other C++ warnings; we aren't going to actually run any of these tests
#define STBDS_ASSERT(x) korl_assert(x)
#include "stb/stb_ds.h"
#include "korl-stringPool.c"
