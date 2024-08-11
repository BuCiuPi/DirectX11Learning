//--------------------------------------------------------------------------------------
// File: Tutorial04.cpp
//
// This application displays a 3D cube using Direct3D 11
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729721.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "BoxApplication.h"
#include "HillApplication.h"
#include "CylinderApplication.h"
#include "SphereApplication.h"
#include "ShapesApplication.h"
#include "HeartPlaneApplication.h"
#include "WaveApplication.h"
#include "LitApplication.h"
#include "CrateApplication.h"
#include "BlendingApplication.h"
#include "MirrorApplication.h"


using namespace DirectX;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MirrorApplication application(hInstance);

    if (!application.Init(nCmdShow))
    {
        return 0;
    }

    return application.Run();
}
