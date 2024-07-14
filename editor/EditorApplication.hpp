//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "Application.hpp"
#include "EditorSettings.hpp"
#include "EditorGui.hpp"

namespace sage
{
	class EditorApplication : public Application
	{
        enum class EditorState
        {
            IDLE,
            EDITOR,
            PLAY
        };
        
        EditorState state = EditorState::IDLE;
		std::unique_ptr<EditorSettings> editorSettings;
		bool debugMode = false;
        
		void init() override;
		void draw() override;
		void enablePlayMode();
		void enableEditMode();
		void manageStates();
		void initEditorScene();
        void initGameScene();
	public:
		EditorApplication();
		void Update() override;
        static void DeserializeEditorSettings(EditorSettings& settings, const char* path);
        static void SerializeEditorSettings(EditorSettings* settings, const char* path);
    };
} // sage
