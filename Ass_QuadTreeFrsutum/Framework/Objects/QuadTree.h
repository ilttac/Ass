#pragma once


class QuadTree : public Renderer
{
public:
	QuadTree(Shader* shader, Frustum* frsutum);
	~QuadTree();

private:
	struct VertexType
	{
		Vector3 Position;
		Vector2 Uv;
		Vector3 Normal; //�ʿ�?
	};

	struct NodeType
	{
		float PositionX, PositionZ, Width;
		int TriangleCount;
		Transform* transform;
		Collider* collider;
		VertexBuffer* VertexBuffer;
		IndexBuffer* IndexBuffer;
		NodeType* Nodes[4];
	}nodeType;

private:
	enum {eTerrainSize =256};

public:
	void Init(class Terrain* terrain);
	void Update();
	void Render();
	Transform* GetTransform() { return transform; }

private:
	int CountTriangles(float positionX, float positionZ, float width);
	void CalculateMeshDimensions(int vertexCount, float& centerX, float& centerZ, float& meshWidth);
	void CreateTreeNode(NodeType* node, float positionX, float positionZ, float width, ID3D11Device* device);
	bool IsTriangleContained(int index, float positionX, float positionZ, float width);
	void search(NodeType* parent,float count);
	void RenderNode(NodeType* node);

private:
	int triangleCount, drawCount;
	VertexType* vertexList = nullptr;
	UINT* indexList = nullptr;
	NodeType* parentNode = nullptr;
	Transform* transform = nullptr;
	Frustum* frustum = nullptr;

	Texture* heightMap;
	Texture* baseMap = NULL;
	ID3DX11EffectShaderResourceVariable* sBaseMap;

};