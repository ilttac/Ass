#include "Framework.h"
#include "SkyCube.h"

SkyCube::SkyCube(wstring file)
{
	shader = new Shader(L"14_SkyCube.fx");

	sphere = new MeshRender(shader, new MeshSphere(0.5f));
	sphere->AddTransform();

	wstring temp = L"../../_Textures/" + file;
	Check(D3DX11CreateShaderResourceViewFromFile
	(	
		D3D::GetDevice(), temp.c_str(), NULL, NULL, &srv, NULL
	));

	sSrv = shader->AsSRV("SkyCubeMap");
}

SkyCube::~SkyCube()
{
	SafeDelete(shader);
	SafeDelete(sphere);
	SafeRelease(srv);
}

void SkyCube::Update()
{
	Vector3 position;
	Context::Get()->GetCamera()->Position(&position);

	sphere->GetTransform(0)->Position(position);
	sphere->UpdateTransforms();
}

void SkyCube::Render()
{	
	sSrv->SetResource(srv);
	sphere->Render();
}
