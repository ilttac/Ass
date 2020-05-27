#include "Framework.h"
#include "Scattering.h"

Scattering::Scattering(Shader * shader)
	: shader(shader), width(128), height(64)
{
	mieTarget = new RenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	rayleighTarget = new RenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	depthStencil = new DepthStencil(width, height);
	viewport = new Viewport((float)width, (float)height);

	render2D = new Render2D();
	render2D->GetTransform()->Scale(200, 200, 1);
	render2D->GetTransform()->RotationDegree(0, 0, -90);

	CreateQuad();
}

Scattering::~Scattering()
{
	SafeDelete(render2D);

	SafeDelete(mieTarget);
	SafeDelete(rayleighTarget);
	SafeDelete(depthStencil);
	SafeDelete(viewport);

	SafeDelete(vertexBuffer);
}

void Scattering::Update()
{
	if (bDebug == true)
		render2D->Update();
}

void Scattering::PreRender()
{
	RenderTarget* rtvs[2];
	rtvs[0] = rayleighTarget;
	rtvs[1] = mieTarget;
	RenderTarget::Sets(rtvs, 2, depthStencil);

	viewport->RSSetViewport();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->Draw(0, pass, 6);
}

void Scattering::Render()
{
	//Todo 05.
}

void Scattering::PostRender()
{
}

void Scattering::CreateQuad()
{
}
