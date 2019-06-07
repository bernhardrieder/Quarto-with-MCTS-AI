// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef IMGUI_API
#define IMGUI_ENABLED 1
#else
#define IMGUI_ENABLED 0
#endif // IMGUI_API

#if IMGUI_ENABLED
#include <imgui.h>
#endif // WITH_IMGUI