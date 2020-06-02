#include "Framework.h"
#include "Sky.h"
#include "Scattering.h"
#include "Dome.h"

Sky::Sky(Shader * shader)
	: shader(shader)
{
	scatterDesc.InvWaveLength.x = 1.0f / powf(scatterDesc.WaveLength.x, 4.0f);
	scatterDesc.InvWaveLength.y = 1.0f / powf(scatterDesc.WaveLength.y, 4.0f);
	scatterDesc.InvWaveLength.z = 1.0f / powf(scatterDesc.WaveLength.z, 4.0f);

	scatterDesc.WaveLengthMie.x = powf(scatterDesc.WaveLength.x, -0.84f);
	scatterDesc.WaveLengthMie.y = powf(scatterDesc.WaveLength.y, -0.84f);
	scatterDesc.WaveLengthMie.z = powf(scatterDesc.WaveLength.z, -0.84f);

	scattering = new Scattering(shader);
	scatterBuffer = new ConstantBuffer(&scatterDesc, sizeof(ScatterDesc));
	sScatterBuffer = shader->AsConstantBuffer("CB_Scattering");

	dome = new Dome(shader, Vector3(0, 16, 0), Vector3(80, 80, 80));

	sRayleighMap = shader->AsSRV("RayleighMap");
	sMieMap = shader->AsSRV("MieMap");
}

Sky::~Sky()
{
	SafeDelete(scattering);
	SafeDelete(scatterBuffer);
	SafeDelete(dome);
}

void Sky::Pass(UINT scatteringPass, UINT domePass)
{
	scattering->Pass(scatteringPass);
	dome->Pass(domePass);
}

void Sky::Update()
{

	//Auto
	if (bRealTime == true)
	{
		theta += Time::Delta() * timeFactor;

		if (theta > Math::PI)
		{
			theta -= Math::PI * 2.0f;
		}
		float x = sinf(theta);
		float y = cosf(theta);

		Context::Get()->Direction() = Vector3(x, y, 0.0f);
	}
	//Manual
	else
	{
		ImGui::SliderFloat("Theta", &theta, -Math::PI, Math::PI);

		float x = sinf(theta);
		float y = cosf(theta);

		Context::Get()->Direction() = Vector3(x, y, 0.0f);
	}


	scattering->Update();
	dome->Update();
}

void Sky::PreRender()
{
	if (theta == prevTheta)
		return;

	prevTheta = theta;

	scatterBuffer->Apply();
	sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());

	scattering->PreRender();
}

void Sky::Render()
{
	Vector3 position;
	Context::Get()->GetCamera()->Position(&position);

	//Dome
	{
		sRayleighMap->SetResource(scattering->RayleighRTV()->SRV());
		sMieMap->SetResource(scattering->MieRTV()->SRV());
		
		dome->Render();
	}
}

void Sky::PostRender()
{
	scattering->PostRender();
}

void Sky::RealTime(bool val, float theta, float timeFactor)
{
	bRealTime = val;

	this->theta = theta;
	this->timeFactor = timeFactor;
}
