#include "Framework.h"
#include "Terrain.h"
#include "../SceneEditor/SceneEditor.h"

Terrain::Terrain(Shader* shader, wstring heightFile)
	: Renderer(shader)
{
	sBaseMap = shader->AsSRV("BaseMap");
	sAlphaMap = shader->AsSRV("AlphaMap");
	sLayerMap = shader->AsSRV("LayerMap");

	//원래 height 맵에 r 에 basemap , g : layer1, b: layer2 ,a:alpha
	heightMap = new Texture(heightFile);
	heightMapFileName = String::ToString(heightFile);
	brushBuffer = new ConstantBuffer(&brushDesc, sizeof(BrushDesc));
	sBrushBuffer = shader->AsConstantBuffer("CB_Brush");

	lineBuffer = new ConstantBuffer(&lineDesc, sizeof(LineDesc));
	sLineBuffer = shader->AsConstantBuffer("CB_TerrainLine");

	CreateVertexData();
	CreateIndexData();
	CreateNormalData();
	vertexBuffer = new VertexBuffer
	(vertices, vertexCount, sizeof(TerrainVertex), 0, true);
	indexBuffer = new IndexBuffer(indices, indexCount);
}

Terrain::~Terrain()
{
	SafeDelete(heightMap);

	SafeDeleteArray(vertices);
	SafeDelete(vertexBuffer);

	SafeDeleteArray(indices);
	SafeDelete(indexBuffer);

	SafeDelete(brushBuffer);
	SafeDelete(lineBuffer);

	SafeDelete(baseMap);
	SafeDelete(layerMap);
	SafeDelete(alphaMap);
}

void Terrain::Update()
{
	Super::Update();

	if (brushDesc.Type > 0)
	{
		brushDesc.Location = GetPickedPosition();
		brushDesc.MousePos = Mouse::Get()->GetPosition();
		if (Mouse::Get()->Press(0) && Keyboard::Get()->Press(SHIFT_PRESSED))
		{
			descHeight(brushDesc.Location, brushDesc.Type, brushDesc.Range);
		}
		else if (Mouse::Get()->Press(0) && brushDesc.Noise == true)
		{
			MakeNoise(brushDesc.Location, brushDesc.Type, brushDesc.Range);
		}
		else if (Mouse::Get()->Press(0) && brushDesc.Smooth == true)
		{
			MakeSmooth(brushDesc.Location, brushDesc.Type, brushDesc.Range);
		}
		else if (Mouse::Get()->Press(0) && brushDesc.Flat == true)
		{
			MakeFlat(brushDesc.Location, brushDesc.Type, brushDesc.Range);
		}
		else if (Mouse::Get()->Down(0) && brushDesc.Slope == true)
		{
			if (brushDesc.SlopeSwitch == 0)
			{
				brushDesc.SlopeSwitch = 1;
				brushDesc.LateLocation = brushDesc.Location;
				brushDesc.LateMousePos = brushDesc.MousePos;
			}
			else if (brushDesc.SlopeSwitch == 1 && brushDesc.LateMousePos != brushDesc.MousePos)
			{
				brushDesc.SlopeSwitch = 0;
				MakeSlope(brushDesc.LateLocation, brushDesc.Location, brushDesc.Type, brushDesc.Range);
			}
		}
		else if (Mouse::Get()->Press(0))
		{
			if (brushDesc.MousePos == brushDesc.LateMousePos)
			{
				RaiseHeightRect(brushDesc.LateLocation, brushDesc.Type, brushDesc.Range);
			}
			else
			{
				RaiseHeightRect(brushDesc.Location, brushDesc.Type, brushDesc.Range);
				brushDesc.LateMousePos = brushDesc.MousePos;
				brushDesc.LateLocation = brushDesc.Location;
			}

		}
	}
}

