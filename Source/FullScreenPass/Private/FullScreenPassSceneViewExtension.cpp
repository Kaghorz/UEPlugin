#include "FullScreenPassSceneViewExtension.h"
#include "FullScreenPassShaders.h"

#include "FXRenderingUtils.h"
#include "PostProcess/PostProcessInputs.h"
#include "DynamicResolutionState.h"


static TAutoConsoleVariable<int32> CVarEnabled(
	TEXT("r.FSP"),
	1,
	TEXT("Controls Full Screen Pass plugin\n")
	TEXT(" 0: disabled\n")
	TEXT(" 1: enabled (default)"));

static TAutoConsoleVariable<float> CVarFocusDepth(
	TEXT("r.FSP.Emphasize.FocusDepth"),
	0.03f,
	TEXT("Manual focus depth in [0, 1]."));

static TAutoConsoleVariable<float> CVarFocusRangeDepth(
	TEXT("r.FSP.Emphasize.FocusRangeDepth"),
	0.93f,
	TEXT("Focus range depth in [0, 1]."));

static TAutoConsoleVariable<float> CVarFocusEdgeDepth(
	TEXT("r.FSP.Emphasize.FocusEdgeDepth"),
	0.050f,
	TEXT("Smooth transition edge depth in [0, 1]."));

static TAutoConsoleVariable<int32> CVarSpherical(
	TEXT("r.FSP.Emphasize.Spherical"),
	0,
	TEXT("Use spherical emphasis zone. 0: disabled, 1: enabled."));

static TAutoConsoleVariable<float> CVarSphereFieldOfView(
	TEXT("r.FSP.Emphasize.SphereFieldOfView"),
	75.0f,
	TEXT("Estimated camera field of view in degrees [1, 180]."));

static TAutoConsoleVariable<float> CVarSphereFocusHorizontal(
	TEXT("r.FSP.Emphasize.SphereFocusHorizontal"),
	0.5f,
	TEXT("Horizontal focus point in [0, 1]."));

static TAutoConsoleVariable<float> CVarSphereFocusVertical(
	TEXT("r.FSP.Emphasize.SphereFocusVertical"),
	0.5f,
	TEXT("Vertical focus point in [0, 1]."));

static TAutoConsoleVariable<float> CVarBlendColorR(
	TEXT("r.FSP.Emphasize.BlendColorR"),
	0.0f,
	TEXT("Blend color red component in [0, 1]."));

static TAutoConsoleVariable<float> CVarBlendColorG(
	TEXT("r.FSP.Emphasize.BlendColorG"),
	0.0f,
	TEXT("Blend color green component in [0, 1]."));

static TAutoConsoleVariable<float> CVarBlendColorB(
	TEXT("r.FSP.Emphasize.BlendColorB"),
	0.0f,
	TEXT("Blend color blue component in [0, 1]."));

static TAutoConsoleVariable<float> CVarBlendFactor(
	TEXT("r.FSP.Emphasize.BlendFactor"),
	0.0f,
	TEXT("Blend between greyscale and blend color in [0, 1]."));

static TAutoConsoleVariable<float> CVarEffectFactor(
	TEXT("r.FSP.Emphasize.EffectFactor"),
	1.0f,
	TEXT("Overall effect factor in [0, 1]."));


FFullScreenPassSceneViewExtension::FFullScreenPassSceneViewExtension(const FAutoRegister& AutoRegister) :
	FSceneViewExtensionBase(AutoRegister)
{

}

void FFullScreenPassSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	if (CVarEnabled->GetInt() == 0)
	{
		return;
	}

	Inputs.Validate();

	const FIntRect PrimaryViewRect = UE::FXRenderingUtils::GetRawViewRectUnsafe(View);

	FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

	if (!SceneColor.IsValid())
	{
		return;
	}

	const FScreenPassTextureViewport Viewport(SceneColor);

	FRDGTextureDesc SceneColorDesc = SceneColor.Texture->Desc;

	SceneColorDesc.Format = PF_FloatRGBA;
	FLinearColor ClearColor(0., 0., 0., 0.);
	SceneColorDesc.ClearValue = FClearValueBinding(ClearColor);

	FRDGTexture* ResultTexture = GraphBuilder.CreateTexture(SceneColorDesc, TEXT("FulllScreenPassResult"));
	FScreenPassRenderTarget ResultRenderTarget = FScreenPassRenderTarget(ResultTexture, SceneColor.ViewRect, ERenderTargetLoadAction::EClear);
	
	FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FFullScreenPassVS> ScreenPassVS(GlobalShaderMap);
	TShaderMapRef<FFullScreenPassPS> ScreenPassPS(GlobalShaderMap);

	FFullScreenPassPS::FParameters* Parameters = GraphBuilder.AllocParameters<FFullScreenPassPS::FParameters>();
	Parameters->View = View.ViewUniformBuffer;
	Parameters->SceneTexturesStruct = Inputs.SceneTextures;
	Parameters->FocusDepth = FMath::Clamp(CVarFocusDepth->GetFloat(), 0.0f, 1.0f);
	Parameters->FocusRangeDepth = FMath::Clamp(CVarFocusRangeDepth->GetFloat(), 0.0f, 1.0f);
	Parameters->FocusEdgeDepth = FMath::Clamp(CVarFocusEdgeDepth->GetFloat(), 0.0f, 1.0f);
	Parameters->Spherical = CVarSpherical->GetInt() != 0 ? 1u : 0u;
	Parameters->SphereFieldOfView = FMath::Clamp(CVarSphereFieldOfView->GetFloat(), 1.0f, 180.0f);
	Parameters->SphereFocusHorizontal = FMath::Clamp(CVarSphereFocusHorizontal->GetFloat(), 0.0f, 1.0f);
	Parameters->SphereFocusVertical = FMath::Clamp(CVarSphereFocusVertical->GetFloat(), 0.0f, 1.0f);
	Parameters->BlendColor = FVector3f(
		FMath::Clamp(CVarBlendColorR->GetFloat(), 0.0f, 1.0f),
		FMath::Clamp(CVarBlendColorG->GetFloat(), 0.0f, 1.0f),
		FMath::Clamp(CVarBlendColorB->GetFloat(), 0.0f, 1.0f));
	Parameters->BlendFactor = FMath::Clamp(CVarBlendFactor->GetFloat(), 0.0f, 1.0f);
	Parameters->EffectFactor = FMath::Clamp(CVarEffectFactor->GetFloat(), 0.0f, 1.0f);

	Parameters->RenderTargets[0] = ResultRenderTarget.GetRenderTargetBinding();

	AddDrawScreenPass(
		GraphBuilder,
		RDG_EVENT_NAME("FullScreenPassShader"),
		View,
		Viewport,
		Viewport,
		ScreenPassVS,
		ScreenPassPS,
		Parameters
	);

	AddCopyTexturePass(GraphBuilder, ResultTexture, SceneColor.Texture);
}