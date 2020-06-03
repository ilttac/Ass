#pragma once

class QuadTree
{
public:
	QuadTree();
	~QuadTree();

private:
	struct VertexType
	{
		Vector3 Position;
		Vector2 Uv;
		Vector3 Normal; //ÇÊ¿ä?
	};

	struct NodeType
	{
		float PositionX, PositionZ, Width;
		int TriangleCount;
		Transform* transform;
		Collider* collider;
		ID3D11Buffer* VertexBuffer, * IndexBuffer;
		NodeType* Nodes[4];
	}nodeType;

public:
	void Init(class Terrain* terrain);
	void Update();
	void Render();

private:
	int CountTriangles(float positionX, float positionZ, float width);
	void CalculateMeshDimensions(int vertexCount, float& centerX, float& centerZ, float& meshWidth);
	void CreateTreeNode(NodeType* node, float positionX, float positionZ, float width, ID3D11Device* device);
	bool IsTriangleContained(int index, float positionX, float positionZ, float width);
	void search(NodeType* parent);
	
private:
	int triangleCount, drawCount;
	VertexType* vertexList = nullptr;
	NodeType* parentNode = nullptr;

};