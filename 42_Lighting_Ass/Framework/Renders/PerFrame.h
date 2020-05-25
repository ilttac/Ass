#pragma once

class PerFrame
{
public:
	PerFrame(Shader* shader);
	~PerFrame();

public:
	void Update();
	void Render();

private:
	struct BufferDesc
	{
		Matrix View; //16Byte * 4EA
		Matrix ViewInverse;
		Matrix Projection;
		Matrix VP;

		float Time;
		float Padding[3];
	}bufferDesc;

	struct LightDesc
	{
		Color Ambient;
		Color Specular;
		Vector3 Direction;
		float Padding;

		Vector3 Position;
		float Padding2;
	}lightDesc;

	struct PointDesc
	{
		UINT count = 0;
		float Padding[3];

		PointLight Lights[MAX_POINT_LIGHT];
	}pointLightDesc;

private:
	Shader * shader;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	ConstantBuffer* lightBuffer;
	ID3DX11EffectConstantBuffer* sLightBuffer;

	ConstantBuffer* pointLightBuffer;
	ID3DX11EffectConstantBuffer* sPointLightBuffer;
};