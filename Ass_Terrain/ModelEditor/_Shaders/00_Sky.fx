DepthStencilState SkyDepthStencil;

///////////////////////////////////////////////////////////////////////////////

float4 PS_CubeSky(MeshOutput input) : SV_Target0
{
    return SkyCubeMap.Sample(LinearSampler, input.oPosition);
}

///////////////////////////////////////////////////////////////////////////////

static const float EarthInnerRadius = 6356.7523142f;
static const float EarthOuterRadius = 6356.7523142f * 1.0157313f;

static const float KrESun = 0.0025f * 20.0f; //레일리 상수 * 태양의 밝기
static const float KmESun = 0.0010f * 20.0f; //미 상수 * 태양의 밝기
static const float Kr4PI = 0.0025f * 4.0f * 3.1415159f; //'구'로 변환
static const float Km4PI = 0.0010f * 4.0f * 3.1415159f; //'구'로 변환

static const float2 RayleighMieScaleHeight = { 0.25f, 0.1f }; //레일리, 미 산란에 대한 크기 & 높이
static const float EarthScale = 1.0f / (6356.7523142f * 1.0157313f - 6356.7523142f);

static const float SunExposure = -2.0f;

static const float FloatOne = 0.00390625f; //float에 대한 최소 정확도
static const float FloatHalf = 0.001953125f; //..를 반으로 나눔

///////////////////////////////////////////////////////////////////////////////

struct ScatteringDesc
{
    float3 WaveLength;
    float Padding;

    float3 InvWaveLength;
    int SamepleCount;

    float3 WaveLengthMie;
};

cbuffer CB_Scattering
{
    ScatteringDesc Scattering;
};

///////////////////////////////////////////////////////////////////////////////
// Scattering
///////////////////////////////////////////////////////////////////////////////
struct VertexOutput_Scattering
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
};

VertexOutput_Scattering VS_Scattering(VertexTexture input)
{
    VertexOutput_Scattering output;

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

float2 GetDensityRatio(float height)
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

    float sampleLength = far / Scattering.SamepleCount;
    float scaleLength = sampleLength * EarthScale;

    float3 sampleRay = direction * sampleLength;
    p1 += sampleRay * 0.5f;

    for (int i = 0; i < Scattering.SamepleCount; i++)
    {
        float height = length(p1);
        opticalDepth += GetDensityRatio(height);
        
        p1 += sampleRay;
    }

    return opticalDepth * scaleLength;
}

struct PixelOutput_Scattering
{
    float4 R : SV_Target0;
    float4 M : SV_Target1;
};

PixelOutput_Scattering PS_Scattering(VertexOutput_Scattering input)
{
    PixelOutput_Scattering output;

    float3 sunDirection = -normalize(GlobalLight.Direction);
    float2 uv = input.Uv;

    float3 pointPv = float3(0, EarthInnerRadius + 1e-3f, 0.0f);
    float angleXZ = PI * uv.y;
    float angleY = 100.0f * uv.x * PI / 180.0f;

    //구면 좌표계 방향
    float3 direction;
    direction.x = sin(angleY) * cos(angleXZ);
    direction.y = cos(angleY);
    direction.z = sin(angleY) * sin(angleXZ);
    direction = normalize(direction);

    float farPvPa = HitOuterSphere(pointPv, direction);
    float3 ray = direction;

    float3 pointP = pointPv;
    float sampleLength = farPvPa / Scattering.SamepleCount;
    float scaledLength = sampleLength * EarthScale;
    float3 sampleRay = ray * sampleLength;
    pointP += sampleRay * 0.5f;

    //산란 회수
    float3 rayleighSum = 0;
    float3 mieSum = 0;
    for (int i = 0; i < Scattering.SamepleCount; i++)
    {
        float pHeight = length(pointP);

        float2 densityRatio = GetDensityRatio(pHeight);
        densityRatio *= scaledLength;

        float2 viewOpticalDepth = GetDistance(pointP, pointPv);

        float farPPC = HitOuterSphere(pointP, sunDirection);
        float2 sunOpticalDepth = GetDistance(pointP, pointP + sunDirection * farPPC);

        float2 opticalDepthP = sunOpticalDepth.xy + viewOpticalDepth.xy;
        float3 attenuation = exp(-Kr4PI * Scattering.InvWaveLength * opticalDepthP.x - Km4PI * opticalDepthP.y);

        rayleighSum += densityRatio.x * attenuation;
        mieSum += densityRatio.y * attenuation;

        pointP += sampleRay;
    }

    float3 rayleigh = rayleighSum * KrESun;
    float3 mie = mieSum * KmESun;

    rayleigh *= Scattering.InvWaveLength;
    mie *= Scattering.WaveLengthMie;

    output.R = float4(rayleigh, 1);
    output.M = float4(mie, 1);

    return output;

}

///////////////////////////////////////////////////////////////////////////////
// Draw SkyDome
///////////////////////////////////////////////////////////////////////////////
struct VertexOutput_Dome
{
    float4 Position : SV_Position;
    float3 oPosition : Position1;
    float2 Uv : Uv0;
};

VertexOutput_Dome VS_Dome(VertexTexture input)
{
    VertexOutput_Dome output;

    output.oPosition = -input.Position.xyz;
    output.Position = WorldPostion(input.Position);
    output.Position = ViewProjetion(output.Position);

    output.Uv = input.Uv;

    return output;
}

VertexOutput_Dome VS_PreRender_Reflection_Dome(VertexTexture input)
{
    VertexOutput_Dome output;

    output.oPosition = -input.Position.xyz;
    output.Position = WorldPostion(input.Position);
    output.Position = mul(output.Position, Reflection);
    output.Position = mul(output.Position, Projection);

    output.Uv = input.Uv;

    return output;
}


