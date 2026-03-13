#pragma once
#include "CoreMinimal.h"
#include "ScreenPass.h"
#include "SceneTexturesConfig.h"


class FFullScreenPassVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFullScreenPassVS);

	FFullScreenPassVS() = default;
	FFullScreenPassVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer) {}
};

class FFullScreenPassPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FFullScreenPassPS);
	SHADER_USE_PARAMETER_STRUCT(FFullScreenPassPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		RENDER_TARGET_BINDING_SLOTS()
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneTextureUniformParameters, SceneTexturesStruct)
		SHADER_PARAMETER(float, FocusDepth)
		SHADER_PARAMETER(float, FocusRangeDepth)
		SHADER_PARAMETER(float, FocusEdgeDepth)
		SHADER_PARAMETER(uint32, Spherical)
		SHADER_PARAMETER(float, SphereFieldOfView)
		SHADER_PARAMETER(float, SphereFocusHorizontal)
		SHADER_PARAMETER(float, SphereFocusVertical)
		SHADER_PARAMETER(FVector3f, BlendColor)
		SHADER_PARAMETER(float, BlendFactor)
		SHADER_PARAMETER(float, EffectFactor)
	END_SHADER_PARAMETER_STRUCT()
};