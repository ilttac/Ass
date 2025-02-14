#include "Framework.h"
#include "Mesh.h"

Mesh::Mesh()
{
		
}

Mesh::~Mesh()
{
	SafeDelete(perFrame);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(vertexBuffer);
	SafeDelete(indexBuffer);
}

void Mesh::SetShader(Shader * shader)
{
	this->shader = shader;

	SafeDelete(perFrame);
	perFrame = new PerFrame(shader);
}

void Mesh::Update()
{
	assert(perFrame != NULL);

	perFrame->Update();

}

void Mesh::Render(UINT drawCount)
{
	if (vertexBuffer == NULL || indexBuffer == NULL)
	{
		Create();

		vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(MeshVertex));
		indexBuffer = new IndexBuffer(indices, indexCount);
	}

	perFrame->Render();
	vertexBuffer->Render();
	indexBuffer->Render();

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexedInstanced(0, pass, indexCount, drawCount);
}
