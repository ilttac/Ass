#include "00_Global.fx"
#include "00_Light.fx"
#include "00_Model.fx"

float4 PS(MeshOutput input) : SV_Target0
{
    float4 color = PS_AllLight(input);

    

    input.sPosition.xyz /= input.sPosition.w;

    [flatten]
    if (input.sPosition.x < -1.0f || input.sPosition.x > 1.0f ||
        input.sPosition.y < -1.0f || input.sPosition.y > 1.0f ||
        input.sPosition.z < 0.0f || input.sPosition.z > 1.0f)
        return color;

    input.sPosition.x = input.sPosition.x * 0.5f + 0.5f;
    input.sPosition.y = -input.sPosition.y * 0.5f + 0.5f;
    input.sPosition.z -= ShadowBias;

    float depth = 0.0f;
    float factor = 0.0f;

    [branch]
    if (ShadowQuality == 0)
    {
        depth = ShadowMap.Sample(LinearSampler, input.sPosition.xy).r;
        factor = (float) input.sPosition.z <= depth;
    }
    else if(ShadowQuality == 1)
    {
        depth = input.sPosition.z;
        factor = ShadowMap.SampleCmpLevelZero(ShadowSampler, input.sPosition.xy, depth).r;
    }    
    else if(ShadowQuality == 2)
    {
        depth = input.sPosition.z;

        float2 size = 1.0f / ShadowMapSize;
        float2 offsets[] =
        {
            float2(-size.x, -size.y), float2(0.0f, -size.y), float2(+size.x, -size.y),
            float2(-size.x, 0.0f), float2(0.0f, 0.0f), float2(+size.x, 0.0f),
            float2(-size.x, +size.y), float2(0.0f, +size.y), float2(+size.x, +size.y)
        };

        float2 uv = 0;
        float2 sum = 0;

        [unroll(9)]
        for (int i = 0; i < 9; i++)
        {
            uv = input.sPosition.xy + offsets[i];
            sum += ShadowMap.SampleCmpLevelZero(ShadowSampler, uv, depth).r;
        }

        factor = sum / 9.0f;

    }

    factor = saturate(factor + depth);
    return float4(color.rgb * factor, 1);
}


technique11 T0
{
    P_RS_VP(P0, FrontCounterCloskwise_True, VS_Depth_Mesh, PS_Depth)
    P_RS_VP(P1, FrontCounterCloskwise_True, VS_Depth_Model, PS_Depth)
    P_RS_VP(P2, FrontCounterCloskwise_True, VS_Depth_Animation, PS_Depth)

    ///P_VP(P0, VS_Depth_Mesh, PS_Depth)
    ///P_VP(P1, VS_Depth_Model, PS_Depth)
    ///P_VP(P2, VS_Depth_Animation, PS_Depth)

    P_VP(P3, VS_Mesh, PS)
    P_VP(P4, VS_Model, PS)
    P_VP(P5, VS_Animation, PS)
}