#include "00_Light.fx"

Texture2D GBufferMaps[6];

DepthStencilState Deffered_DepthStencil_State;
RasterizerState Deffered_Rasterizer_State;

////////////////////////////////////////////////////////////
//PackGBuffer
////////////////////////////////////////////////////////////

struct VertexOutput_PackGBuffer
{
	float4 Position : SV_Position;
	float2 Screen : Position1;
};

VertexOutput_PackGBuffer VS_Deffered(uint VertexID : SV_VertexID)
{
	VertexOutput_PackGBuffer output;

	output.Position = float4(NDC[VertexID], 0, 1);
	output.Screen = output.Position.xy;

	return output;
}
struct PixelOutput_PackGBuffer
{
	float4 Diffuse : SV_Target0;
	float4 Specular : SV_Target1;
	float4 Emissive : SV_Target2;
	float4 Normal : SV_Target3;
	float4 Tangent : SV_Target4;
};

PixelOutput_PackGBuffer PS_PackGBuffer(MeshOutput input)
{
	Texture(Material.Diffuse, DiffuseMap, input.Uv);
	Texture(Material.Specular, SpecularMap, input.Uv);
   
	PixelOutput_PackGBuffer output;

	output.Diffuse = float4(Material.Diffuse.rgb, 1);
	output.Specular = Material.Specular;
	output.Emissive = Material.Emissive;
	output.Normal = float4(input.Normal, 1);
	output.Tangent = float4(input.Tangent, 1);
	
	return output;
}

////////////////////////////////////////////////////////////
//UnpackGBuffer
////////////////////////////////////////////////////////////

void UnpackGBuffer(inout float4 position, in float2 screen, out MaterialDesc material, out float3 normal, out float3 tangent)
{
	material.Ambient = float4(0, 0, 0, 1);
	material.Diffuse = GBufferMaps[1].Load(int3(position.xy, 0));
	material.Specular = GBufferMaps[2].Load(int3(position.xy, 0));
	material.Emissive = GBufferMaps[3].Load(int3(position.xy, 0));

	normal = GBufferMaps[4].Load(int3(position.xy, 0)).rgb;
	tangent = GBufferMaps[5].Load(int3(position.xy, 0)).rgb;

	//float linearDepth = Deffered.Perspective.z / (depth + Deffered.Perspective.w);

	//position.xy = screen * Deffered.Perspective.xy * linearDepth;
	//position.z = linearDepth;
	//position.w = 1.0f;
	//position = mul(position, ViewInverse);

	float2 xy = 1.0f / float2(Projection._11, Projection._22);
	float z = Projection._43;
	float w = -Projection._33;
	
	float depth = GBufferMaps[0].Load(int3(position.xy, 0)).r;
	
	float linearDepth = z / (depth + w);
	
	position.xy = screen * xy * linearDepth;
	position.z = linearDepth;
	position.w = 1.0f;
	position = mul(position, ViewInverse);
	
}

////////////////////////////////////////////////////////////
//Directional
////////////////////////////////////////////////////////////
struct VertexOutput_Directional
{
	float4 Position : SV_Position;
	float2 Screen : Position1;
};


VertexOutput_PackGBuffer VS_Directional(uint VertexID : SV_VertexID)
{
	VertexOutput_PackGBuffer output;

	output.Position = float4(NDC[VertexID], 0, 1);
	output.Screen = output.Position.xy;

	return output;
}


void ComputeLight_Deffered(out MaterialDesc output, MaterialDesc material, float3 normal, float3 wPosition)
{
	output.Ambient = 0;
	output.Diffuse = 0;
	output.Specular = 0;
	output.Emissive = 0;

	normal = normalize(normal);

	float3 direction = -GlobalLight.Direction;
	float NdotL = dot(direction, normal);

	output.Ambient = GlobalLight.Ambient * material.Ambient;
	float3 E = normalize(ViewPosition() - wPosition);
    

    [flatten]
	if (NdotL > 0.0f)
	{
		output.Diffuse = NdotL * material.Diffuse;


        [flatten]
		if (any(material.Specular.rgb))
		{
			float3 R = normalize(reflect(-direction, normal));
			float RdotE = saturate(dot(R, E));

			float specular = pow(RdotE, material.Specular.a);
			output.Specular = specular * material.Specular * GlobalLight.Specular;
		}

	}

     [flatten]
	if (any(material.Emissive.rgb))
	{
		float NdotE = dot(E, normal);
        
		float emissive = smoothstep(1.0f - material.Emissive.a, 1.0f, 1.0f - saturate(NdotE));
            
		output.Emissive = material.Emissive * emissive;
	}

}

float4 PS_Directional(VertexOutput_Directional input) : SV_Target
{
	float4 position = input.Position;
	float3 normal = 0, tangent = 0;
	MaterialDesc material = MakeMaterial();
   
	UnpackGBuffer(position, input.Screen, material, normal, tangent);
	//material.Diffuse = float4(1, 1, 1, 1);
	
	MaterialDesc result = MakeMaterial();
    
	ComputeLight_Deffered(result, material, normal, position.xyz);
    
	return float4(MaterialToColor(result), 1);
}
/////////////////////////////////////////////////////////
//PointLights
/////////////////////////////////////////////////////////

