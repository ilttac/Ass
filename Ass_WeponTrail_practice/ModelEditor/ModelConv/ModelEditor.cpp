
#include "stdafx.h"
#include "ModelEditor.h"
#include "Systems/ImSequencer.h"
#include "Converter.h"
#include "Utilities/Xml.h"
#include "Systems/imgui_internal.h"

static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float* m16)
{
	float temp, temp2, temp3, temp4;
	temp = 2.0f * znear;
	temp2 = right - left;
	temp3 = top - bottom;
	temp4 = zfar - znear;
	m16[0] = temp / temp2;
	m16[1] = 0.0;
	m16[2] = 0.0;
	m16[3] = 0.0;
	m16[4] = 0.0;
	m16[5] = temp / temp3;
	m16[6] = 0.0;
	m16[7] = 0.0;
	m16[8] = (right + left) / temp2;
	m16[9] = (top + bottom) / temp3;
	m16[10] = (-zfar - znear) / temp4;
	m16[11] = -1.0f;
	m16[12] = 0.0;
	m16[13] = 0.0;
	m16[14] = (-temp * zfar) / temp4;
	m16[15] = 0.0;
}

void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float* m16)
{
	float ymax, xmax;
	ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
	xmax = ymax * aspectRatio;
	Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
}

void Cross(const float* a, const float* b, float* r)
{
	r[0] = a[1] * b[2] - a[2] * b[1];
	r[1] = a[2] * b[0] - a[0] * b[2];
	r[2] = a[0] * b[1] - a[1] * b[0];
}

float Dot(const float* a, const float* b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void Normalize(const float* a, float* r)
{
	float il = 1.f / (sqrtf(Dot(a, a)) + FLT_EPSILON);
	r[0] = a[0] * il;
	r[1] = a[1] * il;
	r[2] = a[2] * il;
}

void LookAt(const float* eye, const float* at, const float* up, float* m16)
{
	float X[3], Y[3], Z[3], tmp[3];

	tmp[0] = eye[0] - at[0];
	tmp[1] = eye[1] - at[1];
	tmp[2] = eye[2] - at[2];
	//Z.normalize(eye - at);
	Normalize(tmp, Z);
	Normalize(up, Y);
	//Y.normalize(up);

	Cross(Y, Z, tmp);
	//tmp.cross(Y, Z);
	Normalize(tmp, X);
	//X.normalize(tmp);

	Cross(Z, X, tmp);
	//tmp.cross(Z, X);
	Normalize(tmp, Y);
	//Y.normalize(tmp);

	m16[0] = X[0];
	m16[1] = Y[0];
	m16[2] = Z[0];
	m16[3] = 0.0f;
	m16[4] = X[1];
	m16[5] = Y[1];
	m16[6] = Z[1];
	m16[7] = 0.0f;
	m16[8] = X[2];
	m16[9] = Y[2];
	m16[10] = Z[2];
	m16[11] = 0.0f;
	m16[12] = -Dot(X, eye);
	m16[13] = -Dot(Y, eye);
	m16[14] = -Dot(Z, eye);
	m16[15] = 1.0f;
}

void OrthoGraphic(const float l, float r, float b, const float t, float zn, const float zf, float* m16)
{
	m16[0] = 2 / (r - l);
	m16[1] = 0.0f;
	m16[2] = 0.0f;
	m16[3] = 0.0f;
	m16[4] = 0.0f;
	m16[5] = 2 / (t - b);
	m16[6] = 0.0f;
	m16[7] = 0.0f;
	m16[8] = 0.0f;
	m16[9] = 0.0f;
	m16[10] = 1.0f / (zf - zn);
	m16[11] = 0.0f;
	m16[12] = (l + r) / (l - r);
	m16[13] = (t + b) / (b - t);
	m16[14] = zn / (zn - zf);
	m16[15] = 1.0f;
}

void EditTransform(const float* cameraView, float* cameraProjection, float* matrix, bool editTransformDecomposition)
{
	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
	static bool useSnap = false;
	static float snap[3] = { 1.f, 1.f, 1.f };
	static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
	static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
	static bool boundSizing = false;
	static bool boundSizingSnap = false;

	if (editTransformDecomposition)
	{
		if (ImGui::IsKeyPressed(90))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(69))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		if (ImGui::IsKeyPressed(82)) // r Key
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
		ImGui::InputFloat3("Tr", matrixTranslation, 3);
		ImGui::InputFloat3("Rt", matrixRotation, 3);
		ImGui::InputFloat3("Sc", matrixScale, 3);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

		if (mCurrentGizmoOperation != ImGuizmo::SCALE)
		{
			if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
				mCurrentGizmoMode = ImGuizmo::LOCAL;
			ImGui::SameLine();
			if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
				mCurrentGizmoMode = ImGuizmo::WORLD;
		}
		if (ImGui::IsKeyPressed(83))
			useSnap = !useSnap;
		ImGui::Checkbox("", &useSnap);
		ImGui::SameLine();

		switch (mCurrentGizmoOperation)
		{
		case ImGuizmo::TRANSLATE:
			ImGui::InputFloat3("Snap", &snap[0]);
			break;
		case ImGuizmo::ROTATE:
			ImGui::InputFloat("Angle Snap", &snap[0]);
			break;
		case ImGuizmo::SCALE:
			ImGui::InputFloat("Scale Snap", &snap[0]);
			break;
		}
		ImGui::Checkbox("Bound Sizing", &boundSizing);
		if (boundSizing)
		{
			ImGui::PushID(3);
			ImGui::Checkbox("", &boundSizingSnap);
			ImGui::SameLine();
			ImGui::InputFloat3("Snap", boundsSnap);
			ImGui::PopID();
		}
	}
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
}

