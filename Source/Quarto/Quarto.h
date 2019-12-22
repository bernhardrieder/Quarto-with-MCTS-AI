// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef IMGUI_API
#define IMGUI_ENABLED
#endif // IMGUI_API

#ifdef IMGUI_ENABLED
#include <imgui.h>
#endif // IMGUI_ENABLED

#if defined(UE_BUILD_DEBUG) || defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_TEST) || defined(UE_EDITOR)
#define DEBUG_BUILD
#endif