void Terrain::Render()
{
	Super::Render();

	if (baseMap != NULL)
		sBaseMap->SetResource(baseMap->SRV());

	brushBuffer->Apply();
	sBrushBuffer->SetConstantBuffer(brushBuffer->Buffer());

	lineBuffer->Apply();
	sLineBuffer->SetConstantBuffer(lineBuffer->Buffer());
	if (layerMap != NULL && alphaMap != NULL)
	{
		sAlphaMap->SetResource(alphaMap->SRV());
		sLayerMap->SetResource(layerMap->SRV());
	}

	shader->DrawIndexed(0, Pass(), indexCount);
}

void Terrain::BaseMap(wstring file)
{
	SafeDelete(baseMap);
	baseMap = new Texture(file);
}

void Terrain::LayerMap(wstring file, wstring alpha)
{
	SafeDelete(alphaMap);
	SafeDelete(layerMap);//기존꺼지우거 다시깐다.

	alphaMap = new Texture(alpha);
	layerMap = new Texture(file);

}
void Terrain::LayerMap(wstring file)
{
	SafeDelete(layerMap);
	layerMap = new Texture(file);
}

void Terrain::AlphaMap(wstring file)
{
	SafeDelete(alphaMap);
	alphaMap = new Texture(file);
}
float Terrain::GetHeight(Vector3& position)
{
	UINT x = (UINT)position.x;
	UINT z = (UINT)position.z;

	UINT index[4];
	index[0] = width * z + x;
	index[1] = width * (z + 1) + x;
	index[2] = width * z + (x + 1);
	index[3] = width * (z + 1) + (x + 1);

	Vector3 v[4];
	for (int i = 0; i < 4; i++)
		v[i] = vertices[index[i]].Position;

	float ddx = (position.x - v[0].x) / 1.0f;
	float ddz = (position.z - v[0].z) / 1.0f;

	Vector3 result;

	if (ddx + ddz <= 1)
		result = v[0] + (v[2] - v[0]) * ddx + (v[1] - v[0]) * ddz;
	else
	{
		ddx = 1 - ddx;
		ddz = 1 - ddz;
		result = v[3] + (v[1] - v[3]) * ddx + (v[2] - v[3]) * ddz;
	}

	return result.y;

}

