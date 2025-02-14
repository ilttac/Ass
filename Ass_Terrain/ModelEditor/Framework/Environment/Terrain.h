#pragma once

class Terrain : public Renderer
{
public:
	typedef VertexTextureNormal TerrainVertex;
	friend class SceneEditor;
public:
	Terrain(Shader* shader, wstring heightFile);
	~Terrain();

	void Update();
	void Render();

	void BaseMap(wstring file);
	void LayerMap(wstring file, wstring alpha);
	void LayerMap(wstring file);
	void AlphaMap(wstring file);

	float GetHeight(Vector3& position);
	float GetHeightPick(Vector3& position);
	Vector3 GetPickedPosition();

	void SaveTerrain(wstring file);
	void ChangeVertex(vector<float> ChangedVertex);

	Texture* GetBaseMap() { return baseMap; }
	Texture* GetLayerMap() { return layerMap; }
	Texture* GetAlphaMap() { return alphaMap; }

private:
	void CreateVertexData();
	void CreateIndexData();
	void CreateNormalData();

	void RaiseHeightRect(Vector3& position, UINT type, UINT range);
	void descHeight(Vector3& position, UINT type, UINT range);
	void MakeNoise(Vector3& position, UINT type, UINT range);
	void MakeSmooth(Vector3& position, UINT type, UINT range);
	void MakeFlat(Vector3& position, UINT type, UINT range);
	void MakeSlope(Vector3& oldposition, Vector3& newposition, UINT type, UINT range);

private:
	struct BrushDesc
	{
		Color Color = D3DXCOLOR(0, 1, 0, 1);//16
		Vector3 Location; //12
		UINT Type = 0; //4
		UINT Range = 1;//4
		bool Noise = false;//1
		bool Smooth = false;//1
		bool Flat = false;//1
		bool Slope = false;//1
		Vector3 LateLocation;//12
		Vector3 MousePos;//12
		Vector3 LateMousePos;//12
		UINT SlopeSwitch = 0;
	} brushDesc;

	struct LineDesc
	{
		Color Color = D3DXCOLOR(1, 1, 1, 1);
		UINT Visible = 1;
		float Thickness = 0.01f;
		float Size = 5.0f;
		float Padding;
	} lineDesc;

	struct HeigthDesc
	{
		float positionY = 0;
	};
public:
	BrushDesc& GetbrushDesc() { return brushDesc; }
	LineDesc& GetLineDesc() { return lineDesc; }

private:
	Texture* heightMap;
	string heightMapFileName;
	UINT width;
	UINT height;

	TerrainVertex* vertices;
	UINT* indices;

	Texture* baseMap = NULL;
	ID3DX11EffectShaderResourceVariable* sBaseMap;

	Texture* layerMap = NULL;
	ID3DX11EffectShaderResourceVariable* sLayerMap;

	Texture* alphaMap = NULL;
	ID3DX11EffectShaderResourceVariable* sAlphaMap;

	ConstantBuffer* brushBuffer;
	ID3DX11EffectConstantBuffer* sBrushBuffer;

	ConstantBuffer* lineBuffer;
	ID3DX11EffectConstantBuffer* sLineBuffer;

};