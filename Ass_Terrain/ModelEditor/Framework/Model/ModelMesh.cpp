#include "Framework.h"
#include "ModelMesh.h"

ModelBone::ModelBone()
{
}

ModelBone::~ModelBone()
{
}

///////////////////////////////////////////////////////////////////////////////

ModelMesh::ModelMesh()
{
	boneBuffer = new ConstantBuffer(&boneDesc, sizeof(BoneDesc));
}

ModelMesh::~ModelMesh()
{	
	SafeDelete(perFrame);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(vertexBuffer);
	SafeDelete(indexBuffer);

	SafeDelete(boneBuffer);
	SafeDelete(material);
}

void ModelMesh::Binding(Model * model)
{
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Model::ModelVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);

	Material* srcMaterial = model->MaterialByName(materialName);

	material = new Material();
	material->Ambient(srcMaterial->Ambient());
	material->Diffuse(srcMaterial->Diffuse());
	material->Specular(srcMaterial->Specular());
	material->Emissive(srcMaterial->Emissive());

	if (srcMaterial->DiffuseMap() != NULL)
		material->DiffuseMap(srcMaterial->DiffuseMap()->GetFile());

	if (srcMaterial->SpecularMap() != NULL)
		material->SpecularMap(srcMaterial->SpecularMap()->GetFile());

	if (srcMaterial->NormalMap() != NULL)
		material->NormalMap(srcMaterial->NormalMap()->GetFile());
}

void ModelMesh::SetShader(Shader * shader)
{
	this->shader = shader;
	
	SafeDelete(perFrame);
	perFrame = new PerFrame(shader);

	material->SetShader(shader);
	sBoneBuffer = shader->AsConstantBuffer("CB_Bone");

	sTransformsSRV = shader->AsSRV("TransformsMap");
}

void ModelMesh::Update()
{
	boneDesc.Index = boneIndex;
	perFrame->Update();	
}

void ModelMesh::Render(UINT drawCount)
{
	boneBuffer->Apply();
	sBoneBuffer->SetConstantBuffer(boneBuffer->Buffer());

	perFrame->Render();	
	material->Render();

	if (transformsSRV != NULL)
		sTransformsSRV->SetResource(transformsSRV);

	vertexBuffer->Render();
	indexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->DrawIndexedInstanced(0, pass, indexCount, drawCount);
}

void ModelMesh::TransformsSRV(ID3D11ShaderResourceView * srv)
{
	transformsSRV = srv;
}
