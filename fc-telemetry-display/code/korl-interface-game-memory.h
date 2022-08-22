#pragma once
#include "korl-interface-platform.h"
typedef struct GameMemory
{
    bool continueProgramExecution;
    Korl_Gfx_Camera camera;
    Korl_Memory_AllocatorHandle allocatorStack;
    bool mouseRotateCamera;
} GameMemory;
