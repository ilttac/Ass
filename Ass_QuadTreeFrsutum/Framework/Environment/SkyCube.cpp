#include "Framework.h"
#include "SkyCube.h"

SkyCube::SkyCube(wstring file, Shader* shader)
	: shader(shader)
{
	if (shader == NULL)
	{
		shader = new Shader(L"14_SkyCube.fx");
		bCreateShader = true;
	}

	sphereRender = new MeshRender(this->shader, new MeshSphere(0.5f));
	sphereRender->AddTransform()->Scale(500, 500, 500);

	wstring temp = L"../../_Textures/" + file;
	Check(D3DX11CreateShaderResourceViewFromFile
	(	
		D3D::GetDevice(), temp.c_str(), NULL, NULL, &srv, NULL
	));

	sSrv = this->shader->AsSRV("SkyCubeMap");
}

SkyCube::~SkyCube()
{
	if(bCreateShader == true)
		SafeDelete(shader);

	SafeDelete(sphereRender);
	SafeRelease(srv);
}

void SkyCube::Update()
{
	Vector3 position;
	Context::Get()->GetCamera()->Position(&position);

	sphereRender->GetTransform(0)->Position(position);
	sphereRender->UpdateTransforms();
}

void SkyCube::Render()
{	
	sSrv->SetResource(srv);

	sphereRender->Pass(pass);
	sphereRender->Render();
}
