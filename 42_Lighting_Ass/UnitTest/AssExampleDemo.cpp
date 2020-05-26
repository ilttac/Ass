#include "stdafx.h"
#include "AssExampleDemo.h"
#include "Viewer/Freedom.h"
#include "Environment/Terrain.h"
#include "Environment/SkyCube.h"


void AssExampleDemo::Initialize()
{
	Context::Get()->GetCamera()->Position(0, 32, -67);
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	dynamic_cast<Freedom*>(Context::Get()->GetCamera())->Speed(50, 5);
	srand((UINT)time(NULL));

	cameraState = eCameraIdle;
	shader = new Shader(L"33_Animation.fx");
	sky = new SkyCube(L"Environment/GrassCube1024.dds");

	bPatrolState = false;
	bSearchState = false;
	bAttakRangeState = false;
	ColliderRenderSwitchState = false;

	bArcherPatrolState = false;
	bArcherSearchState = false;
	bArcherAttakRangeState = false;

	bWeaponcolliderState = false;
	bPlayerAttackState = false;

	weaponCount = 0;

	Mesh();
	Michelle();
	Hallin();
	Archer();
}

void AssExampleDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(sky);

	SafeDelete(weapon);
	for (int i = 0; i < arrowCount; i++)
	{
		SafeDeleteArray(archerWeapon[i]);
	}
	for (int i = 0; i < arrowCount; i++)
	{
		SafeDeleteArray(archerWeaponTransform[i]);
	}

}

void AssExampleDemo::Update()
{
	sky->Update();
	grid->Update();
	ImGui::Checkbox("Collider", &ColliderRenderSwitchState);

	if (4 == michelle->CurrClipNumber() && bWeaponcolliderState)
	{
		hallin->PlayClip(0, 3, 2.0f, 6.0f);
	}
	else
	{
		if (true == bSearchState && false == bAttakRangeState)
		{
			PlayerTracking(hallin, michelle);
		}
		if (false == bSearchState && false == bAttakRangeState)
		{
			MonsterPatrol(hallin, bPatrolState, ranX[0], ranZ[0]);
		}
		if (true == bArcherSearchState && false == bArcherAttakRangeState)
		{
			PlayerTracking(archer, michelle);
		}
		if (false == bArcherSearchState && false == bArcherAttakRangeState)
		{
			MonsterPatrol(archer, bArcherPatrolState, ranX[1], ranZ[1]);
		}
		if (bAttakRangeState) MonsterAttack(hallin, michelle, "hallin");
		if (bArcherAttakRangeState) MonsterAttack(archer, michelle, "archer");
	}

	if (michelle != NULL)
	{
		PlayerMove();
		if (Keyboard::Get()->Press('K'))
		{
			michelle->PlayClip(0, 4, 2.0f, 1.0f);
		}
		if (!Keyboard::Get()->Press(VK_LEFT) &&
			!Keyboard::Get()->Press(VK_UP) &&
			!Keyboard::Get()->Press(VK_RIGHT) &&
			!Keyboard::Get()->Press(VK_DOWN) &&
			!Keyboard::Get()->Down(VK_SPACE) &&
			!Keyboard::Get()->Press('K'))
		{
			michelle->PlayClip(0, 0, 2.0f, 1.0f);
		}
		PlayerWeaponChange();

		michelle->UpdateTransforms();
		michelle->Pass(2);
		michelle->Update();

		Matrix attach = michelle->GetTransform(0)->World();
		collider[0].Collider->GetTransform()->World(attach);
		collider[0].Collider->Update();

		Matrix attach2 = michelle->GetAttachTransform(0);
		weaponCollider[0].Collider->GetTransform()->World(attach2);
		weaponCollider[0].Collider->Update();

		playerObbCollider[0].Collider->GetTransform()->World(attach);
		playerObbCollider[0].Collider->Update();
	}
	if (hallin != NULL)
	{

		Matrix attach = hallin->GetTransform(0)->World();

		//Collider
		mcollider[0].Collider->GetTransform()->World(attach);
		mcollider[0].Collider->Update();

		mAtkcollider[0].Collider->GetTransform()->World(attach);
		mAtkcollider[0].Collider->Update();

		hallinObbCollider[0].Collider->GetTransform()->World(attach);
		hallinObbCollider[0].Collider->Update();
		hallin->UpdateTransforms();
		//hallin->Pass(2);
		hallin->Update();

	}
	if (archer != NULL)
	{
		Matrix attach = archer->GetTransform(0)->World();

		//Collider
		archerSerachCollider[0].Collider->GetTransform()->World(attach);
		archerSerachCollider[0].Collider->Update();

		archerAtkCollider[0].Collider->GetTransform()->World(attach);
		archerAtkCollider[0].Collider->Update();

		archerObbColider[0].Collider->GetTransform()->World(attach);
		archerObbColider[0].Collider->Update();

		archer->UpdateTransforms();
		archer->Update();
	}
}

