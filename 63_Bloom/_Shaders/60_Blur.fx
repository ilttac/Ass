#include "00_Global.fx"
#include "00_Light.fx"

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

float2 PixelSize;
uint BlurCount = 8; //인접 픽셀개수
float4 PS_Blur(VertexOutput input) : SV_Target
{
	float2 offset[] =
	{
		float2(-1, -1), float2(+0, -1), float2(+1, -1),
		float2(-1, +0), float2(+0, +0), float2(+1, +0),
		float2(-1, +1), float2(+0, +1), float2(+1, +1)
	}; //인접픽셀위치
	
	float3 color = 0; //평균을 낼 색상의 정보
	
	for (uint blur = 0; blur < BlurCount; blur++)
	{
		[unroll(9)]
		for (int i = 0; i < 9; i++)
		{
			float2 xy = offset[i] *(float)blur* PixelSize;
			float2 uv = input.Uv + xy;
			color += DiffuseMap.Sample(LinearSampler, uv).rgb;
		}
	}
	color /= BlurCount * 9;
	
	return float4(color.rgb, 1);
}

uint RadialBlurCount = 8;
float RadialBlurRadius =0.0f;//집중선 원의 반경
float RadialBlurAmount = 0.04f;
float2 RadialCenter = float2(0.5f, 0.5f);

float4 PS_RadialBlur(VertexOutput input) : SV_Target
{
	float2 radius = input.Uv - RadialCenter;
	float2 r = length(radius) + 1e-6f;
	
	radius /= r;
	
	r = 2 * r/RadialBlurRadius;
	r = saturate(r);
	
	float2 delta = radius * r * r * RadialBlurAmount / RadialBlurCount;
	delta = -delta;
	
	float4 color = 0;
	for (uint i = 0; i < RadialBlurCount; i++)
	{
		color += DiffuseMap.Sample(LinearSampler, input.Uv);
		input.Uv += delta;
	}

	color /= RadialBlurCount;
	
	return float4(color.rgb, 1);

}

technique11 T0
{
	P_VP(P0,VS,PS_Diffuse)
	P_VP(P1,VS,PS_Blur)
	P_VP(P2,VS,PS_RadialBlur)
}