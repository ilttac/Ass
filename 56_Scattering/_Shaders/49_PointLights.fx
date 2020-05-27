#include "00_Global.fx"
#include "00_Deffered.fx"
#include "00_Model.fx"

Texture2D DefferedMaps[6];


struct DefferedDesc
{
    //Specular
    float4 Perspective;
    float2 SPecularPowerRange;
};

cbuffer CB_Deffered
{
    DefferedDesc Deffered;
};

///////////////////////////////////////////////////////////////////////////////

static const float2 ScreenNDC[4] = { float2(-1, +1), float2(+1, +1), float2(-1, -1), float2(+1, -1) };

struct VertexOutput_Deffered
{
    float4 Position : SV_Position;
    float2 Screen : Position1;
};


VertexOutput_Deffered VS_Deffered(uint VertexID : SV_VertexID)
{
    VertexOutput_Deffered output;

    output.Position = float4(ScreenNDC[VertexID], 0, 1);
    output.Screen = output.Position.xy;

    return output;
}

///////////////////////////////////////////////////////////////////////////////

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
    output.Specular.a = max(1e-6, (Material.Specular.a - Deffered.SPecularPowerRange.x) / Deffered.SPecularPowerRange.y);
    output.Emissive = Material.Emissive;
    output.Normal = float4(input.Normal, 1);
    output.Tangent = float4(input.Tangent, 1);

    return output;
}

///////////////////////////////////////////////////////////////////////////////

void UnpackGBuffer(inout float4 position, in float2 screen, out MaterialDesc material, out float3 normal, out float3 tangent)
{
    material.Ambient = float4(0, 0, 0, 1);
    material.Diffuse = DefferedMaps[1].Load(int3(position.xy, 0));
    material.Specular = DefferedMaps[2].Load(int3(position.xy, 0));
    material.Specular.a = Deffered.SPecularPowerRange.x + Deffered.SPecularPowerRange.y * material.Specular.a;
    material.Emissive = DefferedMaps[3].Load(int3(position.xy, 0));

    normal = DefferedMaps[4].Load(int3(position.xy, 0)).rgb;
    tangent = DefferedMaps[5].Load(int3(position.xy, 0)).rgb;

    float depth = DefferedMaps[0].Load(int3(position.xy, 0)).r;
    float linearDepth = Deffered.Perspective.z / (depth + Deffered.Perspective.w);

    position.xy = screen * Deffered.Perspective.xy * linearDepth;
    position.z = linearDepth;
    position.w = 1.0f;
    position = mul(position, ViewInverse);

}

///////////////////////////////////////////////////////////////////////////////

float4 PS_Directional(VertexOutput_Deffered input) : SV_Target
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

///////////////////////////////////////////////////////////////////////////////

float4 VS_PointLights() : Position
{
    return float4(0, 0, 0, 1);
}

struct ConstantHullOutput_PointLights
{
    float Edge[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
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
HullOutput_PointLights HS_PointLights(uint id : SV_PrimitiveID)
{
    HullOutput_PointLights output;

    float4 Direction[2] = { float4(1, 1, 1, 1), float4(-1, 1, -1, 1) };
    output.Direction = Direction[id];
    return output;
}

struct DomainOutput_PointLights
{
    float4 Position : SV_Position;
    float2 Screen : Uv;
};

[domain("quad")]
DomainOutput_PointLights DS_PointLights
(
    ConstantHullOutput_PointLights input,
    float2 uv : SV_DomainLocation,
    const OutputPatch<HullOutput_PointLights, 4> quad
)
{
    float2 clipSpace = uv.xy * 2.0f - 1.0f;

    float2 clipSpaceAbs = abs(clipSpace.xy);
    float maxLength = max(clipSpaceAbs.x, clipSpaceAbs.y);

    float3 direction = normalize(float3(clipSpace.xy, (maxLength - 1.0f)) * quad[0].Direction.xyz);
    float4 position = float4(direction, 1.0f);

    DomainOutput_PointLights output;
    output.Position = mul(position, PointLight_Projection);
    output.Screen = output.Position.xyz / output.Position.w;

    return output;
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
    ComputePointLight_Deffered(result, material, normal, position.xyz);

    return float4(MaterialToColor(result), 1.0f);
}


DepthStencilState Deffered_DepthStencil_State;
RasterizerState Deffered_Rasterizer_State;

technique11 T0
{
    //Deffered-PreRender
    P_DSS_VP(P0, Deffered_DepthStencil_State, VS_Mesh, PS_PackGBuffer)
    P_DSS_VP(P1, Deffered_DepthStencil_State, VS_Model, PS_PackGBuffer)
    P_DSS_VP(P2, Deffered_DepthStencil_State, VS_Animation, PS_PackGBuffer)

    //Deffered-Directional
    P_DSS_VP(P3, Deffered_DepthStencil_State, VS_Deffered, PS_Directional)

    //Deffered-PointLights
    P_RS_VTP(P4, Deffered_Rasterizer_State, VS_PointLights, HS_PointLights, DS_PointLights, PS_PointLights_Debug)
    P_RS_DSS_BS_VTP(P5, Deffered_Rasterizer_State, Deffered_DepthStencil_State, Blend_Addtive, VS_PointLights, HS_PointLights, DS_PointLights, PS_PointLights)
}

