#include "stdafx.h"
#include "Convert.h"
#include "Converter.h"


void Convert::Initialize()
{
	//Airplane();
	//Tower();
	//Tank();
	//Kachujin();
	//Megan();
	//Weapon();
	//Michelle();
	//Kumata();
	//Hallin();
	Archer();
}

void Convert::Airplane()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"B787/Airplane.fbx");
	conv->ExportMaterial(L"B787/Airplane", false);
	conv->ExportMesh(L"B787/Airplane",false);
	SafeDelete(conv);
}

void Convert::Tower()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Tower/Tower.fbx");
	conv->ExportMaterial(L"Tower/Tower", false);
	conv->ExportMesh(L"Tower/Tower", false);
	SafeDelete(conv);
}

void Convert::Tank()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Tank/Tank.fbx");
	conv->ExportMaterial(L"Tank/Tank", false);
	conv->ExportMesh(L"Tank/Tank", false);
	SafeDelete(conv);
}

void Convert::Kachujin()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Kachujin/Mesh.fbx");
	conv->ExportMaterial(L"Kachujin/Mesh", false);
	conv->ExportMesh(L"Kachujin/Mesh",false);
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Kachujin/Idle.fbx");
	conv->ExportAnimClip(0, L"Kachujin/Idle");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Kachujin/Running.fbx");
	conv->ExportAnimClip(0, L"Kachujin/Running");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Kachujin/Jump.fbx");
	conv->ExportAnimClip(0, L"Kachujin/Jump");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Kachujin/Hip_Hop_Dancing.fbx");
	conv->ExportAnimClip(0, L"Kachujin/Hip_Hop_Dancing");
	SafeDelete(conv);
}

void Convert::Megan()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Megan/Mesh.fbx");
	conv->ExportMaterial(L"Megan/Mesh", false);
	conv->ExportMesh(L"Megan/Mesh", false);
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Megan/Taunt.fbx");
	conv->ExportAnimClip(0, L"Megan/Taunt");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Megan/Dancing.fbx");
	conv->ExportAnimClip(0, L"Megan/Dancing");
	SafeDelete(conv);
}
void Convert::Michelle()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Michelle/Michelle.fbx");
	conv->ExportMaterial(L"Michelle/Mesh", false);
	conv->ExportMesh(L"Michelle/Mesh", false);
	SafeDelete(conv);
	conv = new Converter();
	conv->ReadFile(L"Michelle/Idle.fbx");
	conv->ExportAnimClip(0, L"Michelle/Idle");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Michelle/Running.fbx");
	conv->ExportAnimClip(0, L"Michelle/Running");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Michelle/Jab_Elbow_Punch.fbx");
	conv->ExportAnimClip(0, L"Michelle/Jab_Elbow_Punch");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Michelle/Hit_Reaction.fbx");
	conv->ExportAnimClip(0, L"Michelle/Hit_Reaction");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Michelle/Sword_Default.fbx");
	conv->ExportAnimClip(0, L"Michelle/Sword_Default");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Michelle/Sword_Skill.fbx");
	conv->ExportAnimClip(0, L"Michelle/Sword_Skill");
	SafeDelete(conv);

}
void Convert::Weapon()
{
	vector<wstring> names;
	names.push_back(L"Cutter.fbx");
	names.push_back(L"Cutter2.fbx");
	names.push_back(L"Dagger_epic.fbx");
	names.push_back(L"Dagger_small.fbx");
	names.push_back(L"Katana.fbx");
	names.push_back(L"LongArrow.obj");
	names.push_back(L"LongBow.obj");
	names.push_back(L"Rapier.fbx");
	names.push_back(L"Sword.fbx");
	names.push_back(L"Sword_epic.fbx");
	names.push_back(L"Sword2.fbx");

	for (wstring name : names)
	{
		Converter* conv = new Converter();
		conv->ReadFile(L"Weapon/" + name);

		String::Replace(&name, L".fbx", L"");
		String::Replace(&name, L".obj", L"");

		conv->ExportMaterial(L"Weapon/" + name,false);
		conv->ExportMesh(L"Weapon/" + name);
		SafeDelete(conv);
	}
}

void Convert::Kumata()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Kumata/kumata.fbx");
	conv->ExportMaterial(L"Kumata/Mesh", false);
	conv->ExportMesh(L"Kumata/Mesh");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Kumata/kumata_attack_01.fbx");
	conv->ExportAnimClip(0, L"Kumata/kumata_attack_01");
	SafeDelete(conv);
}

void Convert::Hallin()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Hallin/Hallin.fbx");
	conv->ExportMaterial(L"Hallin/Mesh", false);
	conv->ExportMesh(L"Hallin/Mesh", false);
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Hallin/Walking.fbx");
	conv->ExportAnimClip(0, L"Hallin/Walking");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Hallin/Zombie_Idle.fbx");
	conv->ExportAnimClip(0, L"Hallin/Zombie_Idle");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Hallin/Zombie_Attack.fbx");
	conv->ExportAnimClip(0, L"Hallin/Zombie_Attack");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Hallin/Hit_Reaction.fbx");
	conv->ExportAnimClip(0, L"Hallin/Hit_Reaction");
	SafeDelete(conv);
}
void Convert::Archer()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Archer/Archer.fbx");
	conv->ExportMaterial(L"Archer/Mesh", false);
	conv->ExportMesh(L"Archer/Mesh", false);
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Archer/Archer_Idle.fbx");
	conv->ExportAnimClip(0, L"Archer/Archer_Idle");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Archer/Archer_Run.fbx");
	conv->ExportAnimClip(0, L"Archer/Archer_Run");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Archer/Archer_Attak.fbx");
	conv->ExportAnimClip(0, L"Archer/Archer_Attak");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Archer/Archer_Dead.fbx");
	conv->ExportAnimClip(0, L"Archer/Archer_Dead");
	SafeDelete(conv);

}