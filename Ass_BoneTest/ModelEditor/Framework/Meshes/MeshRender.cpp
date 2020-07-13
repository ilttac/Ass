#include "Framework.h"
#include "MeshRender.h"

MeshRender::MeshRender(Shader * shader, Mesh * mesh)
	: mesh(mesh)
{
	Pass(0);

	mesh->SetShader(shader);

	for (UINT i = 0; i < MAX_MESH_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);

	instanceBuffer = new VertexBuffer(worlds, MAX_MESH_INSTANCE, sizeof(Matrix), 1, true);
}

MeshRender::~MeshRender()
{
	for (Transform* transform : transforms)
		SafeDelete(transform);

	SafeDelete(instanceBuffer);
	SafeDelete(mesh);
}

void MeshRender::Update()
{
	mesh->Update();
}

void MeshRender::Render()
{
	instanceBuffer->Render();

	mesh->Render(transforms.size());
}

Transform * MeshRender::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform);

	return transform;
}


UINT MeshRender::GetPickedPosition()
{
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();
	Viewport* vp = Context::Get()->GetViewport();

	Vector3 mouse = Mouse::Get()->GetPosition();

	Matrix M;
	for (UINT k = 0; k < transforms.size(); k++)
	{
		Vector3 n;
		Vector3 f;
		mouse.z = 0.0f;
		vp->UnProject(&n, mouse, transforms[k]->World(), V, P);

		mouse.z = 1.0f;
		vp->UnProject(&f, mouse, transforms[k]->World(), V, P);

		Vector3 start = n;
		Vector3 direction = f - n;
		
		for (int i = 0; i < mesh->GetVertexCount();)
		{
			Vector3 p[4];
			for (int j = 0; j < 4; j++)
			{
				p[j] = mesh->GetVertices()[i].Position;
				i++;
			}
			float u, v, distance;
			if (D3DXIntersectTri(&p[0], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
				return k;

			if (D3DXIntersectTri(&p[3], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
				return k;

		}
	}
	return UINT32_MAX;
}

void MeshRender::UpdateTransforms()
{
	for (UINT i = 0; i < transforms.size(); i++)
		memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix) * MAX_MESH_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
}
