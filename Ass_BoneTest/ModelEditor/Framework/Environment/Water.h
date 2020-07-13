#pragma once

class Water : public Renderer
{
public:
	Water(Shader* shader, float radius, UINT width = 0, UINT height = 0);
	~Water();

	void RestartClipPlane();

	void Update() override;

	void PreRender_Reflection();
	void PreRender_Refraction();
	void Render() override;

private:
	struct Desc
	{
		Color RefractionColor = Color(0.1f, 0.1f, 0.3f, 1.0f);
		
		Vector2 NormalMapTile = Vector2(0.1f, 0.2f);
		float WaveTranslation = 0.0f;
		float WaveScale = 0.05f;
		
		float WaterShiness = 30.0f;
		float WaterAlpha = 0.5f;

		float Padding[2];
	} desc;
	
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

	ID3DX11EffectMatrixVariable* sReflection;

	ID3DX11EffectShaderResourceVariable* sReflectionMap;
	ID3DX11EffectShaderResourceVariable* sRefractionMap;

};