//
//
// ImSequencer interface
static const char* SequencerItemTypeNames[] = { "Camera","Music", "ScreenEffect", "FadeIn", "Animation" };


struct RampEdit : public ImCurveEdit::Delegate
{
	RampEdit()
	{
		mPts[0][0] = ImVec2(-10.f, 0);
		mPts[0][1] = ImVec2(20.f, 0.6f);
		mPts[0][2] = ImVec2(25.f, 0.2f);
		mPts[0][3] = ImVec2(70.f, 0.4f);
		mPts[0][4] = ImVec2(120.f, 1.f);
		mPointCount[0] = 5;

		mPts[1][0] = ImVec2(-50.f, 0.2f);
		mPts[1][1] = ImVec2(33.f, 0.7f);
		mPts[1][2] = ImVec2(80.f, 0.2f);
		mPts[1][3] = ImVec2(82.f, 0.8f);
		mPointCount[1] = 4;


		mPts[2][0] = ImVec2(40.f, 0);
		mPts[2][1] = ImVec2(60.f, 0.1f);
		mPts[2][2] = ImVec2(90.f, 0.82f);
		mPts[2][3] = ImVec2(150.f, 0.24f);
		mPts[2][4] = ImVec2(200.f, 0.34f);
		mPts[2][5] = ImVec2(250.f, 0.12f);
		mPointCount[2] = 6;
		mbVisible[0] = mbVisible[1] = mbVisible[2] = true;
		mMax = ImVec2(1.f, 1.f);
		mMin = ImVec2(0.f, 0.f);
	}
	size_t GetCurveCount()
	{
		return 3;
	}

	bool IsVisible(size_t curveIndex)
	{
		return mbVisible[curveIndex];
	}
	size_t GetPointCount(size_t curveIndex)
	{
		return mPointCount[curveIndex];
	}

	uint32_t GetCurveColor(size_t curveIndex)
	{
		uint32_t cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
		return cols[curveIndex];
	}
	ImVec2* GetPoints(size_t curveIndex)
	{
		return mPts[curveIndex];
	}
	virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const { return ImCurveEdit::CurveSmooth; }
	virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
	{
		mPts[curveIndex][pointIndex] = ImVec2(value.x, value.y);
		SortValues(curveIndex);
		for (size_t i = 0; i < GetPointCount(curveIndex); i++)
		{
			if (mPts[curveIndex][i].x == value.x)
				return (int)i;
		}
		return pointIndex;
	}
	virtual void AddPoint(size_t curveIndex, ImVec2 value)
	{
		if (mPointCount[curveIndex] >= 8)
			return;
		mPts[curveIndex][mPointCount[curveIndex]++] = value;
		SortValues(curveIndex);
	}
	virtual ImVec2& GetMax() { return mMax; }
	virtual ImVec2& GetMin() { return mMin; }
	virtual unsigned int GetBackgroundColor() { return 0; }
	ImVec2 mPts[3][8];
	size_t mPointCount[3];
	bool mbVisible[3];
	ImVec2 mMin;
	ImVec2 mMax;
private:
	void SortValues(size_t curveIndex)
	{
		auto b = std::begin(mPts[curveIndex]);
		auto e = std::begin(mPts[curveIndex]) + GetPointCount(curveIndex);
		std::sort(b, e, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });

	}
};

