#include "Framework.h"
#include "TerrainLod.h"

TerrainLod::TerrainLod(InitialDesc& initDesc)
	:Renderer(initDesc.shader), initDesc(initDesc)
{
	Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	sBaseMap = shader->AsSRV("BaseMap");
	sLayerMap = shader->AsSRV("LayerMap");
	sAlphaMap = shader->AsSRV("AlphaMap");
	sHeightMap = shader->AsSRV("HeightMap");
	sNormalMap = shader->AsSRV("NormalMap");

	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_TerrainLod");

	heightMap = new Texture(initDesc.HeightMap);
	sHeightMap->SetResource(heightMap->SRV());
	heightMap->ReadPixel(DXGI_FORMAT_R8G8B8A8_UINT, &heightMapPixel);

	width = this->heightMap->GetWidth()-1;
	height = this->heightMap->GetHeight() - 1;

	vertexPerPatchX = (width / initDesc.CellsPerPatch) + 1;
	vertexPerPatchZ = (height / initDesc.CellsPerPatch) + 1;

	vertexCount = vertexPerPatchX * vertexPerPatchZ;
	faceCount = (vertexPerPatchX - 1) * (vertexPerPatchZ - 1);
	indexCount = faceCount * 4;

	CalcBoundY();
	CreateVertexData();
	CreateIndexData();

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTerrain));
	indexBuffer = new IndexBuffer(indices, indexCount);

	bufferDesc.TexelCellSpaceU = 1.0f / (float)heightMap->GetWidth() - 1; 
	bufferDesc.TexelCellSpaceV = 1.0f / (float)heightMap->GetHeight() - 1; 
	bufferDesc.TerrainHeightRatio = initDesc.HeightRatio;
	
	camera = new Fixity();
	perspective = new Perspective(D3D::Width(), D3D::Height(), 0.1f, 1000.0f, Math::PI * 0.35f);

	frustum = new Frustum(NULL, perspective);
}

TerrainLod::~TerrainLod()
{
	SafeDelete(vertices);
	SafeDelete(indices);
	SafeDelete(buffer);
	SafeDelete(frustum);
	SafeDelete(camera);
	SafeDelete(perspective);

	SafeDelete(heightMap);
	SafeDelete(layerMap);
	SafeDelete(alphaMap);
	SafeDelete(baseMap);
	SafeDelete(normalMap);
}

void TerrainLod::Update()
{
	Super::Update();
	camera->Update();

	frustum->Update();
	frustum->Planes(bufferDesc.WorldFrustumPlanes);
}

void TerrainLod::Render()
{
	Super::Render();
	if (baseMap != NULL)
		sBaseMap->SetResource(baseMap->SRV());

	if (layerMap != NULL)
		sLayerMap->SetResource(layerMap->SRV());
	if (alphaMap != NULL)
		sAlphaMap->SetResource(alphaMap->SRV());
	if (normalMap != NULL)
		sNormalMap->SetResource(normalMap->SRV());

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
	shader->DrawIndexed(0, Pass(), indexCount);
}

void TerrainLod::BaseMap(wstring file)
{
	SafeDelete(baseMap);

	baseMap = new Texture(file);
}

void TerrainLod::LayerMap(wstring layer, wstring alpha)
{
	SafeDelete(layerMap);
	SafeDelete(alphaMap);

	alphaMap = new Texture(alpha);
	layerMap = new Texture(layer);
}

void TerrainLod::NormalMap(wstring file)
{
	SafeDelete(normalMap);
	normalMap = new Texture(file);
}

bool TerrainLod::InBounds(UINT x, UINT z)
{
	return x>= 0 && x< width && z>=0 && z < height;
}

void TerrainLod::CalcPatchBounds(UINT x, UINT z)
{
	UINT x0 = x * initDesc.CellsPerPatch;
	UINT x1 = (x + 1) * initDesc.CellsPerPatch;

	UINT z0 = z * initDesc.CellsPerPatch;
	UINT z1 = (z + 1) * initDesc.CellsPerPatch;

	float minY = FLT_MAX;
	float maxY = FLT_MIN;
	for (UINT z = z0; z <= z1; z++)
	{
		for (UINT x = x0; x <= x1; x++)
		{
			float y = 0.0f;
			UINT pixel = width * (height - 1 - z) + x;

			if (InBounds(x, z))
				y = heightMapPixel[pixel].b * 255 / initDesc.HeightRatio;

			minY = min(minY, y);
			maxY = max(maxY, y);
		}
	}

	UINT patchID = (vertexPerPatchX - 1) * z + x;
	bounds[patchID] = Vector2(minY, maxY);
}

void TerrainLod::CalcBoundY()
{
	bounds.assign(faceCount, Vector2());

	for (UINT z = 0; z < vertexPerPatchZ-1; z++)
	{
		for (UINT x = 0; x < vertexPerPatchX-1; x++)
		{
			CalcPatchBounds(x, z);
		}
	}
}

void TerrainLod::CreateVertexData()
{
}

void TerrainLod::CreateIndexData()
{
}
