#include "Framework.h"
#include "Sky.h"
#include "Scattering.h"

Sky::Sky(Shader * shader)
	: Renderer(shader)
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
}

Sky::~Sky()
{
	SafeDelete(scattering);
	SafeDelete(scatterBuffer);
}

void Sky::Pass(UINT scatteringPass)
{
	scattering->Pass(scatteringPass);
}

void Sky::Update()
{
	Super::Update();
	
	//Auto
	{

	}
	//Manual
	{
		ImGui::SliderFloat("Theta", &theta, -Math::PI,Math::PI);//0 양쪽끝 // 맨위 3.14

		float x = sinf(theta);
		float y = cosf(theta);

		Context::Get()->Direction() = Vector3(x, y, 0.0f);
	}

	scattering->Update();
}

void Sky::PreRender()
{
	if (theta == prevTheta)
		return;

	prevTheta = theta;

	//쉐이더로 밀어주는 부분 
	scatterBuffer->Apply();
	sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());

	Super::Render();//쉐이더로 밀어준다.
	scattering->PreRender();
}

void Sky::Render()
{
	Vector3 position;
	Context::Get()->GetCamera()->Position(&position);

	//Scattering
	{
		position.y -= 0.2f;
		
		GetTransform()->Position(position);
		GetTransform()->Scale(1, 1, 1);
		GetTransform()->Rotation(0, 0, 1);

		scatterDesc.StarIntensity = Context::Get()->Direction().y;
		scatterBuffer->Apply();
		sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());

		Super::Render();
		scattering->Render();
	}
}

void Sky::PostRender()
{
	scattering->PostRender(); // rtv 띄우는 부분.
}
