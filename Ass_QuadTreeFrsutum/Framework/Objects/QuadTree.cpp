#include "Framework.h"
#include "QuadTree.h"

QuadTree::QuadTree(Shader* shader, Frustum* frustum)
	:Renderer(shader), frustum(frustum)
{
}

QuadTree::~QuadTree()
{
	SafeDelete(parentNode);
	SafeDelete(heightMap);
	SafeDelete(baseMap);
}

void QuadTree::Init(Terrain* terrain)
{
	baseMap = terrain->GetBaseMap();
	sBaseMap = shader->AsSRV("BaseMap");
	heightMap = terrain->GetHeightMap();

	float centerX = 0.0f;
	float centerZ = 0.0f;
	float width = 0.0f;

	Shader* test = shader;

	int indexCount = terrain->GetIndexCount();
	// ��������Ʈ�� �� �ﰢ�� ���� �����մϴ�.
	int vertexCount = terrain->GetVertexCount();

	triangleCount = indexCount / 3;

	vertexList = new VertexType[indexCount];
	indexList = new UINT[indexCount];
	if (!vertexList)
	{
		return;
	}

	terrain->CopyVertexArray((void*)vertexList);
	terrain->CopyIndicesArray((void*)indexList);

	CalculateMeshDimensions(vertexCount, centerX, centerZ, width);

	parentNode = new NodeType;
	if (!parentNode)
	{
		return;
	}

	// ���� ��� �����Ϳ� �޽� ������ ������� ���� Ʈ���� ��� ������ �����մϴ�.
	CreateTreeNode(parentNode, centerX, centerZ, width, D3D::GetDevice());

}

void QuadTree::Update()
{
	Super::Update();
}
void QuadTree::Render()
{
	Super::Render();
	search(parentNode, 1);
	if (baseMap != NULL)
		sBaseMap->SetResource(baseMap->SRV());
	// �� �����ӿ� ���� �׷����� �ﰢ���� ���� �ʱ�ȭ�մϴ�.
	drawCount = 0;
	// �θ� ��忡�� �����Ͽ� Ʈ�� �Ʒ��� �̵��Ͽ� ���̴� �� ��带 �������մϴ�.
	RenderNode(parentNode);
}


int QuadTree::CountTriangles(float positionX, float positionZ, float width)
{
	// ī��Ʈ�� 0���� �ʱ�ȭ�Ѵ�.
	int count = 0;

	// ��ü �޽��� ��� �ﰢ���� ���캸�� � ��尡 �� ��� �ȿ� �־�� �ϴ��� Ȯ���մϴ�.
	for (int i = 0; i < triangleCount; i++)
	{
		// �ﰢ���� ��� �ȿ� ������ 1�� ������ŵ�ϴ�.
		if (IsTriangleContained(i, positionX, positionZ, width) == true)
		{
			count++;
		}
	}

	return count;
}

