#pragma once

class Billboard : public Renderer
{
public:
	Billboard(Shader* shader);
	~Billboard();

	void Add(Vector3& position, Vector2& scale,UINT mapIndex);
	void AddTexture(wstring file);

	void Update();
	void Render();

private:	
	struct  VertexScale
	{
		Vector3 Position;
		Vector2 Scale;
		UINT MapIndex;
	};

private:
	vector<VertexScale> vertices;

	vector<wstring> textureFiles;
	TextureArray* textures = NULL;
	ID3DX11EffectShaderResourceVariable* sMaps;
};