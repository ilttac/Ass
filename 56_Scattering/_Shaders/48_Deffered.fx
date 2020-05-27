#include "00_Global.fx"
//#include "00_Light.fx"
#include "00_Deffered.fx"
#include "00_Model.fx"

Texture2D DefferedMaps[6];
DepthStencilState Deffered_DepthStencil_State;

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
    material.Diffuse = float4(1, 1, 1, 1);
   
    MaterialDesc result = MakeMaterial();
    MaterialDesc output = MakeMaterial();
    
    ComputeLight_Deffered(output, material, normal, position.xyz);
    AddMaterial(result, output);
    
    return float4(MaterialToColor(result), 1);
}

///////////////////////////////////////////////////////////////////////////////

//float4 VS_PointLights() : Position
//{
//    return float4(0, 0, 0, 1);
//}

//struct ConstantHullOutput_PointLights
//{
//    float Edge[4] : SV_TessFactor;
//    float Inside[2] : SV_InsideTessFactor;
//};

//ConstantHullOutput_PointLights ConstHS_PointLights()
//{
//    ConstantHullOutput_PointLights output;

//    float factor = 18.0f;

//    output.Edge[0] = output.Edge[1] = output.Edge[2] = output.Edge[3] = factor;
//    output.Inside[0] = output.Inside[1] = factor;

//    return output;
//}

//struct HullOutput_PointLights
//{
//    float4 HemiDirection : Positoin;
//};

//static const float4 HemiDirection[2] =
//{
//    float4(1, 1, 1, 1), float4(-1, 1, -1, 1)
//};

//[domain("quad")]
//[partitioning("integer")]
//[outputtopology("triangle_ccw")]
//[outputcontrolpoints(4)]
//[patchconstantfunc("ConstHS_PointLights")]
//HullOutput_PointLights HS_PointLights(uint id : SV_PrimitiveID)
//{
//    HullOutput_PointLights output;

//    output.HemiDirection = HemiDirection[id];
//    return output;
//}



//struct DomainOutput_PointLights
//{
//    float4 Position : SV_Position;
//    float2 Screen : Position1;
//};

//[domain("quad")]
//DomainOutput_PointLights DS_PointLights
//(
//    ConstantHullOutput_PointLights input,
//    float2 uv : SV_DomainLocation,
//    const OutputPatch<HullOutput_PointLights, 4> quad,
//    uint id : SV_PrimitiveID
//)
//{
//    float2 clipSpace = uv.xy * 2.0f - 1.0f;
//    float2 clipSpaceAbs = abs(clipSpace.xy);
//    float maxLength = max(clipSpaceAbs.x, clipSpaceAbs.y);

//    float3 normalDirection = normalize(float3(clipSpace.xy, (maxLength - 1.0f)) * quad[0].HemiDirection.xyz);
//    float4 position = float4(normalDirection, 1.0f);

//    DomainOutput_PointLights output;
//    output.Position = mul(position, PointLights_WVP);
//    output.Screen = output.Position.xyz / output.Position.w;

//    return output;
//}

float4 PS_PointLights(VertexOutput_Deffered input) : SV_target
{
    float depth = DefferedMaps[0].Load(int3(input.Position.xy, 0)).r;
    float linearDepth = Deffered.Perspective.z / (depth + Deffered.Perspective.w);

    MaterialDesc material;
    material.Ambient = float4(0, 0, 0, 1);
    material.Diffuse = DefferedMaps[1].Load(int3(input.Position.xy, 0));
    material.Specular = DefferedMaps[2].Load(int3(input.Position.xy, 0));
    material.Specular.a = Deffered.SPecularPowerRange.x + Deffered.SPecularPowerRange.y * material.Specular.a;

    material.Emissive = DefferedMaps[3].Load(int3(input.Position.xy, 0));
    float3 normal = DefferedMaps[4].Load(int3(input.Position.xy, 0)).rgb;
    float3 tangent = DefferedMaps[5].Load(int3(input.Position.xy, 0)).rgb * 2.0f - 1.0f;

    float4 position;
    position.xy = input.Screen * Deffered.Perspective.xy * linearDepth;
    position.z = linearDepth;
    position.w = 1.0f;
    position = mul(position, ViewInverse);

    MaterialDesc result = MakeMaterial();
    ComputePointLight(result, material, normal, position.xyz);

    return float4(MaterialToColor(result), 1);

}

technique11 T0
{
    //Deffered-PreRender
    P_DSS_VP(P0, Deffered_DepthStencil_State, VS_Mesh, PS_PackGBuffer)
    P_DSS_VP(P1, Deffered_DepthStencil_State, VS_Model, PS_PackGBuffer)
    P_DSS_VP(P2, Deffered_DepthStencil_State, VS_Animation, PS_PackGBuffer)

    //Deffered-Directional
    P_DSS_VP(P3, Deffered_DepthStencil_State, VS_Deffered, PS_Directional)  
}

