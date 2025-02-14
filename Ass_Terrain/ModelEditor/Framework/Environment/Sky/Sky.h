#pragma once

class Sky
{
public:
	Sky(Shader* shader);
	~Sky();

	void ScatteringPass(UINT val);
	void Pass(UINT domePass, UINT moonPass, UINT cloudPass);

	void Update();
	void PreRender();
	void Render();
	void PostRender();

	void Theata(float val) { theta = val; }

	void RealTime(bool val, float theta, float timeFactor = 1.0f);

private:
	struct ScatterDesc
	{
		Vector3 WaveLength = Vector3(0.65f, 0.57f, 0.475f);
		float Padding;

		Vector3 InvWaveLength;
		int SamepleCount = 8;

		Vector3 WaveLengthMie;
		float Padding2;
	} scatterDesc;	

	struct CloudDesc
	{
		float Tile = 1.5f;
		float Cover = 0.005f;
		float Sharpness = 0.405f;
		float Speed = 0.05f;
	} cloudDesc;

private:
	Shader* shader;
	bool bRealTime = false;

	float timeFactor = 1.0f;
	float theta = 0.0f;

	class Scattering* scattering;
	ConstantBuffer* scatterBuffer;
	ID3DX11EffectConstantBuffer* sScatterBuffer;

	ID3DX11EffectShaderResourceVariable* sRayleighMap;
	ID3DX11EffectShaderResourceVariable* sMieMap;

	class Dome* dome;
	class Moon* moon;

	class Cloud* cloud;
	ConstantBuffer* cloudBuffer;
	ID3DX11EffectConstantBuffer* sCloudBuffer;
public:
	//
	Dome* GetDome() { return dome; }
	Moon* GetMoon() { return moon; }
	CloudDesc& GetCloud() { return cloudDesc; }
	//
};
