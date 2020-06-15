#include "Framework.h"
#include "PostEffect.h"

PostEffect::PostEffect(wstring shaderFile)
	:Renderer(shaderFile)
{
	VertexTexture vertices[6];
	vertices[0].Position = Vector3(-1.0f, -1.0f, 0.0f);
	vertices[1].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[2].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[3].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[4].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[5].Position = Vector3(+1.0f, +1.0f, 0.0f);

	vertexBuffer = new VertexBuffer(vertices, 6, sizeof(VertexTexture));
	sDiffuseMap = shader->AsSRV("DiffuseMap");
}

PostEffect::~PostEffect()
{
}

void PostEffect::Update()
{
	Super::Update();
}

void PostEffect::Render()
{
	Super::Render();

	shader->Draw(0, Pass(), 6);
}

void PostEffect::SRV(ID3D11ShaderResourceView* srv)
{
	sDiffuseMap->SetResource(srv);
}
