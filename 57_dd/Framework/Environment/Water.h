#pragma once

class Water : public Renderer
{
public:
	Water(Shader* shader, float radius, UINT width = 0, UINT height = 0);
	~Water();

	void RestartClipPlane();

	void Update() override;

	void PreRender_Reflection();
	void Render() override;

private:
	struct Desc
	{
		Matrix ReflectionMatrix;
		Color RefractionColor = Color(0.2f,0.3f,0.1f,1.0f);

		Vector2 NormalMapTile = Vector2(0.1f,0.2f); //쪼개서 만들어서 물이 계속 반복하게 만듬.
		float WaveTranlation =0.0f;
		float WaveScale = 0.05f;

		float WaterShiness = 30.0f;
		float WaterAlpha = 0.5f;

		float Padding[2];
	}desc;

private:
	float radius;
	UINT width, height;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	float waveSpeed = 0.06f;
	Texture* normalMap;
	ID3DX11EffectShaderResourceVariable* sNormalMap;

	class Fixity* camera;
	RenderTarget* reflection;
	RenderTarget* refraction;
	DepthStencil* depthStencil;
	Viewport* viewport;
};