void QuadTree::CalculateMeshDimensions(int vertexCount, float& centerX, float& centerZ, float& meshWidth)
{
	// �޽��� �߽� ��ġ�� 0���� �ʱ�ȭ�մϴ�.
	centerX = 0.0f;
	centerZ = 0.0f;

	// �޽��� ��� ������ ��ģ��.
	for (int i = 0; i < vertexCount; i++)
	{
		centerX += vertexList[i].Position.x;
		centerZ += vertexList[i].Position.z;
	}

	// �׸��� �޽��� �߰� ���� ã�� ���� ������ ���� �����ϴ�.
	centerX = centerX / (float)vertexCount;
	centerZ = centerZ / (float)vertexCount;

	// �޽��� �ִ� �� �ּ� ũ�⸦ �ʱ�ȭ�մϴ�.
	float maxWidth = 0.0f;
	float maxDepth = 0.0f;

	float minWidth = fabsf(vertexList[0].Position.x - centerX);
	float minDepth = fabsf(vertexList[0].Position.z - centerZ);

	// ��� ������ ���캸�� �޽��� �ִ� �ʺ�� �ּ� �ʺ�� ���̸� ã���ϴ�.
	for (int i = 0; i < vertexCount; i++)
	{
		float width = fabsf(vertexList[i].Position.x - centerX);
		float depth = fabsf(vertexList[i].Position.z - centerZ);

		if (width > maxWidth) { maxWidth = width; }
		if (depth > maxDepth) { maxDepth = depth; }
		if (width < minWidth) { minWidth = width; }
		if (depth < minDepth) { minDepth = depth; }
	}

	// �ּҿ� �ִ� ���̿� �ʺ� ������ ���� �ִ� ���� ã���ϴ�.
	float maxX = (float)max(fabs(minWidth), fabs(maxWidth));
	float maxZ = (float)max(fabs(minDepth), fabs(maxDepth));

	// �޽��� �ִ� ������ ����մϴ�.
	meshWidth = max(maxX, maxZ) * 2.0f;
}
void QuadTree::CreateTreeNode(NodeType * node, float positionX, float positionZ, float width, ID3D11Device * device)
{
	// ����� ��ġ�� ũ�⸦ �����Ѵ�.
	node->PositionX = positionX;
	node->PositionZ = positionZ;
	node->Width = width;

	// ����� �ﰢ�� ���� 0���� �ʱ�ȭ�մϴ�.
	node->TriangleCount = 0;

	//���� �� �ε��� ���۸� null�� �ʱ�ȭ�մϴ�.
	node->VertexBuffer = 0;
	node->IndexBuffer = 0;

	// �� ����� �ڽ� ��带 null�� �ʱ�ȭ�մϴ�.
	node->Nodes[0] = 0;
	node->Nodes[1] = 0;
	node->Nodes[2] = 0;
	node->Nodes[3] = 0;

	// �� ��� �ȿ� �ִ� �ﰢ�� ���� ����

	// ��� 2: �� ��忡 �ʹ� ���� �ﰢ���� �ִ� ��� 4 ���� ������ ũ���� �� ���� Ʈ�� ���� �����մϴ�.
	if (width > 63)
	{
		for (int i = 0; i < 4; i++)
		{
			// ���ο� �ڽ� ��忡 ���� ��ġ �������� ����մϴ�.
			float offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f);
			float offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f);

			// �� ��忡 �ﰢ���� �ִ��� Ȯ���մϴ�.
			int count = CountTriangles((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
			if (count > 0)
			{
				// �� �� ��尡�ִ� �ﰢ�����ִ� ��� �ڽ� ��带 ����ϴ�.
				node->Nodes[i] = new NodeType;
				node->transform = new Transform();
				node->transform->Position(positionX, 0, positionZ);
				node->transform->Scale(width, width / 2, width);
				node->collider = new Collider(node->transform);
				// ������ �� �ڽ� ��忡�� �����ϴ� Ʈ���� Ȯ���մϴ�.
				CreateTreeNode(node->Nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f), device);
			}
		}
		return;
	}

	// ��� 3: �� ��尡 ��� �����ʰ� �� ����� �ﰢ�� ���� �ִ� ������ ������ 
	// �� ���� Ʈ���� �� �Ʒ��� �����Ƿ� ������ �ﰢ�� ����� ����ϴ�.

	// ������ ���� ����մϴ�.
	int indexCount = int(width) * int(width) * 6;
	int vertexCount = int(width + 1) * int(width + 1);
	// ���� �迭�� ����ϴ�.
	VertexType * vertices = new VertexType[vertexCount];

	// �ε��� �迭�� ����ϴ�.
	unsigned long* indices = new unsigned long[indexCount];

	// �� ���ο� ���� �� �ε��� �迭�� �ε����� �ʱ�ȭ�մϴ�.
	int index = 0;
	int index2 = 0;
	int vertexIndex = 0;
	int indexNum = 0;
	int count1 = (eTerrainSize);
	int count2 = (eTerrainSize);

	//���� ����� ��� �ﰢ���� ���� ���ϴ�.
	for (UINT i = 0; i < count1; i++)
	{
		for (UINT j = 0; j < count2; j++)
		{
			// �ﰢ������ ��� �ȿ� ������ ������ �迭�� �߰��մϴ�.
			if (IsTriangleContained((i * (count1)) + (j), positionX, positionZ, width) == true)
			{
				vertices[index].Position = vertexList[i * (count1)+j].Position;
				vertices[index].Uv = vertexList[i * (count1)+j].Uv;
				vertices[index].Normal = vertexList[i * (count1)+j].Normal;
				if (i != (count1 - 1) && j != (count2 - 1))
				{
					indices[index2 + 0] = count1 * i + j;
					indices[index2 + 1] = count1 * (i + 1) + j;
					indices[index2 + 2] = count1 * i + (j + 1);
					indices[index2 + 3] = count1 * i + (j + 1);
					indices[index2 + 4] = count1 * (i + 1) + j;
					indices[index2 + 5] = count1 * (i + 1) + (j + 1);
				}
				index2 += 6;
				index++;
			}

		}
	}

	node->VertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexType) * vertexCount, 0, true);
	node->IndexBuffer = new IndexBuffer(indices, indexCount);

	// ���� ����� ���ۿ� �����Ͱ� ����ǹǷ� �������� �ε��� �迭�� �����մϴ�.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;
}