struct MySequence : public ImSequencer::SequenceInterface
{
	// interface with sequencer

	virtual int GetFrameMin() const {
		return mFrameMin;
	}
	virtual int GetFrameMax() const {
		return mFrameMax;
	}
	virtual int GetItemCount() const { return (int)myItems.size(); }

	virtual int GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char*); }
	virtual const char* GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex]; }
	virtual const char* GetItemLabel(int index) const
	{
		static char tmps[512];
		sprintf_s(tmps, "[%02d] %s", index, SequencerItemTypeNames[myItems[index].mType]);
		return tmps;
	}

	virtual void Get(int index, int** start, int** end, int* type, unsigned int* color)
	{
		MySequenceItem& item = myItems[index];
		if (color)
			* color = 0xFFAA8080; // same color for everyone, return color based on type
		if (start)
			* start = &item.mFrameStart;
		if (end)
			* end = &item.mFrameEnd;
		if (type)
			* type = item.mType;
	}
	virtual void Add(int type) { myItems.push_back(MySequenceItem{ type, 0, 10, false }); };
	virtual void Del(int index) { myItems.erase(myItems.begin() + index); }
	virtual void Duplicate(int index) { myItems.push_back(myItems[index]); }

	virtual size_t GetCustomHeight(int index) { return myItems[index].mExpanded ? 300 : 0; }

	// mydatas
	MySequence() : mFrameMin(0), mFrameMax(0) {}
	int mFrameMin, mFrameMax;
	struct MySequenceItem
	{
		int mType;
		int mFrameStart, mFrameEnd;
		bool mExpanded;
	};
	std::vector<MySequenceItem> myItems;
	RampEdit rampEdit;

	virtual void DoubleClick(int index) {
		if (myItems[index].mExpanded)
		{
			myItems[index].mExpanded = false;
			return;
		}
		for (auto& item : myItems)
			item.mExpanded = false;
		myItems[index].mExpanded = !myItems[index].mExpanded;
	}

	virtual void CustomDraw(int index, ImDrawList * draw_list, const ImRect & rc, const ImRect & legendRect, const ImRect & clippingRect, const ImRect & legendClippingRect)
	{
		static const char* labels[] = { "Translation", "Rotation" , "Scale" };

		rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
		rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
		draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
		for (int i = 0; i < 3; i++)
		{
			ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
			ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
			draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
			if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
				rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
		}
		draw_list->PopClipRect();

		ImGui::SetCursorScreenPos(rc.Min);
		ImCurveEdit::Edit(rampEdit, rc.Max - rc.Min, 137 + index, &clippingRect);
	}

	virtual void CustomDrawCompact(int index, ImDrawList * draw_list, const ImRect & rc, const ImRect & clippingRect)
	{
		rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
		rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
		draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < rampEdit.mPointCount[i]; j++)
			{
				float p = rampEdit.mPts[i][j].x;
				if (p < myItems[index].mFrameStart || p > myItems[index].mFrameEnd)
					continue;
				float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
				float x = ImLerp(rc.Min.x, rc.Max.x, r);
				draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
			}
		}
		draw_list->PopClipRect();
	}
};

inline void rotationY(const float angle, float* m16)
{
	float c = cosf(angle);
	float s = sinf(angle);

	m16[0] = c;
	m16[1] = 0.0f;
	m16[2] = -s;
	m16[3] = 0.0f;
	m16[4] = 0.0f;
	m16[5] = 1.f;
	m16[6] = 0.0f;
	m16[7] = 0.0f;
	m16[8] = s;
	m16[9] = 0.0f;
	m16[10] = c;
	m16[11] = 0.0f;
	m16[12] = 0.f;
	m16[13] = 0.f;
	m16[14] = 0.f;
	m16[15] = 1.0f;
}

MySequence mySequence;
void ModelEditor::Initialize()
{
	// sequence with default values

	mySequence.mFrameMin = -100;
	mySequence.mFrameMax = 1000;
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 0, 10, 30, false });
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 1, 20, 30, true });
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 3, 12, 60, false });
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 2, 61, 90, false });
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 4, 90, 99, false });

	importer = new Assimp::Importer();

	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(1, 25, -50);
	((Freedom*)Context::Get()->GetCamera())->Speed(20, 5);

	shader = new Shader(L"57_ParticleViewer.fxo");
	modelShader = new Shader(L"32_Model.fxo");
	shadow = new Shadow(shader, Vector3(0, 0, 0), 65);

	sky = new Sky(shader);
	sky->ScatteringPass(3);
	sky->RealTime(false, Math::PI - 1e-6f, 0.5f);

	Mesh();
}

