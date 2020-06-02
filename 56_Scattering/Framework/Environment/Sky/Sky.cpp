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

	dome = new Dome(shader, Vector3(0, 0, 0), Vector3(100, 100, 100));
	domeBuffer = new ConstantBuffer(&domeDesc, sizeof(DomeDesc));
	sDomeBuffer = shader->AsConstantBuffer("CB_Dome");

	sRayleighMap = shader->AsSRV("RayleighMap");
	sMieMap = shader->AsSRV("MieMap");
}

Sky::~Sky()
{
	SafeDelete(scattering);
	SafeDelete(scatterBuffer);
	SafeDelete(dome);
	SafeDelete(domeBuffer);
}

void Sky::Pass(UINT scatteringPass, UINT domePass)
{
	scattering->Pass(scatteringPass);
	dome->Pass(domePass);
}

void Sky::Update()
{

	//Auto
	{

	}

	//Manual
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

	//Scattering
	{

		scatterBuffer->Apply();
		sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());

		sRayleighMap->SetResource(scattering->RayleighRTV()->SRV());
		sMieMap->SetResource(scattering->MieRTV()->SRV());
		
		dome->Render();
	}
}

void Sky::PostRender()
{
	scattering->PostRender();
}
