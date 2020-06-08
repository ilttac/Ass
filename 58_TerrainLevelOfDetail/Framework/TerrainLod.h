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
	bool InBounds(UINT x, UINT z);//셀영역에 들어와있는지 안들어와있는지.
	void CalcPatchBounds(UINT x, UINT Z);//셀단위
	void CalcBoundY();//전체

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
		float MaxDistance = 500.0f;
		float MinTessellation = 1.0f;
		float MaxTessellation = 64.0f;

		///지면 한조각
		float TexelCellSpaceU;
		float TexelCellSpaceV;
		float WorldCellSpace = 1.0f;//지면 한칸의 킈기
		float TerrainHeightRatio = 1.0f;//높이를 너무크거나 저무작게 나오지않게 하는 변수.

		Vector2 TextureScale = D3DXVECTOR2(1,1);
		float Padding[2];

		Plane WorldFrustumPlanes[6];
	}bufferDesc;

private:
	UINT faceCount; //면의개수
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

	vector<Vector2> bounds; //높이만 담는다.

	Texture* baseMap = NULL;
	ID3DX11EffectShaderResourceVariable* sBaseMap;
	
	Texture* layerMap = NULL;
	ID3DX11EffectShaderResourceVariable* sLayerMap;
	
	Texture* alphaMap = NULL;
	ID3DX11EffectShaderResourceVariable* sAlphaMap;

	Texture* normalMap = NULL;
	ID3DX11EffectShaderResourceVariable* sNormalMap;
};