void ModelEditor::Destroy()
{
	SafeDelete(shader);
	SafeDelete(shadow);

	SafeDelete(sky);

	SafeDelete(floor);
	SafeDelete(stone);

	SafeDelete(sphere);
	SafeDelete(grid);

	SafeDelete(particleSystem);
	SafeDelete(importer);
	SafeDelete(modelAnimator)
}

void ModelEditor::Update()
{
	///////////////

	static const float identityMatrix[16] =
	{ 1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };
	ImGuizmo::SetOrthographic(!isPerspective);
	ImGuizmo::BeginFrame();

	ImGui::Begin("Editor");
	ImGui::Text("Camera");
	bool viewDirty = false;
	if (ImGui::RadioButton("Perspective", isPerspective)) isPerspective = true;
	ImGui::SameLine();
	if (ImGui::RadioButton("Orthographic", !isPerspective)) isPerspective = false;
	if (isPerspective)
	{
		ImGui::SliderFloat("Fov", &fov, 20.f, 110.f);
	}
	else
	{
		ImGui::SliderFloat("Ortho width", &viewWidth, 1, 20);
	}
	viewDirty |= ImGui::SliderFloat("Distance", &camDistance, 1.f, 10.f);
	ImGui::SliderInt("Gizmo count", &gizmoCount, 1, 4);

	//if (viewDirty || firstFrame)
	//{
	//	float eye[] = { cosf(camYAngle) * cosf(camXAngle) * camDistance, sinf(camXAngle) * camDistance, sinf(camYAngle) * cosf(camXAngle) * camDistance };
	//	float at[] = { 0.f, 0.f, 0.f };
	//	float up[] = { 0.f, 1.f, 0.f };
	//	LookAt(eye, at, up, cameraView);
	//	firstFrame = false;
	//}

	//ImGuizmo::DrawCubes(cameraView, cameraProjection, &objectMatrix[0][0], gizmoCount);
	ImGui::Separator();

	for (int matId = 0; matId < gizmoCount; matId++)
	{
		ImGuizmo::SetID(matId);
		if (modelAnimator != NULL)
		{
			EditTransform(Context::Get()->View(), Context::Get()->Projection(), &modelAnimator->GetTransform(0)->World()[matId], lastUsing == matId);
			modelAnimator->UpdateTransforms();
		}
		if (ImGuizmo::IsUsing())
		{
			lastUsing = matId;
		}
	}
	ImGui::End();

	//ImGuizmo::DrawGrid(Context::Get()->View(), Context::Get()->Projection(), identityMatrix, 10.f);
	// let's create the sequencer
	static int selectedEntry = -1;
	static int firstFrame1 = 0;
	static bool expanded = true;
	static int currentFrame = 100;

///*	ImGui::Begin("Sequencer");
//
//	ImGui::PushItemWidth(130);
//	ImGui::InputInt("Frame Min", &mySequence.mFrameMin);
//	ImGui::SameLine();
//	ImGui::InputInt("Frame ", &currentFrame);
//	ImGui::SameLine();
//	ImGui::InputInt("Frame Max", &mySequence.mFrameMax);
//	ImGui::PopItemWidth();
//	Sequencer(&mySequence, &currentFrame, &expanded, &selectedEntry, &firstFrame1, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
//	*/// add a UI to edit that particular item
//	if (selectedEntry != -1)
//	{
//		const MySequence::MySequenceItem& item = mySequence.myItems[selectedEntry];
//		ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
//		// switch (type) ....
//	}
//	ImGui::End();
//	ImGuizmo::ViewManipulate(cameraView, camDistance, ImVec2(io.DisplaySize.x - 128, 0), ImVec2(128, 128), 0x10101010);
	///////////////

	//Imgui Set
	{
		MainMenu();
		Project();
		Hiarachy();
		Inspector();
	}

	sky->Update();

	grid->Update();
	sphere->Update();

	Vector3 P;
	sphere->GetTransform(0)->Position(&P);
	float moveSpeed = 30.0f;

	if (Mouse::Get()->Press(1) == false)
	{
		const Vector3& F = Context::Get()->GetCamera()->Foward();
		const Vector3& R = Context::Get()->GetCamera()->Right();
		const Vector3& U = Context::Get()->GetCamera()->Up();
		if (Keyboard::Get()->Press('W'))
			P += Vector3(F.x, 0, F.z) * moveSpeed * Time::Delta();
		else if (Keyboard::Get()->Press('S'))
			P += Vector3(-F.x, 0, -F.z) * moveSpeed * Time::Delta();

		if (Keyboard::Get()->Press('A'))
			P += -R * moveSpeed * Time::Delta();
		else if (Keyboard::Get()->Press('D'))
			P += R * moveSpeed * Time::Delta();

		if (Keyboard::Get()->Press('E'))
			P += U * moveSpeed * Time::Delta();
		else if (Keyboard::Get()->Press('Q'))
			P += -U * moveSpeed * Time::Delta();
	}
	sphere->GetTransform(0)->Position(P);
	sphere->UpdateTransforms();

	if (particleSystem != NULL)
	{
		particleSystem->Add(P);
		particleSystem->Update();
	}

	if (modelAnimator != NULL)
	{
		modelAnimator->Update();
	}
}

