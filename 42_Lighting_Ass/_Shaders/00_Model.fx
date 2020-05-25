Texture2DArray TransformsMap;

struct VertexMesh
{
    float4 Position : Position;
    float2 Uv : Uv;
    float3 Normal : Normal;
    float3 Tangent : Tangent;

    matrix Transform : InstTransform;
};

void SetMeshWorld(inout matrix world, VertexMesh input)
{
    world = input.Transform;
}

MeshOutput VS_Mesh(VertexMesh input)
{
    MeshOutput output;
   
    SetMeshWorld(World, input);
    VS_GENERATE

    return output;

}

///////////////////////////////////////////////////////////////////////////////

#define MAX_MODEL_TRANSFORMS 250
cbuffer CB_Bone
{
    uint BoneIndex;
};

struct VertexModel
{
    float4 Position : Position;
    float2 Uv : Uv;
    float3 Normal : Normal;
    float3 Tangent : Tangent0;
    float4 BlendIndices : BlendIndices0;
    float4 BlendWeights : BlendWeights0;

    matrix Transform : InstTransform;
    uint InstanceID : SV_InstanceID;
};

void SetModelWorld(inout matrix world, VertexModel input)
{
    float4 m0 = TransformsMap.Load(int4(BoneIndex * 4 + 0, input.InstanceID, 0, 0));
    float4 m1 = TransformsMap.Load(int4(BoneIndex * 4 + 1, input.InstanceID, 0, 0));
    float4 m2 = TransformsMap.Load(int4(BoneIndex * 4 + 2, input.InstanceID, 0, 0));
    float4 m3 = TransformsMap.Load(int4(BoneIndex * 4 + 3, input.InstanceID, 0, 0));

    matrix transform = matrix(m0, m1, m2, m3);
    world = mul(transform, input.Transform);

}

MeshOutput VS_Model(VertexModel input)
{
    MeshOutput output;

    SetModelWorld(World, input);
    
    VS_GENERATE

    return output;

}

///////////////////////////////////////////////////////////////////////////////

#define MAX_MODEL_KEYFRAMES 500
#define MAX_MODEL_INSTANCE 500

struct AnimationFrame
{
    int Clip;

    uint CurrFrame;
    uint NextFrame;

    float Time;
    float RunningTime;

    float3 Padding;
};

struct TweenFrame
{
    float TakeTime;
    float TweenTime;
    float RunningTime;
    float Padding;
		
    AnimationFrame Curr;
    AnimationFrame Next;
};

cbuffer CB_AnimationFrame
{
    TweenFrame Tweenframes[MAX_MODEL_INSTANCE];
};

void SetAnimationWorld(inout matrix world, VertexModel input)
{   
    matrix transform = 0;
    matrix curr = 0, currAnim = 0;
    matrix next = 0, nextAnim = 0;

    float indices[4] = { input.BlendIndices.x, input.BlendIndices.y, input.BlendIndices.z, input.BlendIndices.w };
    float weights[4] = { input.BlendWeights.x, input.BlendWeights.y, input.BlendWeights.z, input.BlendWeights.w };

    if (any(input.BlendIndices) == false)
    {
        indices[0] = BoneIndex;
        weights[0] = 1.0f;
    }

    uint clip[2];
    uint currentFrame[2];
    uint nextFrame[2];
    float time[2];

    clip[0] = Tweenframes[input.InstanceID].Curr.Clip;
    currentFrame[0] = Tweenframes[input.InstanceID].Curr.CurrFrame;
    nextFrame[0] = Tweenframes[input.InstanceID].Curr.NextFrame;
    time[0] = Tweenframes[input.InstanceID].Curr.Time;

    clip[1] = Tweenframes[input.InstanceID].Next.Clip;
    currentFrame[1] = Tweenframes[input.InstanceID].Next.CurrFrame;
    nextFrame[1] = Tweenframes[input.InstanceID].Next.NextFrame;
    time[1] = Tweenframes[input.InstanceID].Next.Time;

    float4 c0, c1, c2, c3;
    float4 n0, n1, n2, n3;

    [unroll(4)]
    for (int i = 0; i < 4; i++)
    {
        c0 = TransformsMap.Load(int4(indices[i] * 4 + 0, currentFrame[0], clip[0], 0));        
        c1 = TransformsMap.Load(int4(indices[i] * 4 + 1, currentFrame[0], clip[0], 0));        
        c2 = TransformsMap.Load(int4(indices[i] * 4 + 2, currentFrame[0], clip[0], 0));        
        c3 = TransformsMap.Load(int4(indices[i] * 4 + 3, currentFrame[0], clip[0], 0));        
        curr = matrix(c0, c1, c2, c3);

        n0 = TransformsMap.Load(int4(indices[i] * 4 + 0, nextFrame[0], clip[0], 0));
        n1 = TransformsMap.Load(int4(indices[i] * 4 + 1, nextFrame[0], clip[0], 0));
        n2 = TransformsMap.Load(int4(indices[i] * 4 + 2, nextFrame[0], clip[0], 0));
        n3 = TransformsMap.Load(int4(indices[i] * 4 + 3, nextFrame[0], clip[0], 0));
        next = matrix(n0, n1, n2, n3);

        currAnim = lerp(curr, next, time[0]);

        [flatten]
        if(clip[1] >= 0)
        {
            c0 = TransformsMap.Load(int4(indices[i] * 4 + 0, currentFrame[1], clip[1], 0));
            c1 = TransformsMap.Load(int4(indices[i] * 4 + 1, currentFrame[1], clip[1], 0));
            c2 = TransformsMap.Load(int4(indices[i] * 4 + 2, currentFrame[1], clip[1], 0));
            c3 = TransformsMap.Load(int4(indices[i] * 4 + 3, currentFrame[1], clip[1], 0));
            curr = matrix(c0, c1, c2, c3);

            n0 = TransformsMap.Load(int4(indices[i] * 4 + 0, nextFrame[1], clip[1], 0));
            n1 = TransformsMap.Load(int4(indices[i] * 4 + 1, nextFrame[1], clip[1], 0));
            n2 = TransformsMap.Load(int4(indices[i] * 4 + 2, nextFrame[1], clip[1], 0));
            n3 = TransformsMap.Load(int4(indices[i] * 4 + 3, nextFrame[1], clip[1], 0));
            next = matrix(n0, n1, n2, n3);

            nextAnim = lerp(curr, next, time[1]);

            currAnim = lerp(currAnim, nextAnim, Tweenframes[input.InstanceID].TweenTime);
        }

        transform += mul(weights[i], currAnim);
    }

    world = mul(transform, input.Transform);
    
}

MeshOutput VS_Animation(VertexModel input)
{
    MeshOutput output;

    SetAnimationWorld(World, input);
    
    VS_GENERATE

    return output;

}