void AssExampleDemo::Render()
{
	sky->Render();
	Pass(0, 1, 2);
	floor->Render();
	grid->Render();
	for (int i = 0; i < 100; i++)
	{
		if (archerWeapon[i] != NULL)
		{
			archerWeapon[i]->Render();
		}
		else
		{
			break;
		}
	}
	if (michelle != NULL)michelle->Render();
	if (hallin != NULL)hallin->Render();
	if (archer != NULL)archer->Render();

	//if (4 == michelle->CurrClipNumber())
	//{
	//	float x = Context::Get()->GetCamera()->GetPositionX();
	//	float y = Context::Get()->GetCamera()->GetPositionY();
	//	float z = Context::Get()->GetCamera()->GetPositionZ();

	//	switch (cameraState)
	//	{
	//	case eCameraIdle:
	//		Context::Get()->GetCamera()->Position(x - 2, y, z);
	//		cameraState = eCameraLeft;
	//		break;
	//	case eCameraLeft:
	//		Context::Get()->GetCamera()->Position(x + 2, y + 2, z);
	//		cameraState = eCameraTop;
	//		break;
	//	case eCameraTop:
	//		Context::Get()->GetCamera()->Position(x + 2, y - 2, z);
	//		cameraState = eCameraRight;
	//		break;
	//	case eCameraRight:
	//		Context::Get()->GetCamera()->Position(x - 2, y - 2, z);
	//		cameraState = eCameraBottom;
	//		break;
	//	case eCameraBottom:
	//		Context::Get()->GetCamera()->Position(x, y + 2, z);
	//		cameraState = eCameraIdle;
	//		break;
	//	default:
	//		break;
	//	}
	//}
	//collider
	bSearchState = collider[0].Collider->IsIntersect(mcollider[0].Collider);
	bAttakRangeState = collider[0].Collider->IsIntersect(mAtkcollider[0].Collider);

	bArcherSearchState = collider[0].Collider->IsIntersect(archerSerachCollider[0].Collider);
	bArcherAttakRangeState = collider[0].Collider->IsIntersect(archerAtkCollider[0].Collider);



	bWeaponcolliderState = weaponCollider[0].Collider->IsIntersect(hallinObbCollider[0].Collider);
	bWeaponArcherObbState = weaponCollider[0].Collider->IsIntersect(archerObbColider[0].Collider);

	if (ColliderRenderSwitchState)
	{

		collider[0].Collider->Render(bSearchState || bAttakRangeState ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
		mcollider[0].Collider->Render(bSearchState ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
		mAtkcollider[0].Collider->Render(bAttakRangeState ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));

		if (!bWeaponcolliderState)
		{
			weaponCollider[0].Collider->Render(Color(0, 1, 0, 1));
			hallinObbCollider[0].Collider->Render(Color(0, 1, 0, 1));
		}
		else
		{
			weaponCollider[0].Collider->Render(Color(1, 0, 0, 1));
			hallinObbCollider[0].Collider->Render(Color(1, 0, 0, 1));
		}
		if (!bWeaponArcherObbState)
		{
			weaponCollider[0].Collider->Render(Color(0, 1, 0, 1));
			archerObbColider[0].Collider->Render(Color(0, 1, 0, 1));
		}
		else
		{
			weaponCollider[0].Collider->Render(Color(1, 0, 0, 1));
			archerObbColider[0].Collider->Render(Color(1, 0, 0, 1));
		}

		playerObbCollider[0].Collider->Render(Color(0, 1, 0, 1));
		archerSerachCollider[0].Collider->Render(bArcherSearchState ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
		archerAtkCollider[0].Collider->Render(bArcherAttakRangeState ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
		archerObbColider[0].Collider->Render(Color(0, 1, 0, 1));
		for (int i = 0; i < 100; i++)
		{
			if (archerArrowCollider[i].Collider != NULL)archerArrowCollider[i].Collider->Render(Color(0, 1, 0, 0));
		}
	}
}

void AssExampleDemo::Mesh()
{

	//Create Material
	{
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
	}
	//Create Mesh
	{
		Transform* transform = NULL;

		grid = new MeshRender(shader, new MeshGrid(5, 5));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(20, 10, 20);

		grid->UpdateTransforms();

		meshes.push_back(grid);
	}
}
void AssExampleDemo::Michelle()
{
	michelle = new ModelAnimator(shader);
	michelle->ReadMaterial(L"Michelle/Mesh");
	michelle->ReadMesh(L"Michelle/Mesh");
	michelle->ReadClip(L"Michelle/Idle");//0
	michelle->ReadClip(L"Michelle/Running"); //1
	michelle->ReadClip(L"Michelle/Jab_Elbow_Punch");//2
	michelle->ReadClip(L"Michelle/Hit_Reaction");//3
	michelle->ReadClip(L"Michelle/Sword_Default");//4
	michelle->ReadClip(L"Michelle/Sword_Skill");//5

	weapon = new Model();
	weapon->ReadMaterial(L"Weapon/Sword");
	weapon->ReadMesh(L"Weapon/Sword");
	//D3DX11SaveTextureToFile
	//D3DX11LoadTextureFromTexture
	Transform attachTransform;
	attachTransform.Position(0, 0, 0);
	attachTransform.RotationDegree(0, -90, 0);
	attachTransform.Scale(0.5f, 0.5f, 0.5f);

	michelle->GetModel()->Attach(shader, weapon, 121, &attachTransform);

	Transform* transform = NULL;

	transform = michelle->AddTransform();
	transform->Position(40, 0, -35);
	transform->Scale(0.075f, 0.075f, 0.075f);
	michelle->PlayClip(0, 0, 1.0f);

	michelle->UpdateTransforms();

	//Collider init
	{
		collider[0].Init = new Transform();
		collider[0].Init->Scale(150, 150, 150);
		collider[0].Transform = new Transform();
		collider[0].Collider = new SquareCollider(collider[0].Transform, collider[0].Init);

		weaponCollider[0].Init = new Transform();

		weaponCollider[0].Init->Scale(10, 10, 50);
		weaponCollider[0].Init->Position(30, 0, 0);
		weaponCollider[0].Init->RotationDegree(0, 90, 0);

		weaponCollider[0].Transform = new Transform();
		weaponCollider[0].Collider = new Collider(weaponCollider[0].Transform, weaponCollider[0].Init);

		playerObbCollider[0].Init = new Transform();
		playerObbCollider[0].Init->Position(0, 100, 0);
		playerObbCollider[0].Init->Scale(100, 200, 100);

		playerObbCollider[0].Transform = new Transform();
		playerObbCollider[0].Collider = new Collider(playerObbCollider[0].Transform, playerObbCollider[0].Init);
	}
	michelle->Pass(2);
}
void AssExampleDemo::Hallin()
{
	hallin = new ModelAnimator(shader);
	hallin->ReadMaterial(L"Hallin/Mesh");
	hallin->ReadMesh(L"Hallin/Mesh");
	hallin->ReadClip(L"Hallin/Zombie_Idle");
	hallin->ReadClip(L"Hallin/Walking");
	hallin->ReadClip(L"Hallin/Zombie_Attack");
	hallin->ReadClip(L"Hallin/Hit_Reaction");

	Transform* transform = NULL;

	transform = hallin->AddTransform();
	transform->Position(-25, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	hallin->PlayClip(0, 0, 1.0, 8.0f);

	hallin->UpdateTransforms();


	//Collider init
	{
		mcollider[0].Init = new Transform();
		mcollider[0].Init->Scale(400, 400, 400);

		//mcollider[0].Init->Position(0, 0, 0);

		mcollider[0].Transform = new Transform();
		mcollider[0].Collider = new SquareCollider(mcollider[0].Transform, mcollider[0].Init);

		mAtkcollider[0].Init = new Transform();
		mAtkcollider[0].Init->Scale(200, 200, 200);


		mAtkcollider[0].Transform = new Transform();
		mAtkcollider[0].Collider = new SquareCollider(mAtkcollider[0].Transform, mAtkcollider[0].Init);

		hallinObbCollider[0].Init = new Transform();
		hallinObbCollider[0].Init->Position(0, 100, 0);
		hallinObbCollider[0].Init->Scale(100, 200, 100);

		hallinObbCollider[0].Transform = new Transform();
		hallinObbCollider[0].Collider = new Collider(hallinObbCollider[0].Transform, hallinObbCollider[0].Init);
	}

	hallin->Pass(2);
}

void AssExampleDemo::Archer()
{
	archer = new ModelAnimator(shader);
	archer->ReadMaterial(L"Archer/Mesh");
	archer->ReadMesh(L"Archer/Mesh");
	archer->ReadClip(L"Archer/Archer_Idle");//0
	archer->ReadClip(L"Archer/Archer_Run");//1
	archer->ReadClip(L"Archer/Archer_Attak");//2
	archer->ReadClip(L"Archer/Archer_Dead");//3

	for (int i = 0; i < 100; i++)
	{
		archerWeapon[i] = new ModelRender(shader);
		archerWeapon[i]->ReadMaterial(L"Weapon/LongArrow");
		archerWeapon[i]->ReadMesh(L"Weapon/LongArrow");

		archerWeapon[i]->Pass(1);
	}


	Transform* transform = NULL;

	transform = archer->AddTransform();
	transform->Position(-5, 0, -10);
	transform->Scale(0.075f, 0.075f, 0.075f);

	//archerWeaponTransform->Position(archer->GetTransform(0)->GetPositon().x, 5, archer->GetTransform(0)->GetPositon().z);
	//archerWeaponTransform->Scale(3.5f, 3.5f, 3.5f);
	//archerWeapon[0]->UpdateTransforms();
	Matrix S;
	Matrix R;
	D3DXMatrixIdentity(&S);
	D3DXMatrixIdentity(&R);
	archer->PlayClip(0, 2, 1.0, 8.0f);
	*archer->GetModel()->Bones()[70]->Parent() = *(archer->GetModel()->Bones()[37]);	//arrow 70

	D3DXMatrixTranslation(&S, 0, 0, -30);
	D3DXMatrixRotationY(&R, Math::ToRadian(90));
	archer->GetModel()->Bones()[70]->Transform() *= S * R;
	archer->UpdateTransforms();

	//righthand 37

	//Collider init
	{
		archerSerachCollider[0].Init = new Transform();
		archerSerachCollider[0].Init->Scale(1000, 1000, 1000);

		//mcollider[0].Init->Position(0, 0, 0);

		archerSerachCollider[0].Transform = new Transform();
		archerSerachCollider[0].Collider = new SquareCollider(archerSerachCollider[0].Transform, archerSerachCollider[0].Init);

		archerAtkCollider[0].Init = new Transform();
		archerAtkCollider[0].Init->Scale(800, 800, 800);


		archerAtkCollider[0].Transform = new Transform();
		archerAtkCollider[0].Collider = new SquareCollider(archerAtkCollider[0].Transform, archerAtkCollider[0].Init);

		archerObbColider[0].Init = new Transform();
		archerObbColider[0].Init->Position(0, 100, 0);
		archerObbColider[0].Init->Scale(100, 200, 100);

		archerObbColider[0].Transform = new Transform();
		archerObbColider[0].Collider = new Collider(archerObbColider[0].Transform, archerObbColider[0].Init);
	}
	archer->Pass(2);
}

void AssExampleDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (ModelRender* temp : models)
		temp->Pass(model);

	for (ModelAnimator* temp : animators)
		temp->Pass(anim);
}

void AssExampleDemo::PlayerTracking(ModelAnimator * monster, ModelAnimator * player)
{
	Vector3 mPos = monster->GetTransform(0)->GetPositon();
	Vector3 pPos = player->GetTransform(0)->GetPositon();
	RotateTowards(monster, pPos);

	Vector3 nor = Vector3(0, 0, 0);
	D3DXVec3Normalize(&nor, &(pPos - mPos));

	float runSpeed = 5.0f;

	Vector3 origin = monster->GetTransform(0)->GetPositon();
	origin.x += runSpeed * nor.x * Time::Delta();
	origin.z += runSpeed * nor.z * Time::Delta();
	monster->PlayClip(0, 1, 1.0f, 4.0f);

	if (D3DXVec3Length(&(pPos - origin)) < 1.0f)
	{
		return;
	}
	monster->GetTransform(0)->Position(origin.x, 0, origin.z);
}

void AssExampleDemo::MonsterPatrol(ModelAnimator * monster, bool& mbPatrolState, float& ranX, float& ranZ)
{
	//순찰 종료 조건
	//1. 플레이어가 search range에 있을때. --PlayerTracking의 반환값으로 판별
	//2. 해당 정점에 도착했을때. -- patrol 함수내에서 판별
	Vector3 mPos = monster->GetTransform(0)->GetPositon();

	float runSpeed = 10.0f;

	if (mbPatrolState == false)
	{
		ranX = rand() % 40 - 20;
		ranZ = rand() % 40 - 20;
		mbPatrolState = true;
	}

	RotateTowards(monster, Vector3(ranX, 0, ranZ));

	Vector3 nor = Vector3(0, 0, 0);
	D3DXVec3Normalize(&nor, &(Vector3(ranX, 0, ranZ) - mPos));

	Vector3 origin = monster->GetTransform(0)->GetPositon();
	origin.x += runSpeed * nor.x * Time::Delta();
	origin.z += runSpeed * nor.z * Time::Delta();

	monster->GetTransform(0)->Position(origin.x, 0, origin.z);
	monster->PlayClip(0, 1, 1.0f, 4.0f);


	if (D3DXVec3Length(&(Vector3(ranX, 0, ranZ) - origin)) < 1.0f)
	{
		mbPatrolState = false;
	}

}

void AssExampleDemo::RotateTowards(ModelAnimator * modelAnim, Vector3 targetPos)
{
	Vector3 mPos = modelAnim->GetTransform(0)->GetPositon();
	Vector3 nor = Vector3(0, 0, 0);

	D3DXVec3Normalize(&nor, &(targetPos - mPos));
	Vector3 front = Vector3(0, 0, -1);

	float radian = acos(D3DXVec3Dot(&front, &nor));
	Vector3  right;
	D3DXVec3Cross(&right, &front, &D3DXVECTOR3(0.0f, 1.0f, 0.0f));

	if (D3DXVec3Dot(&right, &nor) > 0)
	{
		modelAnim->GetTransform(0)->Rotation(0, -radian, 0);
	}
	else
	{
		modelAnim->GetTransform(0)->Rotation(0, +radian, 0);
	}
}

void AssExampleDemo::MonsterAttack(ModelAnimator * monster, ModelAnimator * player, string monsterName)
{
	Vector3 pPos = player->GetTransform(0)->GetPositon();
	Matrix transform;

	if (monsterName == "archer")
	{
		//archerWeaponTransform = archerWeapon[0]->AddTransform();
		if (arrowdelayTime > 2.0f)
		{
			arrowdelayTime = 0.0f;
			for (int i = 0; i < 100; i++)
			{
				if (archerWeaponTransform[i] == NULL)
				{
					archerWeaponTransform[i] = new Transform();
					archerWeaponTransform[i] = archerWeapon[i]->AddTransform();
					archerWeaponTransform[i]->Scale(0.35f, 0.35f, 0.35f);
					archerWeaponTransform[i]->Rotation(Math::ToRadian(90), archer->GetTransform(0)->GetRotation().y, 0);
					archerWeaponTransform[i]->Position(archer->GetTransform(0)->GetPositon().x, 10, archer->GetTransform(0)->GetPositon().z);
					D3DXVec3Normalize(&arrowNorMal[i], &(pPos - archerWeaponTransform[i]->GetPositon()));

					archerArrowCollider[i].Init = new Transform();
					archerArrowCollider[i].Init->Position(0, 0, 1);
					archerArrowCollider[i].Init->Scale(1, 10, 1);

					archerArrowCollider[i].Transform = new Transform();
					archerArrowCollider[i].Collider = new Collider(archerArrowCollider[i].Transform, archerArrowCollider[i].Init);
					archerWeaponTransform[i]->Update();
					archerWeapon[i]->Update();
					archerWeapon[i]->UpdateTransforms();
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			arrowdelayTime += 1.0f * Time::Delta();
			for (int i = 0; i < 100; i++)
			{
				if (archerWeaponTransform[i] != NULL)
				{
					Matrix attach = archerWeaponTransform[i]->World();

					Vector3 origin = archerWeaponTransform[i]->GetPositon();
					float speed = 0.05f;
					origin.x += arrowNorMal[i].x * speed;
					origin.z += arrowNorMal[i].z * speed;
					archerWeaponTransform[i]->Position(origin.x, 10, origin.z);


					archerArrowCollider[i].Collider->GetTransform()->World(attach);
					archerArrowCollider[i].Collider->Update();
					archerWeaponTransform[i]->Update();
					archerWeapon[i]->Update();
					archerWeapon[i]->UpdateTransforms();
				}
				else
				{
					break;
				}
				
			}
			


		}
		
		
	}

	else if (monsterName == "hallin")
	{

	}
	RotateTowards(monster, pPos);
	monster->PlayClip(0, 2, 3.0f, 3.0f);
	monster->UpdateTransforms();

}

void AssExampleDemo::PlayerMove()
{
	Vector3 origin = michelle->GetTransform(0)->GetPositon();

	if (Keyboard::Get()->Press(VK_UP))
	{
		if (michelle->CurrClipNumber() != 1)
		{
			michelle->PlayClip(0, 1, 3.0f, 3.0f);
		}
		origin.z += 12.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(180.f);
		michelle->GetTransform(0)->Position(origin);
	}
	else if (Keyboard::Get()->Press(VK_RIGHT))
	{
		if (michelle->CurrClipNumber() != 1)
		{
			michelle->PlayClip(0, 1, 3.0f, 3.0f);
		}
		origin.x += 12.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(-90.f);
		michelle->GetTransform(0)->Position(origin);
	}
	else if (Keyboard::Get()->Press(VK_LEFT))
	{
		if (michelle->CurrClipNumber() != 1)
		{
			michelle->PlayClip(0, 1, 3.0f, 3.0f);
		}
		origin.x -= 12.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(90.0f);
		michelle->GetTransform(0)->Position(origin);
	}
	else if (Keyboard::Get()->Press(VK_DOWN))
	{
		if (michelle->CurrClipNumber() != 1)
		{
			michelle->PlayClip(0, 1, 3.0f, 3.0f);
		}
		origin.z -= 12.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(0.0f);
		michelle->GetTransform(0)->Position(origin);
	}
}

void AssExampleDemo::PlayerWeaponChange()
{
	if (Keyboard::Get()->Down(VK_SPACE))
	{
		if (weapon != NULL)
		{
			for (UINT i = 0; i < weapon->BoneCount(); i++)
			{
				michelle->GetModel()->Bones().pop_back();
			}
			michelle->GetModel()->Materials().pop_back();
			michelle->GetModel()->Meshes().pop_back();
			michelle->GetModel()->BoneByIndex(121)->Childs().pop_back();
			michelle->GetModel()->BoneByIndex(121)->Childs().pop_back();

			SafeDelete(weapon);

			if ((weaponCount % 3) == 0)
			{
				weapon = new Model();
				weapon->ReadMaterial(L"Weapon/Dagger_epic");
				weapon->ReadMesh(L"Weapon/Dagger_epic");
				weaponCount++;
			}
			else if ((weaponCount % 3) == 1)
			{
				weaponCount++;
			}


			if ((weaponCount % 3) != 2)
			{
				Transform attachTransform;
				attachTransform.Position(0, 0, 0);
				attachTransform.RotationDegree(0, -90, 0);
				attachTransform.Scale(0.5f, 0.5f, 0.5f);
				michelle->GetModel()->Attach(shader, weapon, 121, &attachTransform);
				michelle->CreateTexture();
			}

		}
		else
		{
			weapon = new Model();
			weapon->ReadMaterial(L"Weapon/Sword");
			weapon->ReadMesh(L"Weapon/Sword");

			Transform attachTransform;
			attachTransform.Position(0, 0, 0);
			attachTransform.RotationDegree(0, -90, 0);
			attachTransform.Scale(0.5f, 0.5f, 0.5f);

			michelle->GetModel()->Attach(shader, weapon, 121, &attachTransform);
			michelle->CreateTexture();
			weaponCount++;
		}
	}
}


