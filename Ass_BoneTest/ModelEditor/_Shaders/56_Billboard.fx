#include "00_Global.fx"
#include "00_Light.fx"
#include "00_Model.fx"

struct VertexInput
{
    float4 Position : Position;
    float2 Scale : Scale;

    uint MapIndex : MapIndex;
};

struct VertexOutput
{
    float4 Position : Position;
    float2 Scale : Scale;

	uint MapIndex : MapIndex;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;

    //input.Position = float4(input.VertexID * 2.0f, 0, 0, 1);
    //input.Scale = float2(1, 1);

    output.Position = WorldPostion(input.Position);
    output.Scale = input.Scale;

	output.MapIndex = input.MapIndex;

    return output;

}

struct GeometryOutput
{
    float4 Position : SV_Position;
    float2 Uv : Uv;

	uint MapIndex : MapIndex;
};

[maxvertexcount(4)]
void GS(point VertexOutput input[1], inout TriangleStream<GeometryOutput> stream)
{
    float3 up = float3(0, 1, 0);
    //float3 forward = float3(0, 0, 1);
    float3 forward = input[0].Position.xyz - ViewPosition();
    float3 right = normalize(cross(up, forward));

    float2 size = input[0].Scale * 0.5f;

    float4 position[4];
    position[0] = float4(input[0].Position.xyz - size.x * right - size.y * up, 1);
    position[1] = float4(input[0].Position.xyz - size.x * right + size.y * up, 1);
    position[2] = float4(input[0].Position.xyz + size.x * right - size.y * up, 1);
    position[3] = float4(input[0].Position.xyz + size.x * right + size.y * up, 1);

    float2 uv[4] =
    {
        float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0)
    };

	
    GeometryOutput output;

    [unroll(4)]
    for (int i = 0; i < 4; i++)
    {
        output.Position = ViewProjetion(position[i]);
        output.Uv = uv[i];
		output.MapIndex = input[0].MapIndex;

        stream.Append(output);
    }

}

[maxvertexcount(8)]
void GS_CrossQuad(point VertexOutput input[1], inout TriangleStream<GeometryOutput> stream)
{
	float3 up = float3(0, 1, 0);
    //float3 forward = float3(0, 0, 1);
	float3 forward = float3(0, 0, 1);
	float3 right = normalize(cross(up, forward));

	float2 size = input[0].Scale * 0.5f;

	float4 position[8];
	position[0] = float4(input[0].Position.xyz - size.x * right - size.y * up, 1);
	position[1] = float4(input[0].Position.xyz - size.x * right + size.y * up, 1);
	position[2] = float4(input[0].Position.xyz + size.x * right - size.y * up, 1);
	position[3] = float4(input[0].Position.xyz + size.x * right + size.y * up, 1);
	
	position[4] = float4(input[0].Position.xyz - size.x * forward - size.y * up, 1);
	position[5] = float4(input[0].Position.xyz - size.x * forward + size.y * up, 1);
	position[6] = float4(input[0].Position.xyz + size.x * forward - size.y * up, 1);
	position[7] = float4(input[0].Position.xyz + size.x * forward + size.y * up, 1);

	float2 uv[4] =
	{
		float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0)
	};

	int i = 0;
	GeometryOutput output;
	
	
    [unroll(8)]
	for (i = 0; i < 8; i++)
	{
		output.Position = ViewProjetion(position[i]);
		output.Uv = uv[i%4];
		output.MapIndex = input[0].MapIndex;

		stream.Append(output);
		[flatten]
		if (i == 3)
			stream.RestartStrip();
	}

}

Texture2DArray Maps;
float4 PS(GeometryOutput input) : SV_Target0
{
	float4 diffuse = Maps.Sample(LinearSampler, float3(input.Uv, input.MapIndex));
    
	float3 color = diffuse.rgb * dot(float3(0, 1, 0), -GlobalLight.Direction) * 1.5f;
	
	return float4(color, diffuse.a);
}

technique11 T0
{
	P_BS_VGP(P0, AlphaBlend_AlphaToCoverage, VS, GS, PS)
	P_RS_BS_VGP(P1,CullMode_None,AlphaBlend_AlphaToCoverage,VS,GS_CrossQuad,PS)
}