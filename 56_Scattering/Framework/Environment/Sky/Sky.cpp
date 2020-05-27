#include "Framework.h"
#include "Sky.h"

Sky::Sky(Shader * shader)
	: Renderer(shader)
{
	scatterDesc.InvWaveLength.x = 1.0f / powf(scatterDesc.WaveLength.x, 4.0f);
	scatterDesc.InvWaveLength.y = 1.0f / powf(scatterDesc.WaveLength.y, 4.0f);
	scatterDesc.InvWaveLength.z = 1.0f / powf(scatterDesc.WaveLength.z, 4.0f);

	scatterDesc.WaveLengthMie.x = powf(scatterDesc.WaveLength.x, -0.84f);
	scatterDesc.WaveLengthMie.y = powf(scatterDesc.WaveLength.y, -0.84f);
	scatterDesc.WaveLengthMie.z = powf(scatterDesc.WaveLength.z, -0.84f);

	
}

Sky::~Sky()
{
}

void Sky::Pass(UINT scatteringPass)
{
}

void Sky::Update()
{
}

void Sky::PreRender()
{
}

void Sky::Render()
{
}

void Sky::PostRender()
{
}
