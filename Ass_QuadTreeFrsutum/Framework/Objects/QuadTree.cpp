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
	CreateTreeNode(parentNode, centerX, centerZ, width);

}

void QuadTree::Update()
{
	Super::Update();
	ImGui::InputInt("RenderMode", &renderMode);
	renderMode %= 2;
}
void QuadTree::Render()
{
	Super::Render();
	search(parentNode, 1);
	if (baseMap != NULL)
		sBaseMap->SetResource(baseMap->SRV());
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
void QuadTree::CreateTreeNode(NodeType * node, float positionX, float positionZ, float width)
{
	int numTriangles;

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
	// Count the number of triangles that are inside this node.
	numTriangles = CountTriangles(positionX, positionZ, width);

	// Case 1: If there are no triangles in this node then return as it is empty and requires no processing.
	if (numTriangles == 0)
	{
		return;
	}
	if (width > 15)
	{
		for (int i = 0; i < 4; i++)
		{
			// ���ο� �ڽ� ��忡 ���� ��ġ �������� ����մϴ�.
			float offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f);
			float offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f);

			node->transform = new Transform();
			node->transform->Position(positionX, 0, positionZ);
			node->transform->Scale(width, width / 2, width);
			node->collider = new Collider(node->transform);

			//�ٿ�ĳ����
			int tWidth = static_cast<int>(width / 2);//���� 127
			int index = 0;
			int index2 = 0;
			int terrainSize = (eTerrainSize); //256

			if (width <= 16)
			{
				// ���� �迭�� ����ϴ�.
				VertexType* vertices = new VertexType[(width + 2) * (width + 2)];

				// �ε��� �迭�� ����ϴ�.
				UINT * indices = new UINT[(width + 1) * (width + 1) * 6];
				assert(positionZ - tWidth >= 0);
				assert(positionX - tWidth >= 0);

				for (UINT y = UINT(positionZ) - tWidth; y <= UINT(positionZ) + tWidth + 2; y++)
				{
					for (UINT x = UINT(positionX) - tWidth; x <= UINT(positionX) + tWidth + 2; x++)
					{
						if (y == terrainSize || x == terrainSize)
						{
							break;
						}
						vertices[index].Position = vertexList[y * (terrainSize)+x].Position;
						vertices[index].Uv = vertexList[y * (terrainSize)+x].Uv;

						vertices[index].Normal = vertexList[y * (terrainSize)+x].Normal;
						if ((UINT(positionZ) + tWidth + 2 == terrainSize) && (UINT(positionX) + tWidth + 2 == terrainSize)) //�𼭸� ���� ������
						{
							if (y != (UINT(positionZ) + tWidth + 1) && x != (UINT(positionX) + tWidth + 1))
							{
								indices[index2 + 0] = index;
								indices[index2 + 1] = index + int(width) + 1;
								indices[index2 + 2] = index + 1;
								indices[index2 + 3] = index + 1;
								indices[index2 + 4] = index + int(width) + 1;
								indices[index2 + 5] = index + int(width) + 2;
								index2 += 6;
							}
						}
						else if ((UINT(positionZ) + tWidth + 2 == terrainSize)) //�𼭸� ����
						{
							if (y != (UINT(positionZ) + tWidth + 1) && x != (UINT(positionX) + tWidth + 2))
							{
								indices[index2 + 0] = index;
								indices[index2 + 1] = index + int(width) + 2;
								indices[index2 + 2] = index + 1;
								indices[index2 + 3] = index + 1;
								indices[index2 + 4] = index + int(width) + 2;
								indices[index2 + 5] = index + int(width) + 3;
								index2 += 6;
							}
						}
						else if ((UINT(positionX) + tWidth + 2 == terrainSize)) //�𼭸� ������
						{
							if (y != (UINT(positionZ) + tWidth + 2) && x != (UINT(positionX) + tWidth + 1))
							{
								indices[index2 + 0] = index;
								indices[index2 + 1] = index + int(width) + 1;
								indices[index2 + 2] = index + 1;
								indices[index2 + 3] = index + 1;
								indices[index2 + 4] = index + int(width) + 1;
								indices[index2 + 5] = index + int(width) + 2;
								index2 += 6;
							}
						}
						else if (y != (UINT(positionZ) + tWidth + 2) && x != (UINT(positionX) + tWidth + 2))
						{
							indices[index2 + 0] = index;
							indices[index2 + 1] = index + int(width) + 2;
							indices[index2 + 2] = index + 1;
							indices[index2 + 3] = index + 1;
							indices[index2 + 4] = index + int(width) + 2;
							indices[index2 + 5] = index + int(width) + 3;
							index2 += 6;
						}

						index++;
					}

					
				}
				//���߿� ���缭 �ַ��� ����
				node->vertexCnt = index;
				node->indexCnt = index2;
				node->TriangleCount = index2/3;
				node->VertexBuffer = new VertexBuffer(vertices, node->vertexCnt, sizeof(VertexType), 0, true);
				node->IndexBuffer = new IndexBuffer(indices, node->indexCnt);
				delete[] vertices;
				vertices = 0;

				delete[] indices;
				indices = 0;
			}

			// ������ �� �ڽ� ��忡�� �����ϴ� Ʈ���� Ȯ���մϴ�.
			if (width < 31)
			{
				continue;
			}
			node->Nodes[i] = new NodeType;
			CreateTreeNode(node->Nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));

		}
		return;
	}
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
	if (parent->Nodes[0] == NULL)
	{
		return;
	}
	if (parent->Nodes[0]->Width > 15)
	{
		parent->Nodes[0]->collider->Render(Color(0.8f / (count), 0.7f / (count ), 1.2f / (count ), 1));
		search(parent->Nodes[0], count + 0.5f);
	}
	if (parent->Nodes[1]->Width > 15)
	{
		parent->Nodes[1]->collider->Render(Color(0.8f / (count ), 0.7f / (count ), 1.2f / (count ), 1));
		search(parent->Nodes[1], count + 0.5f);
	}
	if (parent->Nodes[2]->Width > 15)
	{
		parent->Nodes[2]->collider->Render(Color(0.8f / (count), 0.7f / (count ), 1.2f / (count), 1));
		search(parent->Nodes[2], count + 0.5f);
	}
	if (parent->Nodes[3]->Width > 15)
	{
		parent->Nodes[3]->collider->Render(Color(0.8f / (count ), 0.7f / (count ), 1.2f / (count), 1));
		search(parent->Nodes[3], count + 0.5f);
	}
}

