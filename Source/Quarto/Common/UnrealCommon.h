#pragma once

#include "CoreMinimal.h"

using brBool = bool;

using brFloat = float;
using brDouble = double;

using brU64 = uint64;
using brU32 = uint32;
using brU16 = uint16;
using brU8 = uint8;

using brS64 = int64;
using brS32 = int32;
using brS16 = int16;
using brS8 = int8;

using PlayerId = brU32;

#define brFloatMax FLT_MAX
#define brFloatMin FLT_MIN

#define GETENUMSTRING(etype, evalue) ( (FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetEnumName((int32)evalue) : FString("Invalid - are you sure enum uses UENUM() macro?") )
