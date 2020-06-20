#pragma once

#include "Systems/IExecute.h"

class AssExampleDemo : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {};
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override {};
	virtual void Render() override;
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void Mesh();
	void Michelle();
	void Hallin();
	void Archer();
	void Pass(UINT mesh, UINT model, UINT anim);
	
	void PlayerTracking(ModelAnimator* monster, ModelAnimator* player,UINT monIndex);
	void MonsterPatrol(ModelAnimator* monster, bool& mbPatrolState,float* randomX,float* randomZ, UINT monIndex);
	void RotateTowards(ModelAnimator* modelAnim, Vector3 targetPos, UINT monIndex);
	void MonsterAttack(ModelAnimator* monster, ModelAnimator* player,string monsterName, UINT monIndex);
	void PlayerMove();
	void PlayerWeaponChange();
	void ArrowUpdate();

private:
	const static UINT arrowCount = 100;
	enum { eMonsterMaxNum = 10};
	Shader * shader;
	
	class SkyCube* sky;

	Material* floor;
	MeshRender* grid;
	
	Model* weapon;
	Model* weapon2;
	ModelRender* archerWeapon;
	Transform* archerWeaponTransform[arrowCount] = { NULL ,};
	Vector3 arrowNorMal[arrowCount];
	float arrowdelayTime[eMonsterMaxNum] = { 0.0f ,};

	ModelAnimator* michelle = NULL;
	ModelAnimator* hallin = NULL;
	ModelAnimator* archer = NULL;
	ModelAnimator* michelle2 = NULL;
	UINT weaponCount;

	//Monster 가 여러명이라면 index 로 관리
	float ranArcherX[10] = { 0.0f , };
	float ranArcherZ[10] = { 0.0f ,};
	float ranHallinX[10] = { 0.0f , };
	float ranHallinZ[10] = { 0.0f , };
	

	Vector3 cameraPos;


	bool ColliderRenderSwitchState;

	bool bPatrolState[eMonsterMaxNum] = {false,}; //순찰중이면 ranX,Z의 값을 바꾸면 안됨.
	bool bSearchState[eMonsterMaxNum] = { false, }; //search range에 들어오면 true
	bool bAttakRangeState[eMonsterMaxNum] = { false, };

	//archer
	bool bArcherPatrolState[eMonsterMaxNum] = {false,};	//순찰중이면 ranX,Z의 값을 바꾸면 안됨.
	bool bArcherSearchState[eMonsterMaxNum] = {false,};	//search range에 들어오면 true
	bool bArcherAttakRangeState[eMonsterMaxNum] = {false,};


	bool bWeaponcolliderState[eMonsterMaxNum] = {false,};
	bool bWeaponArcherObbState[eMonsterMaxNum] = {false,};
	bool bPlayerAttackState;
	bool bPlayerHitReaction[2] = {false,};

	
	enum CameraState
	{
		eCameraIdle = 0,
		eCameraLeft ,
		eCameraTop ,
		eCameraRight,
		eCameraBottom
	};

	CameraState cameraState;

	//player
	struct ColliderDesc
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}collider[eMonsterMaxNum];

	struct PlayerObbCollider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}playerObbCollider[eMonsterMaxNum];
	struct WeaponCollider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}weaponCollider[eMonsterMaxNum];
	
	//hallin
	struct HallinObbCollider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}hallinObbCollider[eMonsterMaxNum];
	struct MColliderDesc
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}mcollider[eMonsterMaxNum];

	struct MAttackColliderDesc
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}mAtkcollider[eMonsterMaxNum];

	//Archer
	struct ArcherObbColider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}archerObbColider[eMonsterMaxNum];
	struct ArcherAttackCollider
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}archerAtkCollider[eMonsterMaxNum];

	struct ArcherSearchCollider
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}archerSerachCollider[eMonsterMaxNum];

	struct ArcherArrowCollider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}archerArrowCollider[100];
	///////////

	vector<MeshRender*> meshes;
	vector<ModelRender*> models;
	vector<ModelAnimator*> animators;
};