void ModelEditor::PreRender()
{
	sky->PreRender();
	//Depth
	{
		shadow->Set();
		Pass(0);
		sphere->Render();
	}

}

void ModelEditor::Render()
{
	sky->Pass(4, 5, 6);
	sky->Render();

	Pass(7);
	stone->Render();
	sphere->Render();

	floor->Render();
	grid->Render();

	if (particleSystem != NULL)
	{
		particleSystem->Render();
	}

	if (modelAnimator != NULL)
	{
		modelAnimator->Render();
	}
}


////////////////////////////////////

void ModelEditor::Mesh()
{
	//Create Material
	{
		//바닥
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		floor->SpecularMap("Floor_Specular.png");
		floor->NormalMap("Floor_Normal.png");
		floor->Specular(1, 1, 1, 15);
		floor->Emissive(0.2f, 0.2f, 0.2f, 0.3f);

		//구
		stone = new Material(shader);
		stone->DiffuseMap("Bricks.png");
		stone->SpecularMap("Bricks_Specular.png");
		stone->NormalMap("Bricks_Normal.png");
		stone->Specular(0.3f, 0.3f, 0.3f, 20.0f);
		stone->Emissive(0.2f, 0.2f, 0.2f, 0.3f);
	}

	//Create Mesh
	{
		Transform* transform = NULL;

		grid = new MeshRender(shader, new MeshGrid(15, 15));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(20, 1, 20);

		sphere = new MeshRender(shader, new MeshSphere(0.5f, 20, 20));
		transform = sphere->AddTransform();
		transform->Position(0, 5, 0);
		transform->Scale(5, 5, 5);
	}

	sphere->UpdateTransforms();
	grid->UpdateTransforms();
	meshes.push_back(sphere);
	meshes.push_back(grid);

}


