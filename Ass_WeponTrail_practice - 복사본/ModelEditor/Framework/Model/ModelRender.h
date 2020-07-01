#pragma once

class ModelRender
{
public:
	ModelRender(Shader* shader);
	~ModelRender();

	void Update();
	void Render();

public:	
	Model* GetModel() { return model; }

public:
	void ReadMaterial(wstring file);
	void ReadMesh(wstring file);

	void Pass(UINT pass);

	Transform* AddTransform();
	Transform* GetTransform(UINT index) { return transforms[index]; }
	void UpdateTransforms();

	void UpdateTransform(UINT instanceId, UINT boneIndex, Transform& transform);
	void UpdateboneTransform(UINT instanceId, UINT boneIndex);
public:
	void CreateTexture();
private:
	struct ChangedBoneDesc
	{
		UINT ChangedBoneIndex;
		float Padding[3];

		Matrix ChagnedTransfrom;
	} changedBoneDesc;

private:
	ConstantBuffer* boneBuffer;
	ID3DX11EffectConstantBuffer* sBoneBuffer;

private:	
	Shader * shader;
	Model* model;	

	Matrix bones[MAX_MODEL_TRANSFORMS];
	Matrix boneTransforms[MAX_MODEL_INSTANCE][MAX_MODEL_TRANSFORMS];

	vector<Transform*> transforms;
	Matrix worlds[MAX_MESH_INSTANCE];

	VertexBuffer* instanceBuffer;



	ID3D11Texture2D* texture = NULL;
	ID3D11ShaderResourceView* srv;
};