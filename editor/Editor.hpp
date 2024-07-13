//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "Application.hpp"
#include "EditorSettings.hpp"
#include "Gui.hpp"

namespace sage
{
	class Editor : public Application
	{
		std::unique_ptr<EditorSettings> editorSettings;
		bool debugMode = false;
        
		void init() override;
		void draw() override;
		void enablePlayMode();
		void enableEditMode();
		void manageScenes();
		void initEditorScene();
		void SerializeEditorSettings(EditorSettings* settings, const char* path);
		void DeserializeEditorSettings(EditorSettings& settings, const char* path);
	public:
		Editor();
		void Update() override;
		void initGameScene();
	};
} // sage
