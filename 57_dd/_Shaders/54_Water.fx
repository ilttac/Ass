#include "00_Global.fx"
#include "00_Deffered.fx"
#include "00_Model.fx"
#include "00_Sky.fx"

cbuffer CB_Water
{
	float4 RefractionColor;
	
	float2 NormalMapTile; //쪼개서 만들어서 물이 계속 반복하게 만듬.
	float WaveTranlation;
	float WaveScale;
	
	float WaterShiness;
	float WaterAlpha;
};

struct VertexOutput
{
	float4 Position : SV_Position;
	float3 wPosition : Position1;
	
	float2 Uv : Uv;
	float2 Uv2 : Uv1;
};

VertexOutput VS(VertexTexture input)
{
	VertexOutput output;
	output.Position = WorldPostion(input.Position);
	output.wPosition = output.Position.xyz;
	
	output.Position = ViewProjetion(output.Position);
	
	output.Uv = input.Uv / NormalMapTile.x;
	output.Uv2 = input.Uv / NormalMapTile.y;
	
	return output;
}

float4 PS(VertexOutput input) : SV_Target
{
	input.Uv.y += WaveTranlation;
	input.Uv2.x += WaveTranlation;
	
	float4 normalMap  = NormalMap.Sample(LinearSampler, input.Uv)  * 2.0f - 1.0f;
	float4 normalMap2 = NormalMap.Sample(LinearSampler, input.Uv2) * 2.0f - 1.0f;
	
	float3 normal = normalMap.rgb  + normalMap2.rgb;
	
	float3 light = GlobalLight.Direction;
	light.y *= -1.0f;
	light.z *= -1.0f;
	
	float3 view = normalize(ViewPosition() - input.wPosition);
	
	float3 diffuse = float3(0.0f, 0.0f, 0.3f);
	
	float3 R = normalize(reflect(light, normal));
	float specular = saturate(dot(R, view));
	
	[flatten]
	if (specular > 0.0f)
	{
		specular = pow(specular, WaterShiness);
		diffuse = saturate(diffuse + specular);
	}
	
	return float4(diffuse, WaterAlpha);
}

technique11 T0
{
    //Deffered-Depth
    P_RS_VP(P0, Deffered_Rasterizer_State, VS_Depth_Mesh, PS_Depth)
    P_RS_VP(P1, Deffered_Rasterizer_State, VS_Depth_Model, PS_Depth)
    P_RS_VP(P2, Deffered_Rasterizer_State, VS_Depth_Animation, PS_Depth)

    //Deffered-PreRender
    P_DSS_VP(P3, Deffered_DepthStencil_State, VS_Mesh, PS_PackGBuffer)
    P_DSS_VP(P4, Deffered_DepthStencil_State, VS_Model, PS_PackGBuffer)
    P_DSS_VP(P5, Deffered_DepthStencil_State, VS_Animation, PS_PackGBuffer)

    //Deffered-Directional
    P_DSS_VP(P6, Deffered_DepthStencil_State, VS_Directional, PS_Directional)

    //Deffered-PointLights
    P_RS_VTP(P7, Deffered_Rasterizer_State, VS_PointLights, HS_PointLights, DS_PointLights, PS_PointLights_Debug)
    P_RS_DSS_BS_VTP(P8, Deffered_Rasterizer_State, Deffered_DepthStencil_State, Blend_Addtive, VS_PointLights, HS_PointLights, DS_PointLights, PS_PointLights)

    //Deffered-SpotLights    
    P_RS_VTP(P9, Deffered_Rasterizer_State, VS_SpotLights, HS_SpotLights, DS_SpotLights, PS_SpotLights_Debug)
    P_RS_DSS_BS_VTP(P10, Deffered_Rasterizer_State, Deffered_DepthStencil_State, Blend_Addtive, VS_SpotLights, HS_SpotLights, DS_SpotLights, PS_SpotLights)

    //Sky
    //P_RS_DSS_VP(P11, FrontCounterCloskwise_True, SkyDepthStencil, VS_Mesh, PS_Sky)
    P_VP(P11, VS_Scattering, PS_Scattering)
	P_VP(P12, VS_Dome, PS_Dome)
	P_BS_VP(P13, AlphaBlend ,VS_Moon, PS_Moon)
	P_BS_VP(P14,AlphaBlend,VS_Cloud,PS_Cloud)

	//Water
	P_BS_VP(P15,AlphaBlend,VS,PS)
}

