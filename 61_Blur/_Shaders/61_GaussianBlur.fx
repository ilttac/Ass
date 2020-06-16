#include "00_Global.fx"
#include "00_Light.fx"

float2 PixelSize;

struct VertexOutput
{
	float4 Position : SV_Position;
	float2 Uv : Uv;
};

VertexOutput VS(float4 position : Position)
{
	VertexOutput output;
	
	output.Position = position;
	output.Uv.x = position.x * 0.5f + 0.5f;
	output.Uv.y = -position.y * 0.5f + 0.5f;
	
	return output;
}

float4 PS_Diffuse(VertexOutput input) : SV_Target
{
	return DiffuseMap.Sample(LinearSampler, input.Uv);
}

static const float Weights[13] =
{
 0.0561f, 0.1353f, 0.2780, 0.4868, 0.7261f,0.9231f,
 1.0f,
 0.9231,0.7261f, 0.4868, 0.2780, 0.1353f,0.0561f
};

float4 PS_GaussianBlurX(VertexOutput input) : SV_Target
{
	float2 uv = input.Uv;
	float u = PixelSize.x;
	
	float sum = 0;
	float4 color = 0;
	for (int i = -6; i <= 6; i++)
	{
		float2 temp = uv + float2(u * (float) i, 0.0f);
		color += DiffuseMap.Sample(LinearSampler, temp)* Weights[6+i];
		
		sum += Weights[6 + i];
	}
	color /= sum;

	return float4(color.rgb, 1.0f);
}

float4 PS_GaussianBlurY(VertexOutput input) : SV_Target
{
	float2 uv = input.Uv;
	float v = PixelSize.y;
	
	float sum = 0;
	float4 color = 0;
	for (int i = -6; i <= 6; i++)
	{
		float2 temp = uv + float2(0.0f,v * (float) i);
		color += DiffuseMap.Sample(LinearSampler, temp) * Weights[6 + i];
		
		sum += Weights[6 + i];
	}
	color /= sum;

	return float4(color.rgb, 1.0f);
}

technique11 T0
{
	P_VP(P0,VS,  PS_Diffuse)
	P_VP(P1, VS, PS_GaussianBlurX)
	P_VP(P2, VS, PS_GaussianBlurY)
}