bool QuadTree::IsTriangleContained(int index, float positionX, float positionZ, float width)
{
	if (index > 256 * 255)
	{
		return true;
	}
	// �� ����� �ݰ��� ����մϴ�.
	float radius = width / 2.0f;

	// �ε����� ���� ������� �����ɴϴ�.
	// vertexIndex
	int vertexIndex = index % 2;

	float x1 = 0.0f;
	float z1 = 0.0f;
	float x2 = 0.0f;
	float z2 = 0.0f;
	float x3 = 0.0f;
	float z3 = 0.0f;

	// ���� ��Ͽ��� �� �ﰢ���� �� �������� �����ɴϴ�.
	if (vertexIndex == 0)
	{
		x1 = vertexList[index].Position.x;
		z1 = vertexList[index].Position.z;

		x2 = vertexList[index + 255 + 1].Position.x;
		z2 = vertexList[index + 255 + 1].Position.z;

		x3 = vertexList[index + 1].Position.x;
		z3 = vertexList[index + 1].Position.z;
	}
	if (vertexIndex == 1)
	{

		x1 = vertexList[index].Position.x;
		z1 = vertexList[index].Position.z;

		x2 = vertexList[index + 255].Position.x;
		z2 = vertexList[index + 255].Position.z;

		x3 = vertexList[index + 255 + 1].Position.x;
		z3 = vertexList[index + 255 + 1].Position.z;
	}

	// �ﰢ���� x ��ǥ�� �ּҰ��� ��� �ȿ� �ִ��� Ȯ���Ͻʽÿ�.
	float minimumX = min(x1, min(x2, x3));
	if (minimumX > (positionX + radius))
	{
		return false;
	}

	// �ﰢ���� x ��ǥ�� �ִ� ���� ��� �ȿ� �ִ��� Ȯ���Ͻʽÿ�.
	float maximumX = max(x1, max(x2, x3));
	if (maximumX < (positionX - radius))
	{
		return false;
	}

	// �ﰢ���� z ��ǥ�� �ּҰ��� ��� �ȿ� �ִ��� Ȯ���Ͻʽÿ�.
	float minimumZ = min(z1, min(z2, z3));
	if (minimumZ > (positionZ + radius))
	{
		return false;
	}
	// �ﰢ���� z ��ǥ�� �ִ� ���� ��� �ȿ� �ִ��� Ȯ���Ͻʽÿ�.
	float maximumZ = max(z1, max(z2, z3));
	if (maximumZ < (positionZ - radius))
	{
		return false;
	}

	return true;
}

void QuadTree::search(NodeType * parent, float count)
{
	if (count > 4) return;

	if (parent->Nodes[0]->Width > 31)
	{
		parent->Nodes[0]->collider->Render(Color(count / 3, count / 3, count / 3, 1));
		search(parent->Nodes[0], count + 1);
	}
	if (parent->Nodes[1]->Width > 31)
	{
		parent->Nodes[1]->collider->Render(Color(count / 3, count / 3, count / 3, 1));
		search(parent->Nodes[1], count + 1);
	}
	if (parent->Nodes[2]->Width > 31)
	{
		parent->Nodes[2]->collider->Render(Color(count / 3, count / 3, count / 3, 1));
		search(parent->Nodes[2], count + 1);
	}
	if (parent->Nodes[3]->Width > 31)
	{
		parent->Nodes[3]->collider->Render(Color(count / 3, count / 3, count / 3, 1));
		search(parent->Nodes[3], count + 1);
	}
}

void QuadTree::RenderNode(NodeType * node)
{

	int indexCount;
	unsigned int stride, offset;


	// Check to see if the node can be viewed, height doesn't matter in a quad tree.



	// If it can't be seen then none of its children can either so don't continue down the tree, this is where the speed is gained.
	bool result[4] = { true,true,true,true };
	//������
	if (!frustum->CheckPoint(Vector3(node->PositionX - (node->Width / 2), 0, node->PositionZ + (node->Width / 2))))
	{
		result[0] = false;
	}
	if (!frustum->CheckPoint(Vector3(node->PositionX + (node->Width / 2), 0, node->PositionZ + (node->Width / 2))))
	{
		result[1] = false;
	}
	if (!frustum->CheckPoint(Vector3(node->PositionX + (node->Width / 2), 0, node->PositionZ - (node->Width / 2))))
	{
		result[2] = false;
	}
	if (!frustum->CheckPoint(Vector3(node->PositionX - (node->Width / 2), 0, node->PositionZ - (node->Width / 2))))
	{
		result[3] = false;
	}

	int cnt = 0;
	// If it can be seen then check all four child nodes to see if they can also be seen.
	for (int i = 0; i < 4; i++)
	{
		if (false == result[i])
		{
			cnt++;
			if (node->Nodes[i] != 0)
			{
				RenderNode(node->Nodes[i]);
			}
		}
	}



	//if (cnt != 0)
	//{
	//	return;
	//}
	// If there were any children nodes then there is no need to continue as parent nodes won't contain any triangles to render.


	// Otherwise if this node can be seen and has triangles in it then render these triangles.

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;


	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Determine the number of indices in this node.
	indexCount = node->TriangleCount * 3;

	// Call the terrain shader to render the polygons in this node.
	shader->DrawIndexed(0, 1, indexCount);

	// Increase the count of the number of polygons that have been rendered during this frame.
	drawCount += node->TriangleCount;

	return;
}
