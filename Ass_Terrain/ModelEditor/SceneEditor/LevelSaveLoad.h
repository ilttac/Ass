#pragma once

class LevelSaveLoad
{
private:
	friend class SceneEditor;

private:
	void SaveLevel(SceneEditor& SceneData);
	void LoadLevel(SceneEditor& SceneData);

};