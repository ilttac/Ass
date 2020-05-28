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

	ColliderRenderSwitchState = false;

	bPlayerAttackState = false;
	bPlayerHitReaction = false;

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
	SafeDelete(grid);
	SafeDelete(floor);

	SafeDelete(weapon);
	SafeDelete(archerWeapon);
	SafeDelete(michelle);
	SafeDelete(archer);
	SafeDelete(hallin);
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


	for (int i = 0; i < eMonsterMaxNum; i++)
	{
		if (archer->CurrClipNumber(i) == 3)
		{
			continue;
		}
		if (4 == michelle->CurrClipNumber(0) && bWeaponcolliderState[i])
		{
			hallin->PlayClip(i, 3, 1.0f, 1.0f);
		}
		if (4 == michelle->CurrClipNumber(0) && bWeaponArcherObbState[i])
		{
			archer->PlayClip(i, 3, 1.0f, 1.0f);
		}


		if (true == bSearchState[i] && false == bAttakRangeState[i]) // 수정필요
		{
			PlayerTracking(hallin, michelle, i);
		}
		else if (false == bSearchState[i] && false == bAttakRangeState[i])
		{
			MonsterPatrol(hallin, bPatrolState[i], ranHallinX, ranHallinZ, i);
		}

		if (true == bArcherSearchState[i] && false == bArcherAttakRangeState[i]) //수정필요
		{
			PlayerTracking(archer, michelle, i);
		}
		else if (false == bArcherSearchState[i] && false == bArcherAttakRangeState[i])
		{
			MonsterPatrol(archer, bArcherPatrolState[i], ranArcherX, ranArcherZ, i);
		}
		if (bAttakRangeState[i]) MonsterAttack(hallin, michelle, "hallin", i);
		if (bArcherAttakRangeState[i]) MonsterAttack(archer, michelle, "archer", i);
	}


	if (michelle != NULL)
	{
		PlayerMove();
		
		PlayerWeaponChange();

		michelle->UpdateTransforms();

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
		//Collider
		for (int i = 0; i < eMonsterMaxNum; i++)
		{
			Matrix attach = hallin->GetTransform(i)->World();
			mcollider[i].Collider->GetTransform()->World(attach);
			mcollider[i].Collider->Update();

			mAtkcollider[i].Collider->GetTransform()->World(attach);
			mAtkcollider[i].Collider->Update();

			hallinObbCollider[i].Collider->GetTransform()->World(attach);
			hallinObbCollider[i].Collider->Update();
		}
		hallin->UpdateTransforms();
		//hallin->Pass(2);
		hallin->Update();

	}
	if (archer != NULL)
	{
		for (int i = 0; i < eMonsterMaxNum; i++)
		{
			Matrix attach = archer->GetTransform(i)->World();

			//Collider
			archerSerachCollider[i].Collider->GetTransform()->World(attach);
			archerSerachCollider[i].Collider->Update();

			archerAtkCollider[i].Collider->GetTransform()->World(attach);
			archerAtkCollider[i].Collider->Update();

			archerObbColider[i].Collider->GetTransform()->World(attach);
			archerObbColider[i].Collider->Update();
			arrowdelayTime[i] += 1.0f * Time::Delta();
		}
		ArrowUpdate();

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
	if (archerWeapon != NULL)
	{
		archerWeapon->Render();
		//archerWeapon->Pass(1);
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
	for (int i = 0; i < eMonsterMaxNum; i++)
	{
		bSearchState[i] = collider[0].Collider->IsIntersect(mcollider[i].Collider);
		bAttakRangeState[i] = collider[0].Collider->IsIntersect(mAtkcollider[i].Collider);

		bArcherSearchState[i] = collider[0].Collider->IsIntersect(archerSerachCollider[i].Collider);
		bArcherAttakRangeState[i] = collider[0].Collider->IsIntersect(archerAtkCollider[i].Collider);

		bWeaponcolliderState[i] = weaponCollider[0].Collider->IsIntersect(hallinObbCollider[i].Collider);
		bWeaponArcherObbState[i] = weaponCollider[0].Collider->IsIntersect(archerObbColider[i].Collider);

	}
	for (int i = 0; i < 100; i++)
	{
		if (archerArrowCollider[i].Collider != NULL)
		{
			bPlayerHitReaction = archerArrowCollider[i].Collider->IsIntersect(playerObbCollider[0].Collider);
			if (bPlayerHitReaction == true)
			{
				break;
			}
		}
	}
	if (ColliderRenderSwitchState)
	{
		for (int i = 0; i < eMonsterMaxNum; i++)
		{
			collider[0].Collider->Render(bSearchState[i] || bAttakRangeState[i] ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
			mcollider[i].Collider->Render(bSearchState[i] ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
			mAtkcollider[i].Collider->Render(bAttakRangeState[i] ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));

			if (!bWeaponcolliderState[i])
			{
				weaponCollider[0].Collider->Render(Color(0, 1, 0, 1));
				hallinObbCollider[i].Collider->Render(Color(0, 1, 0, 1));
			}
			else
			{
				weaponCollider[0].Collider->Render(Color(1, 0, 0, 1));
				hallinObbCollider[i].Collider->Render(Color(1, 0, 0, 1));
			}
			if (!bWeaponArcherObbState[i])
			{
				weaponCollider[0].Collider->Render(Color(0, 1, 0, 1));
				archerObbColider[i].Collider->Render(Color(0, 1, 0, 1));
			}
			else
			{
				weaponCollider[0].Collider->Render(Color(1, 0, 0, 1));
				archerObbColider[i].Collider->Render(Color(1, 0, 0, 1));
			}
			archerSerachCollider[i].Collider->Render(bArcherSearchState[i] ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
			archerAtkCollider[i].Collider->Render(bArcherAttakRangeState[i] ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
			archerObbColider[i].Collider->Render(Color(0, 1, 0, 1));
		}


		playerObbCollider[0].Collider->Render(Color(0, 1, 0, 1));

		for (int i = 0; i < 100; i++)
		{
			if (archerArrowCollider[i].Collider != NULL)archerArrowCollider[i].Collider->Render(bPlayerHitReaction ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
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
	michelle->PlayClip(0, 0,0.5f,0.5f);

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

	for (float x = 0; x < 20; x += 4.0f)
	{
		for (float z = 0; z < 20; z += 10.0f)
		{
			Transform* transform = NULL;

			transform = hallin->AddTransform();
			transform->Position(-25 + x, 0, -30 + z);
			transform->Scale(0.075f, 0.075f, 0.075f);
		}

	}

	for (int i = 0; i < eMonsterMaxNum; i++)
	{
		hallin->PlayClip(i, 0, 0.5f, 0.5f);
	}


	hallin->UpdateTransforms();


	//Collider init
	for (int i = 0; i < eMonsterMaxNum; i++)
	{
		mcollider[i].Init = new Transform();
		mcollider[i].Init->Scale(400, 400, 400);

		//mcollider[i].Init->Position(0, 0, 0);

		mcollider[i].Transform = new Transform();
		mcollider[i].Collider = new SquareCollider(mcollider[i].Transform, mcollider[i].Init);

		mAtkcollider[i].Init = new Transform();
		mAtkcollider[i].Init->Scale(200, 200, 200);


		mAtkcollider[i].Transform = new Transform();
		mAtkcollider[i].Collider = new SquareCollider(mAtkcollider[i].Transform, mAtkcollider[i].Init);

		hallinObbCollider[i].Init = new Transform();
		hallinObbCollider[i].Init->Position(0, 100, 0);
		hallinObbCollider[i].Init->Scale(100, 200, 100);

		hallinObbCollider[i].Transform = new Transform();
		hallinObbCollider[i].Collider = new Collider(hallinObbCollider[i].Transform, hallinObbCollider[i].Init);
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

	archerWeapon = new ModelRender(shader);
	archerWeapon->ReadMaterial(L"Weapon/LongArrow");
	archerWeapon->ReadMesh(L"Weapon/LongArrow");

	archerWeapon->Pass(1);

	for (float x = 0; x < 20; x += 4.0f)
	{
		for (float z = 0; z < 20; z += 10.0f)
		{
			Transform* transform = NULL;

			transform = archer->AddTransform();
			transform->Position(-5 + x, 0, -10 + z);
			transform->Scale(0.075f, 0.075f, 0.075f);
		}

	}

	//archerWeaponTransform->Position(archer->GetTransform(0)->GetPositon().x, 5, archer->GetTransform(0)->GetPositon().z);
	//archerWeaponTransform->Scale(3.5f, 3.5f, 3.5f);
	//archerWeapon[0]->UpdateTransforms();
	Matrix S;
	Matrix R;
	D3DXMatrixIdentity(&S);
	D3DXMatrixIdentity(&R);
	*archer->GetModel()->Bones()[70]->Parent() = *(archer->GetModel()->Bones()[37]);	//arrow 70

	D3DXMatrixTranslation(&S, 0, 0, -30);
	D3DXMatrixRotationY(&R, Math::ToRadian(90));
	archer->GetModel()->Bones()[70]->Transform() *= S * R;
	archer->UpdateTransforms();

	//righthand 37

	//Collider init
	for (int i = 0; i < eMonsterMaxNum; i++)
	{
		archerSerachCollider[i].Init = new Transform();
		archerSerachCollider[i].Init->Scale(1000, 1000, 1000);

		//mcollider[i].Init->Position(0, 0, 0);

		archerSerachCollider[i].Transform = new Transform();
		archerSerachCollider[i].Collider = new SquareCollider(archerSerachCollider[i].Transform, archerSerachCollider[i].Init);

		archerAtkCollider[i].Init = new Transform();
		archerAtkCollider[i].Init->Scale(800, 800, 800);


		archerAtkCollider[i].Transform = new Transform();
		archerAtkCollider[i].Collider = new SquareCollider(archerAtkCollider[i].Transform, archerAtkCollider[i].Init);

		archerObbColider[i].Init = new Transform();
		archerObbColider[i].Init->Position(0, 100, 0);
		archerObbColider[i].Init->Scale(100, 200, 100);

		archerObbColider[i].Transform = new Transform();
		archerObbColider[i].Collider = new Collider(archerObbColider[i].Transform, archerObbColider[i].Init);
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

void AssExampleDemo::PlayerTracking(ModelAnimator * monster, ModelAnimator * player, UINT monIndex)
{
	Vector3 mPos = monster->GetTransform(monIndex)->GetPositon();
	Vector3 pPos = player->GetTransform(0)->GetPositon();
	if (monster->CurrClipNumber(monIndex) != 3)
	{
		RotateTowards(monster, pPos, monIndex);
	}

	Vector3 nor = Vector3(0, 0, 0);
	D3DXVec3Normalize(&nor, &(pPos - mPos));

	float runSpeed = 5.0f;

	Vector3 origin = monster->GetTransform(monIndex)->GetPositon();
	origin.x += runSpeed * nor.x * Time::Delta();
	origin.z += runSpeed * nor.z * Time::Delta();
	monster->PlayClip(monIndex, 1, 1.0f, 1.0f);

	if(D3DXVec3Length(&(pPos - origin)) < 1.0f)
	{
		return;
	}
	monster->GetTransform(monIndex)->Position(origin.x, 0, origin.z);
}

void AssExampleDemo::MonsterPatrol(ModelAnimator * monster, bool& mbPatrolState, float* ranX, float* ranZ, UINT monIndex)
{
	//순찰 종료 조건
	//1. 플레이어가 search range에 있을때. --PlayerTracking의 반환값으로 판별
	//2. 해당 정점에 도착했을때. -- patrol 함수내에서 판별
	Vector3 mPos = monster->GetTransform(monIndex)->GetPositon();
	float runSpeed = 10.0f;

	if (mbPatrolState == false)
	{
		ranX[monIndex] = rand() % 80 - 40;
		ranZ[monIndex] = rand() % 80 - 40;
		mbPatrolState = true;
	}
	if (monster->CurrClipNumber(monIndex) != 3)
	{
		RotateTowards(monster, Vector3(ranX[monIndex], 0, ranZ[monIndex]), monIndex);
	}

	Vector3 nor = Vector3(0, 0, 0);
	D3DXVec3Normalize(&nor, &(Vector3(ranX[monIndex], 0, ranZ[monIndex]) - mPos));

	Vector3 origin = monster->GetTransform(monIndex)->GetPositon();
	origin.x += runSpeed * nor.x * Time::Delta();
	origin.z += runSpeed * nor.z * Time::Delta();

	monster->GetTransform(monIndex)->Position(origin.x, 0, origin.z);
	monster->PlayClip(monIndex, 1, 1.0f, 1.0f);
	

	if (D3DXVec3Length(&(Vector3(ranX[monIndex], 0, ranZ[monIndex]) - origin)) < 1.0f)
	{
		mbPatrolState = false;
	}

}

void AssExampleDemo::RotateTowards(ModelAnimator * modelAnim, Vector3 targetPos, UINT monIndex)
{
	Vector3 mPos = modelAnim->GetTransform(monIndex)->GetPositon();
	Vector3 nor = Vector3(0, 0, 0);

	D3DXVec3Normalize(&nor, &(targetPos - mPos));
	Vector3 front = Vector3(0, 0, -1);

	float radian = acos(D3DXVec3Dot(&front, &nor));
	Vector3  right;
	D3DXVec3Cross(&right, &front, &D3DXVECTOR3(0.0f, 2.0f, 0.0f));

	if (D3DXVec3Dot(&right, &nor) > 0)
	{
		modelAnim->GetTransform(monIndex)->Rotation(0, -radian, 0);
	}
	else
	{
		modelAnim->GetTransform(monIndex)->Rotation(0, +radian, 0);
	}
}

void AssExampleDemo::MonsterAttack(ModelAnimator * monster, ModelAnimator * player, string monsterName, UINT monIndex)
{
	Vector3 pPos = player->GetTransform(0)->GetPositon();
	Matrix transform;

	if (monsterName == "archer")
	{
		//archerWeaponTransform = archerWeapon[0]->AddTransform();
		if (arrowdelayTime[monIndex] > 2.0f)
		{
			arrowdelayTime[monIndex] = 0.0f;
			for (int i = 0; i < 100; i++)
			{
				if (archerWeaponTransform[i] == NULL)
				{
					archerWeaponTransform[i] = new Transform();
					archerWeaponTransform[i] = archerWeapon->AddTransform();
					archerWeaponTransform[i]->Scale(0.35f, 0.35f, 0.35f);
					archerWeaponTransform[i]->Rotation(Math::ToRadian(90), archer->GetTransform(monIndex)->GetRotation().y, 0);
					archerWeaponTransform[i]->Position(archer->GetTransform(monIndex)->GetPositon().x, 10, archer->GetTransform(monIndex)->GetPositon().z);

					Vector3 pPosYup = Vector3(pPos.x, pPos.y + archerWeaponTransform[i]->GetPositon().y, pPos.z);
					D3DXVec3Normalize(&arrowNorMal[i], &(pPosYup - archerWeaponTransform[i]->GetPositon()));

					archerArrowCollider[i].Init = new Transform();
					archerArrowCollider[i].Init->Position(0, 0, 1);
					archerArrowCollider[i].Init->Scale(1, 10, 1);

					archerArrowCollider[i].Transform = new Transform();
					archerArrowCollider[i].Collider = new Collider(archerArrowCollider[i].Transform, archerArrowCollider[i].Init);
					archerWeaponTransform[i]->Update();
					archerWeapon->Update();
					archerWeapon->UpdateTransforms();
					break;
				}
				else if (archerWeaponTransform[i] != NULL && (archerWeaponTransform[i]->GetPositon().x > 200 ||
					archerWeaponTransform[i]->GetPositon().x < -200 ||
					archerWeaponTransform[i]->GetPositon().z >  200 ||
					archerWeaponTransform[i]->GetPositon().z < -200))
				{
					archerWeaponTransform[i]->Rotation(Math::ToRadian(90), archer->GetTransform(monIndex)->GetRotation().y, 0);
					archerWeaponTransform[i]->Position(archer->GetTransform(monIndex)->GetPositon().x, 10, archer->GetTransform(monIndex)->GetPositon().z);

					Vector3 pPosYup = Vector3(pPos.x, pPos.y + archerWeaponTransform[i]->GetPositon().y, pPos.z);
					D3DXVec3Normalize(&arrowNorMal[i], &(pPosYup - archerWeaponTransform[i]->GetPositon()));
					break;
				}
				else
				{
					continue;
				}
			}
		}

	}
	else if (monsterName == "hallin")
	{

	}
	RotateTowards(monster, pPos, monIndex);

	if (monster->CurrClipNumber(monIndex) == 1)
	{
		monster->PlayClip(monIndex, 2, 1.0f, 1.0f);
	}

	
	monster->UpdateTransforms();

}

void AssExampleDemo::PlayerMove()
{
	Vector3 origin = michelle->GetTransform(0)->GetPositon();

	if (Keyboard::Get()->Press(VK_UP))
	{
		if (michelle->CurrClipNumber(0) != 1)
		{
			michelle->PlayClip(0, 1,1.0f, 1.0f);
		}
		origin.z += 18.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(180.f);
		michelle->GetTransform(0)->Position(origin);
	}
	if (Keyboard::Get()->Press(VK_RIGHT))
	{
		if (michelle->CurrClipNumber(0) != 1)
		{
			michelle->PlayClip(0, 1, 1.0f, 1.0f);
		}
		origin.x += 18.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(-90.f);
		michelle->GetTransform(0)->Position(origin);
	}
	if (Keyboard::Get()->Press(VK_LEFT))
	{
		if (michelle->CurrClipNumber(0) != 1)
		{
			michelle->PlayClip(0, 1, 1.0f, 1.0f);
		}
		origin.x -= 18.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(90.0f);
		michelle->GetTransform(0)->Position(origin);
	}
	if (Keyboard::Get()->Press(VK_DOWN))
	{
		if (michelle->CurrClipNumber(0) != 1)
		{
			michelle->PlayClip(0, 1, 1.0f, 1.0f);
		}
		origin.z -= 18.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(0.0f);
		michelle->GetTransform(0)->Position(origin);
	}

	/////////////////////////////////////
	if (Keyboard::Get()->Press(VK_UP) && Keyboard::Get()->Press(VK_LEFT))
	{
		if (michelle->CurrClipNumber(0) != 1)
		{
			michelle->PlayClip(0, 1, 1.0f, 1.0f);
		}
		origin.x -= 6.0f * Time::Delta();
		origin.z += 6.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(135.0f);
		michelle->GetTransform(0)->Position(origin);
	}
	else if (Keyboard::Get()->Press(VK_UP) && Keyboard::Get()->Press(VK_RIGHT))
	{
		if (michelle->CurrClipNumber(0) != 1)
		{
			michelle->PlayClip(0, 1, 1.0f, 1.0f);
		}
		origin.x += 6.0f * Time::Delta();
		origin.z += 6.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(-135.0f);
		michelle->GetTransform(0)->Position(origin);
	}
	else if (Keyboard::Get()->Press(VK_RIGHT) && Keyboard::Get()->Press(VK_DOWN))
	{
		if (michelle->CurrClipNumber(0) != 1)
		{
			michelle->PlayClip(0, 1, 1.0f, 1.0f);
		}
		origin.x += 6.0f * Time::Delta();
		origin.z -= 6.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(-45.0f);
		michelle->GetTransform(0)->Position(origin);
	}
	else if (Keyboard::Get()->Press(VK_LEFT) && Keyboard::Get()->Press(VK_DOWN))
	{
		if (michelle->CurrClipNumber(0) != 1)
		{
			michelle->PlayClip(0, 1, 1.0f, 1.0f);
		}
		origin.x -= 6.0f * Time::Delta();
		origin.z -= 6.0f * Time::Delta();
		michelle->GetTransform(0)->rotation.y = Math::ToRadian(45.0f);
		michelle->GetTransform(0)->Position(origin);
	}

	if (bPlayerHitReaction)
	{
		michelle->PlayClip(0, 3, 1.0f, 1.0f);
	}
	if (Keyboard::Get()->Press('B'))
	{
		michelle->PlayClip(0, 5, 1.0f, 2.0f);
	}
	if (Keyboard::Get()->Press('K'))
	{
		michelle->PlayClip(0, 4, 1.0f, 1.0f);
	}
	if (!Keyboard::Get()->Press(VK_LEFT) &&
		!Keyboard::Get()->Press(VK_UP) &&
		!Keyboard::Get()->Press(VK_RIGHT) &&
		!Keyboard::Get()->Press(VK_DOWN) &&
		!Keyboard::Get()->Down(VK_SPACE) &&
		!Keyboard::Get()->Press('K') &&
		!Keyboard::Get()->Press('B') &&
		!bPlayerHitReaction)
	{
		michelle->PlayClip(0, 0, 1.0f, 1.0f);
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

void AssExampleDemo::ArrowUpdate()
{

	for (int i = 0; i < 100; i++)
	{

		if (archerWeaponTransform[i] != NULL)
		{

			Matrix attach = archerWeaponTransform[i]->World();

			Vector3 origin = archerWeaponTransform[i]->GetPositon();
			float speed = 30.0f;
			origin.x += arrowNorMal[i].x * Time::Delta() * speed;
			origin.z += arrowNorMal[i].z * Time::Delta() * speed;
			archerWeaponTransform[i]->Position(origin.x, 10, origin.z);

			archerArrowCollider[i].Collider->GetTransform()->World(attach);
			archerArrowCollider[i].Collider->Update();
			archerWeaponTransform[i]->Update();


			archerWeapon->Update();
			archerWeapon->UpdateTransforms();
		}
		else
		{
			continue;
		}
	}
}


