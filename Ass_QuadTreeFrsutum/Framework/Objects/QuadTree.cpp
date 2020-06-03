#include "Framework.h"
#include "QuadTree.h"

QuadTree::QuadTree(Shader* shader,Frustum* frustum)
	:Renderer(shader),frustum(frustum)
{
}

QuadTree::~QuadTree()
{
	SafeDelete(parentNode);
}

void QuadTree::Init(Terrain* terrain)
{
	float centerX =  0.0f;
	float centerZ = 0.0f;
	float width = 0.0f;

	Shader* test = shader;

	int vertexCount = terrain->GetVertexCount();

	// 정점리스트의 총 삼각형 수를 저장합니다.
	triangleCount = vertexCount / 3;

	vertexList = new VertexType[vertexCount];

	if (!vertexList)
	{
		return;
	}

	terrain->CopyVertexArray((void*)vertexList);
	CalculateMeshDimensions(vertexCount, centerX, centerZ, width);

	parentNode = new NodeType;
	if (!parentNode)
	{
		return ;
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
void QuadTree::CreateTreeNode(NodeType* node, float positionX, float positionZ, float width, ID3D11Device* device)
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
	
	// 이 노드 안에 있는 삼각형 수를 센다.
	int numTriangles = CountTriangles(positionX, positionZ, width);

	// 사례 1: 이 노드에 삼각형이 없으면 비어있는 상태로 돌아가서 처리할 필요가 없습니다.
	if (numTriangles == 0)
	{
		return;
	}

	// 사례 2: 이 노드에 너무 많은 삼각형이 있는 경우 4 개의 동일한 크기의 더 작은 트리 노드로 분할합니다.
	if (width > 31)
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
				node->transform->Scale(width, width/2, width);
				node->collider = new Collider(node->transform);
				// 이제이 새 자식 노드에서 시작하는 트리를 확장합니다.
				CreateTreeNode(node->Nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f), device);
			}
		}
		return;
	}

	// 사례 3: 이 노드가 비어 있지않고 그 노드의 삼각형 수가 최대 값보다 작으면 
	// 이 노드는 트리의 맨 아래에 있으므로 저장할 삼각형 목록을 만듭니다.
	node->TriangleCount = numTriangles;

	// 정점의 수를 계산합니다.
	int vertexCount = numTriangles * 3;

	// 정점 배열을 만듭니다.
	VertexType* vertices = new VertexType[vertexCount];

	// 인덱스 배열을 만듭니다.
	unsigned long* indices = new unsigned long[vertexCount];

	// 이 새로운 정점 및 인덱스 배열의 인덱스를 초기화합니다.
	int index = 0;
	int vertexIndex = 0;

	// 정점 목록의 모든 삼각형을 살펴 봅니다.
	for (int i = 0; i < triangleCount; i++)
	{
		// 삼각형이이 노드 안에 있으면 꼭지점 배열에 추가합니다.
		if (IsTriangleContained(i, positionX, positionZ, width) == true)
		{
			// 지형 버텍스 목록에 인덱스를 계산합니다.
			vertexIndex = i * 3;

			// 정점 목록에서 이 삼각형의 세 꼭지점을 가져옵니다.
			vertices[index].Position = vertexList[vertexIndex].Position;
			vertices[index].Uv = vertexList[vertexIndex].Uv;
			vertices[index].Normal = vertexList[vertexIndex].Normal;
			indices[index] = index;
			index++;

			vertexIndex++;
			vertices[index].Position = vertexList[vertexIndex].Position;
			vertices[index].Uv = vertexList[vertexIndex].Uv;
			vertices[index].Normal = vertexList[vertexIndex].Normal;
			indices[index] = index;
			index++;

			vertexIndex++;
			vertices[index].Position = vertexList[vertexIndex].Position;
			vertices[index].Uv = vertexList[vertexIndex].Uv;
			vertices[index].Normal = vertexList[vertexIndex].Normal;
			indices[index] = index;
			index++;
		}
	}

	// 정점 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 마침내 정점 버퍼를 만듭니다.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &node->VertexBuffer);

	// 인덱스 버퍼의 설명을 설정합니다.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * vertexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 하위 리소스 구조에 인덱스 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 만듭니다.
	device->CreateBuffer(&indexBufferDesc, &indexData, &node->IndexBuffer);

	// 이제 노드의 버퍼에 데이터가 저장되므로 꼭지점과 인덱스 배열을 해제합니다.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;
}

bool QuadTree::IsTriangleContained(int index, float positionX, float positionZ, float width)
{
	// 이 노드의 반경을 계산합니다.
	float radius = width / 2.0f;

	// 인덱스를 정점 목록으로 가져옵니다.
	int vertexIndex = index * 3;

	// 정점 목록에서 이 삼각형의 세 꼭지점을 가져옵니다.
	float x1 = vertexList[vertexIndex].Position.x;
	float z1 = vertexList[vertexIndex].Position.z;
	vertexIndex++;

	float x2 = vertexList[vertexIndex].Position.x;
	float z2 = vertexList[vertexIndex].Position.z;
	vertexIndex++;

	float x3 = vertexList[vertexIndex].Position.x;
	float z3 = vertexList[vertexIndex].Position.z;

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

void QuadTree::search(NodeType* parent,float count)
{
	if (count > 4) return;

	if (parent->Nodes[0]->Width > 31) 
	{ 
		parent->Nodes[0]->collider->Render(Color(count/3, count / 3, count / 3, 1));
		search(parent->Nodes[0],count +1);
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

void QuadTree::RenderNode(NodeType* node)
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
	if (result[0])
	{
		int a;
	}
	if (result[1])
	{
		int a;
	}
	if (result[2])
	{
		int a;
	}
	if (result[3])
	{
		int a;
	}
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



	if (cnt != 0)
	{
		return;
	}
	// If there were any children nodes then there is no need to continue as parent nodes won't contain any triangles to render.
	

	// Otherwise if this node can be seen and has triangles in it then render these triangles.

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	D3D::GetDC()->IASetVertexBuffers(0, 1, &node->VertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	D3D::GetDC()->IASetIndexBuffer(node->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Determine the number of indices in this node.
	indexCount = node->TriangleCount * 3;

	// Call the terrain shader to render the polygons in this node.
	shader->DrawIndexed(0,1,indexCount);

	// Increase the count of the number of polygons that have been rendered during this frame.
	drawCount += node->TriangleCount;

	return;
}
