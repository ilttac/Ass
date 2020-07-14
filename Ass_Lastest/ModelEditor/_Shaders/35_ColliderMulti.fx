struct ClipFrameBoneMatrix
{
    matrix Bone;
};

struct ResultMatrix
{
    matrix Result;
};

StructuredBuffer<ClipFrameBoneMatrix> Input;
RWStructuredBuffer<ResultMatrix> Output;

#define MAX_MODEL_TRANSFORMS 250
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

cbuffer CB_Attach
{
    uint AttachBoneIndex;
};

cbuffer CB_AttachAllBones
{
	uint BoneIndex[MAX_MODEL_TRANSFORMS];
	float Padding[2];
};

[numthreads(MAX_MODEL_TRANSFORMS, 1, 1)]
void CS(uint GroupIndex : SV_GroupIndex)
{
    uint index = GroupIndex;

    uint boneIndex[2]; 
    matrix result = 0;
	
	boneIndex[0] = Tweenframes[0].Curr.Clip * MAX_MODEL_KEYFRAMES * MAX_MODEL_TRANSFORMS;
    boneIndex[0] += Tweenframes[0].Curr.CurrFrame * MAX_MODEL_TRANSFORMS;
	boneIndex[0] += index;

    boneIndex[1] = Tweenframes[0].Curr.Clip * MAX_MODEL_KEYFRAMES * MAX_MODEL_TRANSFORMS;
    boneIndex[1] += Tweenframes[0].Curr.NextFrame * MAX_MODEL_TRANSFORMS;
	boneIndex[1] += index;

    matrix currFrame = Input[boneIndex[0]].Bone;
    matrix nextFrame = Input[boneIndex[1]].Bone;
    result = lerp(currFrame, nextFrame, Tweenframes[0].Curr.Time);

    [flatten]
    if (Tweenframes[0].Next.Clip > -1)
    {
        boneIndex[0] = Tweenframes[0].Next.Clip * MAX_MODEL_KEYFRAMES * MAX_MODEL_TRANSFORMS;
        boneIndex[0] += Tweenframes[0].Next.CurrFrame * MAX_MODEL_TRANSFORMS;
		boneIndex[0] += index;

        boneIndex[1] = Tweenframes[0].Next.Clip * MAX_MODEL_KEYFRAMES * MAX_MODEL_TRANSFORMS;
        boneIndex[1] += Tweenframes[0].Next.NextFrame * MAX_MODEL_TRANSFORMS;
		boneIndex[1] += index;

        currFrame = Input[boneIndex[0]].Bone;
        nextFrame = Input[boneIndex[1]].Bone;

        matrix nextAnim = lerp(currFrame, nextFrame, Tweenframes[0].Next.Time);

        result = lerp(result, nextAnim, Tweenframes[0].TweenTime);
    }

	Output[index].Result = result;
}

technique11 T0
{
    pass P0
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);

        SetComputeShader(CompileShader(cs_5_0, CS()));
    }
}