cbuffer CB_Deffered_PointLight
{
	float PointLight_TessFator;
	float3 CB_Deffered_PointLight_Padding;
	
	matrix PointLight_Projection[MAX_POINT_LIGHT];
	PointLightDesc PointLight_Deffered[MAX_POINT_LIGHT];
};

float4 VS_PointLights() : Position
{
	return float4(0, 0, 0, 1);
}

struct ConstantHullOutput_PointLights
{
	float Edge[4] : SV_TessFactor; //몇개로 자를 것인지
	float Inside[2] : SV_InsideTessFactor; // 무게 중심
};

ConstantHullOutput_PointLights ConstHS_PointLights()
{
	ConstantHullOutput_PointLights output;


	output.Edge[0] = output.Edge[1] = output.Edge[2] = output.Edge[3] = PointLight_TessFator;
	output.Inside[0] = output.Inside[1] = PointLight_TessFator;

	return output;
}

struct HullOutput_PointLights
{
	float4 Direction : Positoin;
};


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstHS_PointLights")]
HullOutput_PointLights HS_PointLights(uint id : SV_PrimitiveID)//2개의 정점
{
	HullOutput_PointLights output;

	float4 Direction[2] = { float4(1, 1, 1, 1), float4(-1, 1, -1, 1) };
	output.Direction = Direction[id % 2]; //direction?
	return output;
} //y가 1인 평면을 넘겨준다.

struct DomainOutput_PointLights
{
	float4 Position : SV_Position;
	float2 Screen : Uv;
	uint PrimitiveID : Id;
};

[domain("quad")]
DomainOutput_PointLights DS_PointLights
(
    ConstantHullOutput_PointLights input,
    float2 uv : SV_DomainLocation,
    const OutputPatch<HullOutput_PointLights, 4> quad,
	uint id : SV_PrimitiveID
)
{
	float2 clipSpace = uv.xy * 2.0f - 1.0f;
	
	float2 clipSpaceAbs = abs(clipSpace.xy);
	float maxLength = max(clipSpaceAbs.x, clipSpaceAbs.y);

	float3 Direction = normalize(float3(clipSpace.xy, (maxLength - 1.0f)) * quad[0].Direction.xyz);
	float4 position = float4(Direction, 1.0f);

	DomainOutput_PointLights output;
	output.Position = mul(position, PointLight_Projection[id/2]);
	output.Screen = output.Position.xyz / output.Position.w;
	output.PrimitiveID = id/2;
	
	return output;
}


void ComputePointLight_Deffered(inout MaterialDesc output,MaterialDesc material,uint id, float3 normal, float3 wPosition)
{
	output = MakeMaterial();
	normal = normalize(normal);

	PointLightDesc desc = PointLight_Deffered[id];
    
	float3 light = desc.Position - wPosition;
	float dist = length(light);
    
    [flatten]
	if (dist > desc.Range)
		return;
    
	light /= dist;
    
	output.Ambient = material.Ambient * desc.Ambient;
    
	float NdotL = dot(light, normal);
	float3 E = normalize(ViewPosition() - wPosition);
    
    [flatten]
	if (NdotL > 0.0f)
	{
		float3 R = normalize(reflect(-light, normal));
		float RdotE = saturate(dot(R, E));
		float specular = pow(RdotE, material.Specular.a);
    
		output.Diffuse = NdotL * material.Diffuse * desc.Diffuse;
		output.Specular = specular * material.Specular * desc.Specualr;
	}
    
	float NdotE = dot(E, normal);
	float emissive = smoothstep(1.0f - material.Emissive.a, 1.0f, 1.0f - saturate(NdotE));
	output.Emissive = emissive * material.Emissive * desc.Emissive;
    
    //-
	float temp = 1.0f / saturate(dist / desc.Range);
    
	float att = temp * temp * (1.0f / max(1 - desc.Intensity, 1e-8f));
    
	output.Ambient = output.Ambient * temp;
	output.Diffuse = output.Diffuse * att;
	output.Specular = output.Specular * att;
	output.Emissive = output.Emissive * att;
 
}

float4 PS_PointLights_Debug(DomainOutput_PointLights input) : SV_Target
{
	return float4(1, 1, 1, 1);

}

float4 PS_PointLights(DomainOutput_PointLights input) : SV_Target
{
	float4 position = input.Position;
		
	float3 normal = 0, tangent = 0;
	MaterialDesc material = MakeMaterial();
	UnpackGBuffer(position, input.Screen, material, normal, tangent);
	
	MaterialDesc result = MakeMaterial();
	ComputePointLight_Deffered(result, material,input.PrimitiveID, normal, position.xyz);
	
	return float4(MaterialToColor(result), 1.0f);
}
