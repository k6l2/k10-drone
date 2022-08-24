#pragma once
#include "korl-interface-platform.h"
#include "korl-stringPool.h"
typedef struct GameMemory
{
    bool continueProgramExecution;
    Korl_Gfx_Camera camera;
    Korl_Memory_AllocatorHandle allocatorStack;
    Korl_Memory_AllocatorHandle allocatorHeap;
    bool mouseRotateCamera;
    Korl_Bluetooth_QueryEntry* stbDaLastBluetoothQuery;
    Korl_Bluetooth_SocketHandle bluetoothSocket;
    Korl_StringPool stringPool;
} GameMemory;
