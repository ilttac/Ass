
struct LightDesc
{
	float4 Ambient;
	float4 Specular;
	float3 Direction;
	float Padding;
	float3 Position;
};


struct MaterialDesc
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Emissive;
};

cbuffer CB_Material
{
	MaterialDesc Material;
};

cbuffer CB_Light
{
	LightDesc GlobalLight;
};

float3 MaterialToColor(MaterialDesc result)
{
	return (result.Ambient + result.Diffuse + result.Specular + result.Emissive).rgb;
}

MaterialDesc MakeMaterial()
{
	MaterialDesc output;
	output.Ambient = float4(0, 0, 0, 0);
	output.Diffuse = float4(0, 0, 0, 0);
	output.Specular = float4(0, 0, 0, 0);
	output.Emissive = float4(0, 0, 0, 0);
    
	return output;
}

///////////////////////////////////////////////////////////////////////////////

void Texture(inout float4 color, Texture2D t, float2 uv, SamplerState samp)
{
	float4 sampling = t.Sample(samp, uv);
    
	color = color * sampling;
}

void Texture(inout float4 color, Texture2D t, float2 uv)
{
	Texture(color, t, uv, LinearSampler);
}

///////////////////////////////////////////////////////////////////////////////

void ComputeLight(out MaterialDesc output, float3 normal, float3 wPosition)
{
	output.Ambient = 0;
	output.Diffuse = 0;
	output.Specular = 0;

	float3 direction = -GlobalLight.Direction;
	float NdotL = dot(direction, normalize(normal));

	output.Ambient = GlobalLight.Ambient * Material.Ambient;
	float3 E = normalize(ViewPosition() - wPosition);
	
    [flatten]
	if (NdotL > 0.0f)
	{
		output.Diffuse = NdotL * Material.Diffuse;

        [flatten]
		if (any(Material.Specular.rgb))
		{
			float3 R = normalize(reflect(-direction, normal));
			float RdotE = saturate(dot(R, E));

			float specular = pow(RdotE, Material.Specular.a);
			output.Specular = specular * Material.Specular * GlobalLight.Specular;
		}
	}

	[flatten]
	if (any(Material.Emissive.rgb))
	{
		float NdotE = dot(E, normalize(normal));
		
		float emissive = smoothstep(1.0f - Material.Emissive, 1.0f, 1.0f - saturate(NdotE));

		output.Emissive = Material.Emissive * emissive;
	}
	
}


void NormalMapping(float2 uv, float3 normal, float3 tangent, SamplerState samp)
{
	float4 map = NormalMap.Sample(samp, uv);

    [flatten]
	if (any(map) == false)
		return;

	float3 N = normalize(normal);
	float3 T = normalize(tangent - dot(tangent, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);


	float3 coord = map.rgb * 2.0f - 1.0f;

	coord = mul(coord, TBN);
    
	Material.Diffuse *= saturate(dot(coord, -GlobalLight.Direction));
}

void NormalMapping(float2 uv, float3 normal, float3 tangent)
{
	NormalMapping(uv, normal, tangent, LinearSampler);
}
///////////////////////////////////////////////////////////////////////////////

struct PointLightDesc
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Emissive;
	
	float3 Position;
	float Range;
	
	float Intensity;
	float3 Padding;
};

#define MAX_POINT_LIGHT 128

cbuffer CB_PointLights
{
	uint PointLightCount;
	float3 CB_PointLights_Padding;
	
	PointLightDesc PointLights[MAX_POINT_LIGHT];
};

void ComputePointLight(inout MaterialDesc output, float3 normal, float3 wPosition)
{
	output = MakeMaterial();
	MaterialDesc result = MakeMaterial();
	
	for (uint i = 0; i < PointLightCount; i++)
	{
		float3 light = PointLights[i].Position - wPosition;
		float dist = length(light);
		
		[flatten]
		if (dist > PointLights[i].Range)
		{
			continue;
		}
		light /= dist;
		
		result.Ambient = Material.Ambient * PointLights[i].Ambient;
		
		float NdotL = dot(light, normalize(normal));
		float E = normalize(ViewPosition() - wPosition);
		
		[flatten]
		if (NdotL > 0.0f)
		{
			float3 R = normalize(reflect(-light, normal));
			float RdotE = saturate(dot(R, E));
			float specular = pow(RdotE, Material.Specular.a);//pong
			
			result.Diffuse = NdotL * Material.Diffuse * PointLights[i].Diffuse;
			result.Specular = specular * Material.Specular * PointLights[i].Specular;
		}

		float NdotE = dot(E, normalize(normal));
		float emissive = smoothstep(1.0f - Material.Emissive.a, 1.0f, 1.0f - saturate(NdotE));
		result.Emissive = emissive * Material.Emissive * PointLights[i].Emissive; //Rim

		//-point light
		float temp = 1.0f / saturate(dist / PointLights[i].Range); //거리에따라 작아지는 값
		
		float att = temp * temp * (1.0 / max(1 - PointLights[i].Intensity, 1e-8f));//temp* temp 구 수식

		output.Ambient += result.Ambient * temp;
		output.Diffuse += result.Diffuse * att;
		output.Specular += result.Diffuse * att;
		output.Emissive += result.Emissive * att;
	}

}