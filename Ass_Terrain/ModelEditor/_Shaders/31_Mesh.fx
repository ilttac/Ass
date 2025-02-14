#include "00_Global.fx"
#include "00_Light.fx"
#include "00_Model.fx"

float4 PS(MeshOutput input) : SV_Target0
{
    float3 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv).rgb;
    float NdotL = dot(normalize(input.Normal), -GlobalLight.Direction);

    return float4(diffuse * NdotL, 1);
    //return float4(0, 1, 0, 1);
}

technique11 T0
{
    P_VP(P0, VS_Mesh, PS)
}