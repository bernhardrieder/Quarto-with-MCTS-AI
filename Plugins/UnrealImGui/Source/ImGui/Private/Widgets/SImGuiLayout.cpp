// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiPrivatePCH.h"

#include "SImGuiLayout.h"
#include "SImGuiWidget.h"

#include <Widgets/Layout/SConstraintCanvas.h>
#include <Widgets/Layout/SDPIScaler.h>
#include <Widgets/Layout/SScaleBox.h>


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SImGuiLayout::Construct(const FArguments& InArgs)
{
	checkf(InArgs._GameViewport, TEXT("Null Game Viewport argument"));

	GameViewport = InArgs._GameViewport;

	// TODO: Remove instantiation of ImGui Widget outside of this class.

	ChildSlot
	[
		// Remove accumulated scale to manually control how we draw data. 
		SNew(SScaleBox)
		.IgnoreInheritedScale(true)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			// Apply custom scale if necessary.
			// TODO: Bind to relevant parameter.
			SNew(SDPIScaler)
			.DPIScale(1.f)
			[
				SNew(SConstraintCanvas)
				+ SConstraintCanvas::Slot()
				.Anchors(FAnchors(0.f, 0.f, 1.f, 1.f))
				.AutoSize(true)
				.Offset(FMargin(1.f, 1.f, 0.f, 1.f))
				.Alignment(FVector2D::ZeroVector)
				[
					SNew(SImGuiWidget)
					.ModuleManager(InArgs._ModuleManager)
					.GameViewport(InArgs._GameViewport)
					.ContextIndex(InArgs._ContextIndex)
#if !ENGINE_COMPATIBILITY_LEGACY_CLIPPING_API
					// To correctly clip borders. Using SScissorRectBox in older versions seems to be not necessary.
					.Clipping(EWidgetClipping::ClipToBounds)
#endif
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