float GetRayleightPhase(float c)
{
    return 0.75f * (1.0f + c);
   
}

float GetMiePhase(float c, float c2)
{
    float3 result = 0;
    result.x = 1.5f * ((1.0f - G2) / (2.0f + G2));
    result.y = 1.0f + G2;
    result.z = 2.0f * G;

    return result.x * (1.0f + c2) / pow(result.y - result.z * c, 1.5f);
}

float3 HDR(float3 LDR)
{
    return 1.0f - exp(SunExposure * LDR);
}

Texture2D RayleighMap;
Texture2D MieMap;
Texture2D StarMap;

float4 PS_Dome(VertexOutput_Dome input) : SV_Target
{
    float3 sunDirection = -normalize(GlobalLight.Direction);

    float temp = dot(sunDirection, input.oPosition) / length(input.oPosition);
    float temp2 = temp * temp;

    float3 rSamples = RayleighMap.Sample(LinearSampler, input.Uv).rgb;
    float3 mSamples = MieMap.Sample(LinearSampler, input.Uv).rgb;

    float3 color = 0;
    color = GetRayleightPhase(temp2) * rSamples + GetMiePhase(temp, temp2) * mSamples;
    color = HDR(color);

    color += max(0, (1 - color.rgb)) * float3(0.05f, 0.05f, 0.1f);

    float starIntensity = GlobalLight.Direction.y;
    return float4(color, 1) + StarMap.Sample(LinearSampler, input.Uv) * saturate(starIntensity);

}

///////////////////////////////////////////////////////////////////////////////

struct VertexOutput_Moon
{
    float4 Position : SV_Position;
    float2 Uv : Uv;
};

VertexOutput_Moon VS_Moon(VertexTexture input)
{
    VertexOutput_Moon output;

    output.Position = WorldPostion(input.Position);
    output.Position = ViewProjetion(output.Position);
    output.Uv = input.Uv;

    return output;
}

VertexOutput_Moon VS_PreRender_Reflection_Moon(VertexTexture input)
{
    VertexOutput_Moon output;

    output.Position = WorldPostion(input.Position);
    output.Position = mul(output.Position, Reflection);
    output.Position = mul(output.Position, Projection);

    output.Uv = input.Uv;

    return output;
}

Texture2D MoonMap;
float MoonAlpha;
float4 PS_Moon(VertexOutput_Moon input) : SV_Target
{
    float4 color = MoonMap.Sample(LinearSampler, input.Uv);
    color.a *= MoonAlpha;

    return color;
}

///////////////////////////////////////////////////////////////////////////////

cbuffer CB_Cloud
{
    float CloudTiles;
    float CloudCover;
    float CloudSharpness;
    float CloudSpeed;
};

struct VertexOUtput_Cloud
{
    float4 Position : SV_Position;
    float2 Uv : Uv0;
    float2 oUv : Uv1;
};

VertexOUtput_Cloud VS_Cloud(VertexTexture input)
{
    VertexOUtput_Cloud output;

    output.Position = WorldPostion(input.Position);
    output.Position = ViewProjetion(output.Position);

    output.Uv = (input.Uv * CloudTiles);
    output.oUv = input.Uv;

    return output;
}

VertexOUtput_Cloud VS_PreRender_Reflection_Cloud(VertexTexture input)
{
    VertexOUtput_Cloud output;

    output.Position = WorldPostion(input.Position);
    output.Position = mul(output.Position, Reflection);
    output.Position = mul(output.Position, Projection);

    output.Uv = (input.Uv * CloudTiles);
    output.oUv = input.Uv;

    return output;
}

float Fade(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

texture2D CloudMap;
float Noise(float2 P)
{
    float2 Pi = FloatOne * floor(P) + FloatHalf;
    float2 Pf = frac(P);

    float2 grad00 = CloudMap.Sample(LinearSampler, Pi).rg * 4.0 - 1.0;
    float n00 = dot(grad00, Pf);

    float2 grad10 = CloudMap.Sample(LinearSampler, Pi + float2(FloatOne, 0.0)).rg * 4.0 - 1.0;
    float n10 = dot(grad10, Pf - float2(1.0, 0.0));

    float2 grad01 = CloudMap.Sample(LinearSampler, Pi + float2(0.0, FloatOne)).rg * 4.0 - 1.0;
    float n01 = dot(grad01, Pf - float2(0.0, 1.0));

    float2 grad11 = CloudMap.Sample(LinearSampler, Pi + float2(FloatOne, FloatOne)).rg * 4.0 - 1.0;
    float n11 = dot(grad11, Pf - float2(1.0, 1.0));

    float2 n_x = lerp(float2(n00, n01), float2(n10, n11), Fade(Pf.x));

    float n_xy = lerp(n_x.x, n_x.y, Fade(Pf.y));

    return n_xy;
}

float4 PS_Cloud(VertexOUtput_Cloud input) : SV_Target
{
    input.Uv = input.Uv * CloudTiles;

    float n = Noise(input.Uv + Time * CloudSpeed);
    float n2 = Noise(input.Uv * 2 + Time * CloudSpeed);
    float n3 = Noise(input.Uv * 4 + Time * CloudSpeed);
    float n4 = Noise(input.Uv * 8 + Time * CloudSpeed);

    float nFinal = n + (n2 / 2) + (n3 / 4) + (n4 / 8);

    float c = CloudCover - nFinal;

    if (c < 0)
        c = 0;

    float density = 1.0f - pow(CloudSharpness, c);
    float4 color = density;

    return color;
    
}