///////////////////////////////////////
//ImGui
void ModelEditor::MainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("File"))
		{
			D3DDesc desc = D3D::GetDesc();

			if (ImGui::BeginMenu("Open"))
			{ 
				if (ImGui::MenuItem(".fbx"))
				{
					Path::OpenFileDialog
					(
						openFile,
						L"fbx_File\0*.fbx",
						L"../../_Assets",
						bind(&ModelEditor::OpenFile, this, placeholders::_1),
						desc.Handle
					);
				}
				if (ImGui::MenuItem(".mesh"))
				{
					Path::OpenFileDialog
					(
						openFile,
						L"Mesh_file\0*.mesh",
						L"../../_Models",
						bind(&ModelEditor::OpenFile, this, placeholders::_1),
						desc.Handle
					);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Save"))
			{
			
				if (ImGui::MenuItem(".mesh"))
				{
					Path::SaveFileDialog
					(
						openFile,
						L".mesh\0*.mesh",
						L"../../_Models",
						bind(&ModelEditor::WriteMeshFile, this, placeholders::_1),
						desc.Handle
					);
				}
				if (ImGui::MenuItem(".material"))
				{
					Path::SaveFileDialog
					(
						openFile,
						L".material\0*.material",
						L"../../_Textures",
						bind(&ModelEditor::WriteMaterialFile, this, placeholders::_1),
						desc.Handle
					);
				}
				ImGui::EndMenu();
			}	
		ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();
}

void ModelEditor::Project()
{
	
	bool bOpen = true;
	bOpen = ImGui::Begin("Project", &bOpen);
	//ImGui::SetWindowPos(ImVec2(width - windowWidth, 0));
	//ImGui::SetWindowSize(ImVec2(windowWidth, height));
	ImGui::TextColored(ImVec4(0.1f, 0.1f, 0.9f, 1.0f), "Models");
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Meshes"), ImGuiTreeNodeFlags_DefaultOpen)
	{

	}
	if (ImGui::CollapsingHeader("Materials"), ImGuiTreeNodeFlags_DefaultOpen)
	{

	}
	if (ImGui::CollapsingHeader("Behavior Trees"), ImGuiTreeNodeFlags_DefaultOpen)
	{

	}
	ImGui::End();

}

void ModelEditor::Hiarachy()
{
	bool bOpen = true;
	bOpen = ImGui::Begin("Hiarachy", &bOpen);
	//ImGui::SetWindowPos(ImVec2(width - windowWidth, 0));
	//ImGui::SetWindowSize(ImVec2(windowWidth, height));
	ImGui::TextColored(ImVec4(0.1f, 0.1f, 0.9f, 1.0f), "Models");
	ImGui::Separator();
	//if (ImGui::CollapsingHeader("Meshes"), ImGuiTreeNodeFlags_DefaultOpen)
	//{

	//}
	ImGui::End();
}

void ModelEditor::Inspector()
{
	bool bOpen = true;
	bOpen = ImGui::Begin("Inspector", &bOpen);
	ImGui::Separator();
	ImGui::Separator();
	ImGui::End();
}
//ImGui
///////////////////////////////////////////

void ModelEditor::Pass(UINT meshPass)
{
	for (MeshRender* mesh : meshes)
		mesh->Pass(meshPass);
}

void ModelEditor::WriteMeshFile(wstring file)
{
	if (modelAnimator != NULL)
	{
		modelAnimator->GetModel()->WriteMeshData(file,false);
	}
}

void ModelEditor::WriteMaterialFile(wstring file)
{
	if (modelAnimator != NULL)
	{
		modelAnimator->GetModel()->WriteMaterial(file, false);
	}
}

void ModelEditor::OpenFile(wstring file)
{
	//파일 확장자가 fbx냐 .mesh 에따라 따르게 만듬.
	//fbx면 일단 기본저장된 mesh정보를 읽고 쓴정보를 다시 불러와 모델을만듬
	//.mesh이면 mesh,matrial 정보를찾아서 불러옴 \
	//또다른 포맷 ex).mod 다른 바이너리 파일 -- 나중에  \

	wstring ext = Path::GetExtension(file);
	wstring fileDirectory = Path::GetLastDirectoryName(file);
	wstring fileName = Path::GetFileNameWithoutExtension(file);
	transform(ext.begin(), ext.end(), ext.begin(), toupper);
	if (ext == L"FBX")
	{
		//어떻게 파일명 ? 
		OpenFbxFile(fileDirectory + L"/" + fileName); // ex)Archer/Archer
	}
	else if (ext == L"MESH")
	{
		OpenMeshFile(fileDirectory + L"/" +fileName);
	}
}

void ModelEditor::OpenFbxFile(wstring file)
{
	Converter* conv = new Converter();
	conv->ReadFile(file+L".fbx");
	conv->ExportMaterial(file, false);
	conv->ExportMesh(file,false);
	SafeDelete(conv);

	wstring fileDirectory = Path::GetLastDirectoryName(file);
	wstring fileName = Path::GetFileNameWithoutExtension(file);
	modelAnimator = new ModelRender(modelShader);
	modelAnimator->ReadMaterial(fileDirectory +L"/"+fileName);
	modelAnimator->ReadMesh(fileDirectory + L"/"+ fileName);
	
	Transform* attachTransform = modelAnimator->AddTransform();
	attachTransform->Position(-10, 0, -10);
	attachTransform->Scale(0.1f, 0.1f, 0.1f);
	

	modelAnimator->UpdateTransforms();
	modelAnimator->Pass(1);
}

void ModelEditor::OpenMeshFile(wstring file)
{
	wstring fileDirectory = Path::GetLastDirectoryName(file);

	modelAnimator = new ModelRender(modelShader);
	modelAnimator->ReadMaterial(fileDirectory + L"/Mesh");
	modelAnimator->ReadMesh(fileDirectory + L"/Mesh");

	Transform * attachTransform = modelAnimator->AddTransform();
	attachTransform->Position(-10, 0, -10);
	attachTransform->Scale(0.1f, 0.1f, 0.1f);

	modelAnimator->UpdateTransforms();
	modelAnimator->Pass(1);
}
