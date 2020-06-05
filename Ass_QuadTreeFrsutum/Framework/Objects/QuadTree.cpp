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
	// 정점리스트의 총 삼각형 수를 저장합니다.
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

	// 정점 목록 데이터와 메쉬 차원을 기반으로 쿼드 트리를 재귀 적으로 빌드합니다.
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
	// 이 프레임에 대해 그려지는 삼각형의 수를 초기화합니다.
	drawCount = 0;
	// 부모 노드에서 시작하여 트리 아래로 이동하여 보이는 각 노드를 렌더링합니다.
	RenderNode(parentNode);
}


int QuadTree::CountTriangles(float positionX, float positionZ, float width)
{
	// 카운트를 0으로 초기화한다.
	int count = 0;

	// 전체 메쉬의 모든 삼각형을 살펴보고 어떤 노드가 이 노드 안에 있어야 하는지 확인합니다.
	for (int i = 0; i < triangleCount; i++)
	{
		// 삼각형이 노드 안에 있으면 1씩 증가시킵니다.
		if (IsTriangleContained(i, positionX, positionZ, width) == true)
		{
			count++;
		}
	}

	return count;
}

void QuadTree::CalculateMeshDimensions(int vertexCount, float& centerX, float& centerZ, float& meshWidth)
{
	// 메쉬의 중심 위치를 0으로 초기화합니다.
	centerX = 0.0f;
	centerZ = 0.0f;

	// 메쉬의 모든 정점을 합친다.
	for (int i = 0; i < vertexCount; i++)
	{
		centerX += vertexList[i].Position.x;
		centerZ += vertexList[i].Position.z;
	}

	// 그리고 메쉬의 중간 점을 찾기 위해 정점의 수로 나눕니다.
	centerX = centerX / (float)vertexCount;
	centerZ = centerZ / (float)vertexCount;

	// 메쉬의 최대 및 최소 크기를 초기화합니다.
	float maxWidth = 0.0f;
	float maxDepth = 0.0f;

	float minWidth = fabsf(vertexList[0].Position.x - centerX);
	float minDepth = fabsf(vertexList[0].Position.z - centerZ);

	// 모든 정점을 살펴보고 메쉬의 최대 너비와 최소 너비와 깊이를 찾습니다.
	for (int i = 0; i < vertexCount; i++)
	{
		float width = fabsf(vertexList[i].Position.x - centerX);
		float depth = fabsf(vertexList[i].Position.z - centerZ);

		if (width > maxWidth) { maxWidth = width; }
		if (depth > maxDepth) { maxDepth = depth; }
		if (width < minWidth) { minWidth = width; }
		if (depth < minDepth) { minDepth = depth; }
	}

	// 최소와 최대 깊이와 너비 사이의 절대 최대 값을 찾습니다.
	float maxX = (float)max(fabs(minWidth), fabs(maxWidth));
	float maxZ = (float)max(fabs(minDepth), fabs(maxDepth));

	// 메쉬의 최대 직경을 계산합니다.
	meshWidth = max(maxX, maxZ) * 2.0f;
}
void QuadTree::CreateTreeNode(NodeType * node, float positionX, float positionZ, float width, ID3D11Device * device)
{
	// 노드의 위치와 크기를 저장한다.
	node->PositionX = positionX;
	node->PositionZ = positionZ;
	node->Width = width;

	// 노드의 삼각형 수를 0으로 초기화합니다.
	node->TriangleCount = 0;

	//정점 및 인덱스 버퍼를 null로 초기화합니다.
	node->VertexBuffer = 0;
	node->IndexBuffer = 0;

	// 이 노드의 자식 노드를 null로 초기화합니다.
	node->Nodes[0] = 0;
	node->Nodes[1] = 0;
	node->Nodes[2] = 0;
	node->Nodes[3] = 0;

	// 이 노드 안에 있는 삼각형 수를 센다

	// 사례 2: 이 노드에 너무 많은 삼각형이 있는 경우 4 개의 동일한 크기의 더 작은 트리 노드로 분할합니다.
	if (width > 63)
	{
		for (int i = 0; i < 4; i++)
		{
			// 새로운 자식 노드에 대한 위치 오프셋을 계산합니다.
			float offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f);
			float offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f);

			// 새 노드에 삼각형이 있는지 확인합니다.
			int count = CountTriangles((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
			if (count > 0)
			{
				// 이 새 노드가있는 삼각형이있는 경우 자식 노드를 만듭니다.
				node->Nodes[i] = new NodeType;
				node->transform = new Transform();
				node->transform->Position(positionX, 0, positionZ);
				node->transform->Scale(width, width / 2, width);
				node->collider = new Collider(node->transform);
				// 이제이 새 자식 노드에서 시작하는 트리를 확장합니다.
				CreateTreeNode(node->Nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f), device);
			}
		}
		return;
	}

	// 사례 3: 이 노드가 비어 있지않고 그 노드의 삼각형 수가 최대 값보다 작으면 
	// 이 노드는 트리의 맨 아래에 있으므로 저장할 삼각형 목록을 만듭니다.

	// 정점의 수를 계산합니다.
	int indexCount = int(width) * int(width) * 6;
	int vertexCount = int(width + 1) * int(width + 1);
	// 정점 배열을 만듭니다.
	VertexType * vertices = new VertexType[vertexCount];

	// 인덱스 배열을 만듭니다.
	unsigned long* indices = new unsigned long[indexCount];

	// 이 새로운 정점 및 인덱스 배열의 인덱스를 초기화합니다.
	int index = 0;
	int index2 = 0;
	int vertexIndex = 0;
	int indexNum = 0;
	int count1 = (eTerrainSize);
	int count2 = (eTerrainSize);

	//정점 목록의 모든 삼각형을 살펴 봅니다.
	for (UINT i = 0; i < count1; i++)
	{
		for (UINT j = 0; j < count2; j++)
		{
			// 삼각형이이 노드 안에 있으면 꼭지점 배열에 추가합니다.
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

	// 이제 노드의 버퍼에 데이터가 저장되므로 꼭지점과 인덱스 배열을 해제합니다.
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
	// 이 노드의 반경을 계산합니다.
	float radius = width / 2.0f;

	// 인덱스를 정점 목록으로 가져옵니다.
	// vertexIndex
	int vertexIndex = index % 2;

	float x1 = 0.0f;
	float z1 = 0.0f;
	float x2 = 0.0f;
	float z2 = 0.0f;
	float x3 = 0.0f;
	float z3 = 0.0f;

	// 정점 목록에서 이 삼각형의 세 꼭지점을 가져옵니다.
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

	// 삼각형의 x 좌표의 최소값이 노드 안에 있는지 확인하십시오.
	float minimumX = min(x1, min(x2, x3));
	if (minimumX > (positionX + radius))
	{
		return false;
	}

	// 삼각형의 x 좌표의 최대 값이 노드 안에 있는지 확인하십시오.
	float maximumX = max(x1, max(x2, x3));
	if (maximumX < (positionX - radius))
	{
		return false;
	}

	// 삼각형의 z 좌표의 최소값이 노드 안에 있는지 확인하십시오.
	float minimumZ = min(z1, min(z2, z3));
	if (minimumZ > (positionZ + radius))
	{
		return false;
	}
	// 삼각형의 z 좌표의 최대 값이 노드 안에 있는지 확인하십시오.
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
	//왼쪽위
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
