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

float4 PS_Inverse(VertexOutput input) : SV_Target
{
	return float4(1 - DiffuseMap.Sample(LinearSampler, input.Uv).rgb, 1.0f);

}
float4 PS_GrayScale(VertexOutput input) : SV_Target
{
	float3 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv).rgb;
	float average = (diffuse.r + diffuse.g + diffuse.b) / 3.0f;
	return float4(average,average,average,1.0f);

}
float4 PS_GrayScale2(VertexOutput input) : SV_Target
{
	float3 grayScale = float3(0.2627f, 0.6780f, 0.0593f);
	float3 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv).rgb;
	float average = dot(diffuse, grayScale);
	
	return float4(average, average, average, 1.0f);

}
float3x3 ColorToSepiaMatrix = float3x3
(
	0.393f, 0.768f, 0.189f,//R
	0.349f, 0.686f, 0.168f,//G
	0.272f, 0.534f, 0.131f//B
);

float4 PS_Green(VertexOutput input) : SV_Target
{
	float3 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv).rgb;
	
	return float4(mul(diffuse, ColorToSepiaMatrix), 1.9f);

}
float4 PS_Sepia(VertexOutput input) : SV_Target
{
	float3 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv).rgb;
	
	return float4(mul(ColorToSepiaMatrix,diffuse), 1.9f);

}


float Saturation = 0;
float4 PS_Saturation(VertexOutput input) : SV_Target
{
	float3 grayScale = float3(0.2126f, 0.7152f, 0.0722f);
	
	float4 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv);
	float average = dot(diffuse.rgb, grayScale);

	
	diffuse.rgb = lerp(average, diffuse.rgb, Saturation);
	diffuse.a = 1.0f;
	
	//Saturation = 0 : average(그레이스케일)
	//0~1 : desaturation (그레이~ 원본)
	//1 : original(원본색)
	// >1 :saturation(채도 강조)
	
	return diffuse;
	
}

float Sharpness = 0;
float2 PixelSize;
float4 PS_Sharpness(VertexOutput input) :SV_Target
{

	float4 center = DiffuseMap.Sample(LinearSampler, input.Uv);
	float4 bottom = DiffuseMap.Sample(LinearSampler, input.Uv + float2(0, -PixelSize.y));
	float4 top = DiffuseMap.Sample(LinearSampler, input.Uv + float2(0, +PixelSize.y));
	float4 left = DiffuseMap.Sample(LinearSampler, input.Uv + float2(-PixelSize.x, 0));
	float4 right = DiffuseMap.Sample(LinearSampler, input.Uv + float2(+PixelSize.x, 0));

	float4 edge = center * 4 - left - right - top - bottom;

	float3 grayScale = float3(0.2126f, 0.7152f, 0.0722f);
	float average = dot(edge.rgb, grayScale);
	
	return center+ float4(average, average, average, 1.0f) * Sharpness;
	//return float4(edge.rgb, 1.0f);

}
float Power = 2;
float2 Scale = float2(2, 2);
float4 PS_Vignette(VertexOutput input) :SV_Target
{
	float4 color = DiffuseMap.Sample(LinearSampler, input.Uv);
	
	float radius = length((input.Uv - 0.5f) * 2.0f/Scale);
	float vignette = pow(abs(radius + 1e-6f), Power);
	
	return saturate(1-vignette) * color;

}

float Strength = 1.0f;
int InterlaceValue = 2;
float4 PS_Interlace(VertexOutput input) : SV_Target
{
	float4 color = DiffuseMap.Sample(LinearSampler, input.Uv);
	
	float height = 1.0f / PixelSize.y;
	
	int value = (int) ((floor(input.Uv.y * height) % InterlaceValue) / (InterlaceValue / 2));
	
	[flatten]
	if (value)
	{
		float3 grayScale = float3(0.2126f, 0.7152f, 0.0722f);
		float luminance = dot(color.rgb, grayScale);

		luminance = min(0.99f, luminance);

		color.rgb = lerp(color.rgb, color.rgb * luminance,Strength);
	}
	return color;
}

float LensPower = 1.0f;
float3 Distortion = -0.02f;
float4 PS_LensDistortion(VertexOutput input) : SV_Target
{
	float2 uv = input.Uv * 2 - 1;
	
	float2 vpSize = float2(1.0f / PixelSize.x, 1.0f / PixelSize.y);

	float aspect = vpSize.x / vpSize.y;
	float radiusSquared = aspect * aspect + uv.x * uv.x + uv.y * uv.y;
	float radius = sqrt(radiusSquared);
	
	float3 f = Distortion * pow(abs(radius + 0.0001f), LensPower) + 1;
	
	float2 r = (f.r * uv + 1) * 0.5f;
	float2 g = (f.g * uv + 1) * 0.5f;
	float2 b = (f.b * uv + 1) * 0.5f;
	
	float4 color = 0;
	color.r = DiffuseMap.Sample(LinearSampler, r).r;
	color.g = DiffuseMap.Sample(LinearSampler, g).g;
	color.b = DiffuseMap.Sample(LinearSampler, b).b;
	
	return color;
}
float2 WiggleOffset = float2(10, 10);
float2 WiggleAmount = float2(10, 10);

float4 PS_Wiggle(VertexOutput input) : SV_Target
{
	float2 uv = input.Uv;
	uv.x += sin(Time + uv.x * WiggleOffset.x) * WiggleAmount.x;
	uv.y += sin(Time + uv.y * WiggleOffset.y) * WiggleAmount.y;
	
	return DiffuseMap.Sample(LinearSampler, uv);
}
technique11 T0
{
	P_VP(P0,VS,PS_Diffuse)
	P_VP(P1,VS,PS_Inverse)
	P_VP(P2,VS,PS_GrayScale)
	P_VP(P3, VS, PS_GrayScale2)
	P_VP(P4, VS, PS_Green)
	P_VP(P5, VS, PS_Sepia)
	P_VP(P6, VS, PS_Saturation)
	P_VP(P7,VS,PS_Sharpness)
	P_VP(P8, VS, PS_Vignette)
	P_VP(P9, VS, PS_Interlace)
	P_VP(P10, VS, PS_LensDistortion)
	P_VP(P11, VS, PS_Wiggle)

}