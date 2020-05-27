#pragma once
class Sky : public Renderer
{
public:
	Sky(Shader* shader);
	~Sky();

	void Pass(UINT scatteringPass);

	void Update();
	void PreRender();
	void Render();
	void PostRender();

private:
	struct ScatterDesc
	{
		Vector3 WaveLength = Vector3(0.65f,0.57f,0.475f);
		int SampleCount = 8;

		Vector3 InvWaveLength;
		float StarIntensity;

		Vector3 WaveLengthMie;
		float MoonAlpha;
	}scatterDesc;
private:
	Shader* shader;

	bool bRealTime = false;

	float timeFactor = 1.0f;
	float theta = 0.0f, prevTheta = 1.0f;

	class Scattering* scattering;
	ConstantBuffer* scatteringBuffer;
	ID3DX11EffectConstantBuffer* sScatteringBuffer;
};