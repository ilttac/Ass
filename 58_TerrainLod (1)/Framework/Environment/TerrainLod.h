#pragma once

class TerrainLod : public Renderer
{
public:
	struct InitialDesc
	{
		Shader* shader;
		
		wstring HeightMap;
		float CellSpacing;
		UINT CellsPerPatch;
		float HeightRatio;
	};

public:
	TerrainLod(InitialDesc& initDesc);
	~TerrainLod();

	void Update();
	void Render();

	void BaseMap(wstring file);
	void LayerMap(wstring layer, wstring alpha);
	void NormalMap(wstring file);

private:
	bool InBounds(UINT x, UINT z);
	void CalcPatchBounds(UINT x, UINT z);
	void CalcBoundY();

	void CreateVertexData();
	void CreateIndexData();

private:
	struct VertexTerrain
	{
		Vector3 Position;
		Vector2 Uv;
		Vector2 BoundY;
	};

private:
	struct BufferDesc
	{
		float MinDistance = 1.0f;
		float MaxDistance = 200.0f;
		float MinTessellation = 1.0f;
		float MaxTessellation = 64.0f;

		float TexelCellSpaceU;
		float TexelCellSpaceV;
		float WorldCellSpace = 1.0f;
		float TerrainHeightRatio = 1.0f;

		Vector2 TextureScale = D3DXVECTOR2(1, 1);
		float Padding[2];

		Plane WorldFrustumPlanes[6];
	} bufferDesc;

private:
	UINT faceCount;

	UINT vertexPerPatchX;
	UINT vertexPerPatchZ;

private:
	InitialDesc initDesc;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	UINT width, height;

	VertexTerrain* vertices;
	UINT* indices;

	Frustum* frustum;
	Camera* camera;
	Perspective* perspective;

	Texture* heightMap;
	vector<D3DXCOLOR> heightMapPixel;
	ID3DX11EffectShaderResourceVariable* sHeightMap;

	vector<Vector2> bounds;

	Texture* baseMap = NULL;
	ID3DX11EffectShaderResourceVariable* sBaseMap;
	
	Texture* layerMap = NULL;
	ID3DX11EffectShaderResourceVariable* sLayerMap;
	
	Texture* alphaMap = NULL;
	ID3DX11EffectShaderResourceVariable* sAlphaMap;

	Texture* normalMap = NULL;
	ID3DX11EffectShaderResourceVariable* sNormalMap;
};