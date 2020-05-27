DepthStencilState SkyDepthStencil;
//////////////////////////////////////////////////////////////////
float4 PS_CubeSky(MeshOutput input) : SV_Target0
{
	return SkyCubeMap.Sample(LinearSampler, input.oPosition);
}
///////////////////////////////////////
static const float EarthInnerRadius = 6536.7523142f;
static const float EarthOuterRadius = 6536.7523142f* 1.0157313f;

static const float KrESun = 0.0025f * 20.0f; //Rayleigh * Brightness 레일리상수 * 태양의 밝기
static const float KmESun = 0.0010f * 20.0f; //미 상수 * 태양의 밝기
static const float Kr4PI = 0.0025f * 4.0f * 3.1415159f; //'구'로 변환
static const float Km4PI = 0.0010f * 4.0f * 3.1415159f; //'구'로 변환

static const float2 RayleighMieScaleHeight = { 0.25f, 0.1f }; //레일리, 미 산란에 대한 크기 & 높이
static const float EarthScale = 1.0f / (6536.7523142f * 1.0157313f - 6536.7523142f);

static const float SunExposure = -2.0f;

static const float FloatOne = 0.00390624f; //float에 대한 최소 정확도
static const float FloatHalf = 0.001953124f; //..를 반으로 나눔

///////////////////////////////////////////////////////////////////////////////////////////

struct ScatteringDesc
{
	float3 WaveLength;
	int SampleCount;
	
	float3 InvWaveLength;
	float StarIntensity;
	
	float3 WaveLengthMie;
	float MoonAlpha;
};

cbuffer CB_Scattering
{
	ScatteringDesc scattering;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Scattering
////////////////////////////////////////////////////////////////////////////////////////////////////
struct VertexOutput_Scaterring
{
	float4 Position : SV_Position0;
	float2 Uv : Uv0;
};

VertexOutput_Scaterring VS_Scattering(VertexTexture input)
{
	VertexOutput_Scaterring output;
	
	output.Position = input.Position;
	output.Uv = input.Uv;
	
	return output;
}

float HitOuterSphere(float3 position, float3 direction)
{
	float3 light = -position;
	
	float b = dot(light, direction);
	float c = dot(light, light);
	
	float d = c - b * b;
	float q = sqrt(EarthOuterRadius * EarthOuterRadius - d);
	
	return b + q;
}

float2 GetDensityRatio(float height)//밀도
{
	float altitude = (height - EarthInnerRadius) * EarthScale;
	
	return exp(-altitude / RayleighMieScaleHeight);
}

float2 GetDistance(float3 p1, float3 p2)
{
	float2 opticalDepth = 0;
	
	float3 temp = p2 - p1;
	float far = length(temp);
	float3 direction = temp / far;
	
	float sampleLength = far / scattering.SampleCount;
	float scaleLength = sampleLength * EarthScale;
	
	float3 sampleRay = direction * sampleLength;
	p1 += sampleRay * 0.5f;
	
	for (int i = 0; i < scattering.SampleCount; i++)
	{
		float height = length(p1);
		opticalDepth = GetDensityRatio(height);

		p1 += sampleRay;
	}
	
	return opticalDepth * scaleLength;
	
}

struct PixelOutput_Scattering
{
	float4 R : SV_Target0;
	float4 M : SV_Target1;
};
PixelOutput_Scattering PS_Scattering(VertexOutput_Scaterring input)
{
	PixelOutput_Scattering output;
	
	float3 sunDirection = -normalize(GlobalLight.Direction);
	float2 uv = input.Uv;
	
	float3 pointPv = float3(0, EarthInnerRadius + 1e-3f, 0.0f);
	float angleXZ = PI * uv.y;
	float angleY = 100.0f * uv.x * PI / 180.0f;
	
	float3 direction;
	direction.x = sin(angleY) * cos(angleXZ);
	direction.y = cos(angleY);
	direction.z = sin(angleY) * sin(angleXZ);
	direction = normalize(direction);
	
	float farPvpa = HitOuterSphere(pointPv, direction);
	float3 ray = direction;
	
	float pointP = pointPv;
	float3 sampleLength = farPvpa / scattering.SampleCount;
	float scaledLength = sampleLength * EarthScale;
	float3 sampleRay = ray * sampleLength;
	pointP += sampleRay * 0.5f;
	
	//산란 횟수
	float3 rayleighSum = 0;
	float3 mieSum = 0;
	for (int i = 0; i < scattering.SampleCount; i++)
	{
		float pHeight = length(pointP);
		
		float2 densityRatio = GetDensityRatio(pHeight);
		densityRatio *= scaledLength;
		
		float2 viewOpticalDepth = GetDistance(pointP, pointPv);

		float farPPC = HitOuterSphere(pointP, sunDirection);
		float2 sunOpticalDepth = GetDistance(pointP,pointP + sunDirection* farPPC);
		
		float2 OpticalDepthP = sunOpticalDepth.xy + viewOpticalDepth.xy;
		float3 attenuation = exp(-Kr4PI * scattering.InvWaveLength * OpticalDepthP.x - Km4PI * OpticalDepthP.y);
		rayleighSum += densityRatio.x * attenuation;
		mieSum += densityRatio.y * attenuation;
		
		pointP += sampleRay;
	}

	float3 rayleigh = rayleighSum * KrESun;
	float3 mie = mieSum * KmESun;
	
	rayleigh *= scattering.InvWaveLength;
	mie *= scattering.WaveLengthMie;
	
	output.R = float4(rayleigh, 1);
	output.M = float4(mie, 1);
	
	return output;
}