#include "00_Global.fx"
#include "00_Light.fx"
#include "00_Model.fx"

float4 PS(MeshOutput input) : SV_Target0
{
    float3 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv).rgb;
    float NdotL = dot(normalize(input.Normal), -GlobalLight.Direction);

    return float4(diffuse * NdotL,1);    
}
float4 PS_DepthSphere(MeshOutput input) : SV_Target0
{
	float3 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv).rgb;
	float NdotL = dot(normalize(input.Normal), -GlobalLight.Direction);

	return float4(diffuse * NdotL,0.8);
}

technique11 T0
{
    P_VP(P0, VS_Mesh, PS)
    P_VP(P1, VS_Model, PS)
    P_VP(P2, VS_Animation, PS)
	P_DSS_VP(P3, DepthEnable_False, VS_Mesh, PS)

}