float Terrain::GetHeightPick(Vector3 & position)
{
	UINT x = (UINT)position.x;
	UINT z = (UINT)position.z;

	UINT index[4];
	index[0] = width * z + x;
	index[1] = width * (z + 1) + x;
	index[2] = width * z + (x + 1);
	index[3] = width * (z + 1) + (x + 1);

	Vector3 p[4];
	for (int i = 0; i < 4; i++)
		p[i] = vertices[index[i]].Position;

	Vector3 start(position.x, 1000, position.z);
	Vector3 direction(0, -1, 0);

	float u, v, distance;
	Vector3 result(-1, FLT_MIN, -1);


	if (D3DXIntersectTri(&p[0], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
		result = p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;

	if (D3DXIntersectTri(&p[3], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
		result = p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;

	return result.y;
}

Vector3 Terrain::GetPickedPosition()
{
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();
	Viewport* vp = Context::Get()->GetViewport();

	Vector3 mouse = Mouse::Get()->GetPosition();

	Vector3 n, f;

	mouse.z = 0.0f;
	vp->UnProject(&n, mouse, transform->World(), V, P);

	mouse.z = 1.0f;
	vp->UnProject(&f, mouse, transform->World(), V, P);

	Vector3 start = n;
	Vector3 direction = f - n;

	for (UINT z = 0; z < height - 1; z++)
	{
		for (UINT x = 0; x < width - 1; x++)
		{
			UINT index[4];
			index[0] = width * z + x;
			index[1] = width * (z + 1) + x;
			index[2] = width * z + (x + 1);
			index[3] = width * (z + 1) + (x + 1);

			Vector3 p[4];
			for (int i = 0; i < 4; i++)
				p[i] = vertices[index[i]].Position;

			float u, v, distance;
			if (D3DXIntersectTri(&p[0], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
				return p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;

			if (D3DXIntersectTri(&p[3], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
				return p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;
		}
	}

	return Vector3(-1, FLT_MIN, -1);
}

void Terrain::CreateVertexData()
{
	//heightMap의 색상 정보 받아오기
	vector<Color> heights;
	heightMap->ReadPixel(DXGI_FORMAT_R8G8B8A8_UNORM, &heights);
	width = heightMap->GetWidth();
	height = heightMap->GetHeight();
	//heightMap->
	//Create VData
	vertexCount = width * height;
	vertices = new TerrainVertex[vertexCount];

	for (UINT z = 0; z < height; z++)
	{
		for (UINT x = 0; x < width; x++)
		{
			UINT index = width * z + x;
			UINT pixel = width * (height - z - 1) + x;

			vertices[index].Position.x = (float)x;
			vertices[index].Position.y = (heights[pixel].r * 256.0f) / 10.0f;
			vertices[index].Position.z = (float)z;

			vertices[index].Uv.x = (float)x / (float)width;
			vertices[index].Uv.y = (float)(height - 1 - z) / (float)height;
		}
	}
}

void Terrain::CreateIndexData()
{
	indexCount = (width - 1) * (height - 1) * 6;
	indices = new UINT[indexCount];

	UINT index = 0;
	for (UINT y = 0; y < height - 1; y++)
	{
		for (UINT x = 0; x < width - 1; x++)
		{
			indices[index + 0] = width * y + x;
			indices[index + 1] = width * (y + 1) + x;
			indices[index + 2] = width * y + (x + 1);
			indices[index + 3] = width * y + (x + 1);
			indices[index + 4] = width * (y + 1) + x;
			indices[index + 5] = width * (y + 1) + (x + 1);

			index += 6;
		}
	}
}

void Terrain::CreateNormalData()
{
	for (UINT i = 0; i < indexCount / 3; i++)
	{
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		TerrainVertex v0 = vertices[index0];
		TerrainVertex v1 = vertices[index1];
		TerrainVertex v2 = vertices[index2];

		Vector3 d1 = v1.Position - v0.Position;
		Vector3 d2 = v2.Position - v0.Position;

		Vector3 normal;
		D3DXVec3Cross(&normal, &d1, &d2);

		vertices[index0].Normal = normal;
		vertices[index1].Normal = normal;
		vertices[index2].Normal = normal;
	}

	for (UINT i = 0; i < vertexCount; i++)
		D3DXVec3Normalize(&vertices[i].Normal, &vertices[i].Normal);
}

void Terrain::RaiseHeightRect(Vector3 & position, UINT type, UINT range)
{

	D3D11_BOX rect;
	rect.left = (LONG)position.x - range;
	rect.top = (LONG)position.z + range;
	rect.right = (LONG)position.x + range;
	rect.bottom = (LONG)position.z - range;

	if (rect.left < 0) rect.left = 0;
	if (rect.right >= width) rect.right = width;
	if (rect.bottom < 0) rect.bottom = 0;
	if (rect.top >= height) rect.top = height;

	if (type == 1)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{

				UINT index = width * (UINT)z + (UINT)x;
				if (vertices[index].Position.y >= 128.0f)
				{
					continue;
				}
				vertices[index].Position.y += 5.0f * Time::Delta();
			}
		}
	}
	else if (type == 2)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{
				if (range >= (LONG)sqrt(pow(position.x - x, 2) + pow(position.z - z, 2)))
				{
					UINT index = width * (UINT)z + (UINT)x;
					if (vertices[index].Position.y >= 128.0f)
					{
						continue;
					}
					vertices[index].Position.y += 5.0f * Time::Delta();
				}
			}
		}
	}



	CreateNormalData();

	/*D3D::GetDC()->UpdateSubresource
	(
		vertexBuffer->Buffer(), 0, NULL, vertices, sizeof(TerrainVertex) * vertexCount, 0
	);*/

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}

void Terrain::descHeight(Vector3 & position, UINT type, UINT range)
{
	D3D11_BOX rect;
	rect.left = (LONG)position.x - range;
	rect.top = (LONG)position.z + range;
	rect.right = (LONG)position.x + range;
	rect.bottom = (LONG)position.z - range;

	if (rect.left < 0) rect.left = 0;
	if (rect.right >= width) rect.right = width;
	if (rect.bottom < 0) rect.bottom = 0;
	if (rect.top >= height) rect.top = height;

	if (type == 1)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{
				UINT index = width * (UINT)z + (UINT)x;
				if (vertices[index].Position.y <= -128.0f)
				{
					continue;
				}
				vertices[index].Position.y -= (5.0f * Time::Delta());
			}
		}
	}
	else if (type == 2)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{
				if (range >= (LONG)sqrt(pow(position.x - x, 2) + pow(position.z - z, 2)))
				{
					UINT index = width * (UINT)z + (UINT)x;
					if (vertices[index].Position.y <= -128.0f)
					{
						continue;
					}
					vertices[index].Position.y -= (5.0f * Time::Delta());
				}
			}
		}
	}



	CreateNormalData();

	//D3D::GetDC()->UpdateSubresource
	//(
	//	vertexBuffer->Buffer(), 0, NULL, vertices, sizeof(TerrainVertex) * vertexCount, 0
	//);

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}
void Terrain::MakeNoise(Vector3 & position, UINT type, UINT range)
{
	D3D11_BOX rect;
	srand((UINT)time(0));


	rect.left = (LONG)position.x - range;
	rect.top = (LONG)position.z + range;
	rect.right = (LONG)position.x + range;
	rect.bottom = (LONG)position.z - range;

	if (rect.left < 0) rect.left = 0;
	if (rect.right >= width) rect.right = width;
	if (rect.bottom < 0) rect.bottom = 0;
	if (rect.top >= height) rect.top = height;

	if (type == 1)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{
				float randNum = (rand() % 10) - 5;
				UINT index = width * (UINT)z + (UINT)x;
				vertices[index].Position.y = randNum;
			}
		}
	}
	else if (type == 2)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{
				if (range >= (LONG)sqrt(pow(position.x - x, 2) + pow(position.z - z, 2)))
				{
					float randNum = (rand() % 10) - 5;
					UINT index = width * (UINT)z + (UINT)x;
					vertices[index].Position.y = randNum;
				}
			}
		}
	}

	CreateNormalData();

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}
void Terrain::MakeSmooth(Vector3 & position, UINT type, UINT range)
{
	D3D11_BOX rect;

	rect.left = (LONG)position.x - range;
	rect.top = (LONG)position.z + range;
	rect.right = (LONG)position.x + range;
	rect.bottom = (LONG)position.z - range;

	if (rect.left < 0) rect.left = 0;
	if (rect.right >= width) rect.right = width;
	if (rect.bottom < 0) rect.bottom = 0;
	if (rect.top >= height) rect.top = height;
	int max = 0;
	int min = 0;

	for (LONG z = rect.bottom; z <= rect.top; z++)
	{
		for (LONG x = rect.left; x <= rect.right; x++)
		{
			UINT index = width * (UINT)z + (UINT)x;
			if (vertices[index].Position.y > max)
			{
				max = vertices[index].Position.y;
			}
			if (vertices[index].Position.y < min)
			{
				min = vertices[index].Position.y;
			}
		}
	}
	int avg = (max + min) / 2;
	if (type == 1)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{
				UINT index = width * (UINT)z + (UINT)x;
				if (vertices[index].Position.y > avg)
				{
					vertices[index].Position.y -= 3.0f * Time::Delta();
				}
				else if (vertices[index].Position.y <= avg)
				{
					vertices[index].Position.y += 3.0f * Time::Delta();
				}
			}
		}
	}
	else if (type == 2)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{
				if (range >= (LONG)sqrt(pow(position.x - x, 2) + pow(position.z - z, 2)))
				{
					UINT index = width * (UINT)z + (UINT)x;
					if (vertices[index].Position.y > avg)
					{
						vertices[index].Position.y -= 3.0f * Time::Delta();
					}
					else if (vertices[index].Position.y <= avg)
					{
						vertices[index].Position.y += 3.0f * Time::Delta();
					}
				}
			}
		}
	}

	CreateNormalData();

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}
void Terrain::MakeFlat(Vector3 & position, UINT type, UINT range)
{
	D3D11_BOX rect;

	rect.left = (LONG)position.x - range;
	rect.top = (LONG)position.z + range;
	rect.right = (LONG)position.x + range;
	rect.bottom = (LONG)position.z - range;

	if (rect.left < 0) rect.left = 0;
	if (rect.right >= width) rect.right = width;
	if (rect.bottom < 0) rect.bottom = 0;
	if (rect.top >= height) rect.top = height;


	if (type == 1)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{
				UINT index = width * (UINT)z + (UINT)x;

				vertices[index].Position.y = 0;

			}
		}
	}
	else if (type == 2)
	{
		for (LONG z = rect.bottom; z <= rect.top; z++)
		{
			for (LONG x = rect.left; x <= rect.right; x++)
			{
				if (range >= (LONG)sqrt(pow(position.x - x, 2) + pow(position.z - z, 2)))
				{
					UINT index = width * (UINT)z + (UINT)x;

					vertices[index].Position.y = 0;

				}
			}
		}
	}

	CreateNormalData();

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}
void Terrain::MakeSlope(Vector3 & oldposition, Vector3 & newposition, UINT type, UINT range)
{
	D3D11_BOX rect;
	D3D11_BOX rect2;

	rect.left = (LONG)oldposition.x - range;
	rect.top = (LONG)oldposition.z + range;
	rect.right = (LONG)oldposition.x + range;
	rect.bottom = (LONG)oldposition.z - range;

	if (rect.left < 0) rect.left = 0;
	if (rect.right >= width) rect.right = width;
	if (rect.bottom < 0) rect.bottom = 0;
	if (rect.top >= height) rect.top = height;

	rect2.left = (LONG)newposition.x - range;
	rect2.top = (LONG)newposition.z + range;
	rect2.right = (LONG)newposition.x + range;
	rect2.bottom = (LONG)newposition.z - range;

	if (rect2.left < 0) rect2.left = 0;
	if (rect2.right >= width) rect2.right = width;
	if (rect2.bottom < 0) rect2.bottom = 0;
	if (rect2.top >= height) rect2.top = height;

	float height1;
	float perx;
	if (oldposition.y >= newposition.y)
	{
		height1 = oldposition.y;
		perx = height1 / sqrt(pow((LONG)rect2.left - (LONG)rect.left, 2) + pow((LONG)rect2.top - (LONG)rect.top, 2));
	}
	else
	{
		height1 = newposition.y;
		perx = height1 / sqrt(pow((LONG)rect2.left - (LONG)rect.left, 2) + pow((LONG)rect2.top - (LONG)rect.top, 2));;
	}

	if (oldposition.x <= newposition.x)
	{
		if (oldposition.z <= newposition.z)
		{
			if (type > 0)
			{
				if (oldposition.y >= newposition.y)
				{
					float maxheight = oldposition.y;
					while (true)
					{
						for (LONG z = rect.bottom; z <= rect.top; z++)
						{
							for (LONG x = rect.left; x <= rect.right; x++)
							{
								UINT index = width * (UINT)z + (UINT)x;
								if (maxheight < 0)
								{
									vertices[index].Position.y = 0;
									continue;
								}
								vertices[index].Position.y = maxheight;
							}
						}
						maxheight -= perx;
						if (rect.top < rect2.top)
						{
							rect.top++;
							rect.bottom++;
						}
						if (rect.left < rect2.left)
						{
							rect.left++;
							rect.right++;
						}
						if (rect.top >= rect2.top && rect.left >= rect2.left)
						{
							break;
						}
					}
				}
				else
				{
					float maxheight = oldposition.y;
					while (rect.left <= rect2.left)
					{

						for (LONG z = rect.bottom; z <= rect.top; z++)
						{
							for (LONG x = rect.left; x <= rect.right; x++)
							{
								UINT index = width * (UINT)z + (UINT)x;
								if (maxheight > newposition.y)
								{
									vertices[index].Position.y = newposition.y;
									continue;
								}
								vertices[index].Position.y = maxheight;
							}
						}
						maxheight += perx;
						if (rect.top < rect2.top)
						{
							rect.top++;
							rect.bottom++;
						}
						if (rect.left < rect2.left)
						{
							rect.left++;
							rect.right++;
						}
						if (rect.top >= rect2.top && rect.left >= rect2.left)
						{
							break;
						}
					}
				}
			}
		}
		else
		{
			if (type > 0)
			{
				if (oldposition.y >= newposition.y)
				{
					float maxheight = oldposition.y;
					while (rect.left <= rect2.left)
					{
						for (LONG z = rect.bottom; z <= rect.top; z++)
						{
							for (LONG x = rect.left; x <= rect.right; x++)
							{
								//z 음수값으로 찍힐때 ---

								UINT index = width * (UINT)z + (UINT)x;
								if (maxheight < 0)
								{
									vertices[index].Position.y = 0;
									continue;
								}
								vertices[index].Position.y = maxheight;
							}
						}
						maxheight -= perx;
						if (rect.top > rect2.top)
						{
							rect.top--;
							rect.bottom--;
						}
						if (rect.left < rect2.left)
						{
							rect.left++;
							rect.right++;
						}
						if (rect.top <= rect2.top && rect.left >= rect2.left)
						{
							break;
						}
					}
				}
				else
				{
					float maxheight = oldposition.y;
					while (rect.left <= rect2.left)
					{

						for (LONG z = rect.bottom; z <= rect.top; z++)
						{
							for (LONG x = rect.left; x <= rect.right; x++)
							{
								UINT index = width * (UINT)z + (UINT)x;
								if (maxheight > newposition.y)
								{
									vertices[index].Position.y = newposition.y;
									continue;
								}
								vertices[index].Position.y = maxheight;
							}
						}
						maxheight += perx;
						if (rect.top > rect2.top)
						{
							rect.top--;
							rect.bottom--;
						}
						if (rect.left < rect2.left)
						{
							rect.left++;
							rect.right++;
						}
						if (rect.top <= rect2.top && rect.left >= rect2.left)
						{
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		if (oldposition.z <= newposition.z)
		{
			if (type > 0)
			{
				if (oldposition.y >= newposition.y)
				{
					float maxheight = newposition.y;
					while (rect2.left <= rect.left)
					{
						for (LONG z = rect2.bottom; z <= rect2.top; z++)
						{
							for (LONG x = rect2.left; x <= rect2.right; x++)
							{
								UINT index = width * (UINT)z + (UINT)x;
								if (maxheight >= oldposition.y)
								{
									vertices[index].Position.y = oldposition.y;
									continue;
								}
								vertices[index].Position.y = maxheight;
							}
						}
						maxheight += perx;
						if (rect2.top > rect.top)
						{
							rect2.top--;
							rect2.bottom--;
						}
						if (rect2.left < rect.left)
						{
							rect2.left++;
							rect2.right++;
						}
						if (rect2.top <= rect.top && rect2.left >= rect.left)
						{
							break;
						}
					}
				}
				else
				{
					float maxheight = newposition.y;
					while (rect2.left <= rect.left)
					{
						for (LONG z = rect2.bottom; z <= rect2.top; z++)
						{
							for (LONG x = rect2.left; x <= rect2.right; x++)
							{
								UINT index = width * (UINT)z + (UINT)x;
								if (maxheight <= 0)
								{
									vertices[index].Position.y = 0;
									continue;
								}
								vertices[index].Position.y = maxheight;
							}
						}
						maxheight -= perx;
						if (rect2.top > rect.top)
						{
							rect2.top--;
							rect2.bottom--;
						}
						if (rect2.left < rect.left)
						{
							rect2.left++;
							rect2.right++;
						}
						if (rect2.top <= rect.top && rect2.left >= rect.left)
						{
							break;
						}
					}
				}
			}
		}
		else
		{
			if (type > 0)
			{
				if (oldposition.y >= newposition.y)
				{
					float maxheight = newposition.y;
					while (rect2.left <= rect.left)
					{
						for (LONG z = rect2.bottom; z <= rect2.top; z++)
						{
							for (LONG x = rect2.left; x <= rect2.right; x++)
							{
								UINT index = width * (UINT)z + (UINT)x;
								if (maxheight >= oldposition.y)
								{
									vertices[index].Position.y = oldposition.y;
									continue;
								}
								vertices[index].Position.y = maxheight;
							}
						}
						maxheight += perx;
						if (rect2.top < rect.top)
						{
							rect2.top++;
							rect2.bottom++;
						}
						if (rect2.left < rect.left)
						{
							rect2.left++;
							rect2.right++;
						}
						if (rect2.top >= rect.top && rect2.left >= rect.left)
						{
							break;
						}
					}
				}
				else
				{
					float maxheight = newposition.y;
					while (rect2.left <= rect.left)
					{

						for (LONG z = rect2.bottom; z <= rect2.top; z++)
						{
							for (LONG x = rect2.left; x <= rect2.right; x++)
							{
								UINT index = width * (UINT)z + (UINT)x;

								if (maxheight <= 0)
								{
									vertices[index].Position.y = 0;
									continue;
								}
								vertices[index].Position.y = maxheight;
							}
						}
						maxheight -= perx;
						if (rect2.top < rect.top)
						{
							rect2.top++;
							rect2.bottom++;
						}
						if (rect2.left < rect.left)
						{
							rect2.left++;
							rect2.right++;
						}
						if (rect2.top >= rect.top && rect2.left >= rect.left)
						{
							break;
						}
					}
				}
			}
		}

	}

	CreateNormalData();

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}

void Terrain::SaveTerrain(wstring file)
{
	Path::CreateFolders(Path::GetDirectoryName(file));
	BinaryWriter* w = new BinaryWriter();
	w->Open(file);
	w->String(heightMapFileName);

	wstring s = Path::GetLastDirectoryName(baseMap->GetFile()) + L"/";
	wstring s2 = Path::GetFileName(baseMap->GetFile());
	w->String(String::ToString(s + s2)); //baseMap

	s = Path::GetLastDirectoryName(layerMap->GetFile()) + L"/";
	s2 = Path::GetFileName(layerMap->GetFile());
	w->String(String::ToString(s + s2)); //layerMap

	s = Path::GetLastDirectoryName(alphaMap->GetFile()) + L"/";
	s2 = Path::GetFileName(alphaMap->GetFile());
	w->String(String::ToString(s + s2)); //alphaMap

	w->UInt(vertexCount);
	for (UINT i = 0; i < vertexCount; i++)
	{
		w->Float(vertices[i].Position.y);
	}
	w->Close();
	SafeDelete(w);
}

void Terrain::ChangeVertex(vector<float> ChangedVertex)
{
	for (UINT i = 0; i < vertexCount; i++)
	{
		vertices[i].Position.y = ChangedVertex[i];
	}

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}