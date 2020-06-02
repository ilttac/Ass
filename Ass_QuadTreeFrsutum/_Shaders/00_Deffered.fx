#include "00_Light.fx"

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

///////////////////////////////////////////////////////////////////////////////


cbuffer CB_Deffered_PointLight
{
    float PointLight_TessFator;
    float3 CB_Deffered_PointLight_Padding;

    matrix PointLight_Projection;
    PointLightDesc PointLight_Deffered;
};

void ComputePointLight_Deffered(inout MaterialDesc output, MaterialDesc material, float3 normal, float3 wPosition)
{
    output = MakeMaterial();    
    normal = normalize(normal);

    PointLightDesc desc = PointLight_Deffered;

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