#include "Framework.h"
#include "Water.h"
#include "Viewer/Fixity.h"

Water::Water(Shader * shader, float radius, UINT width, UINT height)
	: Renderer(shader)
	, radius(radius), width(width), height(height)
{
	vertexCount = 4;
	indexCount = 6;

	VertexTexture vertices[4];
	vertices[0].Position = Vector3(-radius, 0.0f, -radius);
	vertices[1].Position = Vector3(-radius, 0.0f, +radius);
	vertices[2].Position = Vector3(+radius, 0.0f, -radius);
	vertices[3].Position = Vector3(+radius, 0.0f, +radius);

	vertices[0].Uv = Vector2(0, 1);
	vertices[1].Uv = Vector2(0, 0);
	vertices[2].Uv = Vector2(1, 1);
	vertices[3].Uv = Vector2(1, 0);

	UINT indices[6] = { 0, 1, 2, 2, 1, 3 };

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTexture));
	indexBuffer = new IndexBuffer(indices, indexCount);

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Water");

	normalMap = new Texture(L"Environment/Wave2.dds");
	sNormalMap = shader->AsSRV("NormalMap");

	this->width = width > 0 ? width : (UINT)D3D::Width();
	this->height = height > 0 ? height : (UINT)D3D::Height();

	camera = new Fixity();
	reflection = new RenderTarget(this->width, this->height);
	refraction = new RenderTarget(this->width, this->height);
	depthStencil = new DepthStencil(this->width, this->height);
	viewport = new Viewport((float)this->width, (float)this->height);

	sReflection = shader->AsMatrix("Reflection");

	sReflectionMap = shader->AsSRV("ReflectionMap");
	sRefractionMap = shader->AsSRV("RefractionMap");
}

Water::~Water()
{
	SafeDelete(buffer);
	SafeDelete(normalMap);

	SafeDelete(camera);
	SafeDelete(reflection);
	SafeDelete(refraction);
	SafeDelete(depthStencil);
	SafeDelete(viewport);
}

void Water::RestartClipPlane()
{
	Plane plane(0, 0, 0, 0);
	perFrame->Clipping(plane);
}

void Water::Update()
{
	Super::Update();

	desc.WaveTranslation += waveSpeed * Time::Delta();
	if (desc.WaveTranslation > 1.0f)
		desc.WaveTranslation -= 1.0f;

	ImGui::SliderFloat("Shiness", &desc.WaterShiness, 1, 100);

	Vector3 R, T;
	Context::Get()->GetCamera()->Rotation(&R);
	Context::Get()->GetCamera()->Position(&T);

	R.x *= -1.0f;
	camera->Rotation(R);
	
	Vector3 position;
	GetTransform()->Position(&position);
	T.y = (position.y * 2.0f) - T.y;
	camera->Position(T);

	Matrix reflection;
	camera->GetMatrix(&reflection);
	sReflection->SetMatrix(reflection);
}

void Water::PreRender_Reflection()
{
	Vector3 position;
	GetTransform()->Position(&position);

	Plane plane = Plane(0, 1, 0, -position.y);
	perFrame->Clipping(plane);

	reflection->Set(depthStencil);
	viewport->RSSetViewport();
}

void Water::PreRender_Refraction()
{
	Vector3 position;
	GetTransform()->Position(&position);

	Plane plane = Plane(0, -1, 0, position.y + 0.1f);
	perFrame->Clipping(plane);

	refraction->Set(depthStencil);
	viewport->RSSetViewport();
}

void Water::Render()
{
	Super::Render();

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
	sNormalMap->SetResource(normalMap->SRV());

	sReflectionMap->SetResource(reflection->SRV());
	sRefractionMap->SetResource(refraction->SRV());

	shader->DrawIndexed(0, Pass(), indexCount);
}
