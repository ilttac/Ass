#pragma once

class Converter
{
public:
	Converter();
	~Converter();

	void ReadFile(wstring file);

public:
	void ExportMaterial(wstring savePath, bool bOverwirte = true);

private:
	void ReadMaterial();
	void WriteMaterial(wstring savePath, bool bOverWrite);
	string WriteTexture(string savePath, string file);

public:
	void ExportMesh(wstring savePath, bool bOverwirte = true);

private:
	void ReadBoneData(aiNode* node, int index, int parent);
	void ReadMeshData(aiNode* node, int bone);
	void ReadSkinData();
	void WriteMeshData(wstring savePath, bool bOverwrite);

public:
	void ClipList(vector<wstring>* list);
	void ExportAnimClip(UINT index, wstring savePath, bool bOverwrite = true);

private:
	struct asClip* ReadClipData(aiAnimation* animation);
	void ReadKeyFrameData(struct asClip* clip, aiNode* node, vector<struct asClipNode>& aiNodeInfos);
	void WriteClipData(struct asClip* clip, wstring savePath, bool bOverwrite);


private:
	wstring file;

	Assimp::Importer* importer;
	const aiScene* scene;

	vector<struct asMaterial*> materials;

	vector<struct asBone*> bones;
	vector<struct asMesh*> meshes;
};