void QuadTree::RenderNode(NodeType * node)
{

	int indexCount;
	unsigned int stride, offset;

	// ??? �����̻��� 
	//bool result[4] = { true,true,true,true };
	////������
	//if (!frustum->CheckPoint(Vector3(node->PositionX - (node->Width / 2), 0, node->PositionZ + (node->Width / 2))))
	//{
	//	result[0] = false;
	//}
	//if (!frustum->CheckPoint(Vector3(node->PositionX + (node->Width / 2), 0, node->PositionZ + (node->Width / 2))))
	//{
	//	result[1] = false;
	//}
	//if (!frustum->CheckPoint(Vector3(node->PositionX + (node->Width / 2), 0, node->PositionZ - (node->Width / 2))))
	//{
	//	result[2] = false;
	//}
	//if (!frustum->CheckPoint(Vector3(node->PositionX - (node->Width / 2), 0, node->PositionZ - (node->Width / 2))))
	//{
	//	result[3] = false;
	//}
	bool result = frustum->CheckCube(Vector3(node->PositionX, 0.0f, node->PositionZ), (node->Width / 2.0f));
	if (!result)
	{
		return;
	}
	else
	{
		int b = 10;
	}
	int count = 0;

	for (int i = 0; i < 4; i++)
	{
		count++;
		if (node->Nodes[i] != 0)
		{
			RenderNode(node->Nodes[i]);
		}
	}

	// If it can be seen then check all four child nodes to see if they can also be seen.

	if (node->Nodes[0] == NULL)
	{
		//Set vertex buffer stride and offset.
		stride = sizeof(VertexType);
		offset = 0;

		if (node->VertexBuffer == NULL)
		{
			return;
		}
		// Set the vertex buffer to active in the input assembler so it can be rendered.
		node->VertexBuffer->Render();
		// Set the index buffer to active in the input assembler so it can be rendered.
		node->IndexBuffer->Render();
		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Determine the number of indices in this node.
		int indexCount = node->indexCnt;

		// Call the terrain shader to render the polygons in this node.

		shader->DrawIndexed(0, renderMode, indexCount);

		// Increase the count of the number of polygons that have been rendered during this frame.
		drawCount += node->TriangleCount;
		return;
	}


}
