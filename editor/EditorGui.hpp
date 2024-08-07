//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "raygui.h"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "EditorSettings.hpp"
#include "Settings.hpp"
#include "UserInput.hpp"
#include "windows/FloatingWindow.hpp"
#include "windows/Window.hpp"
#include <gui_window_file_dialog.h>

#include <entt/entt.hpp>

#include <memory>
#include <string>
#include <vector>

namespace sage::editor
{
    class EditorGui
    {
        Camera* camera;
        std::unique_ptr<FloatingWindow> objectprops;
        std::unique_ptr<FloatingWindow> toolprops;
        std::unique_ptr<FloatingWindow> toolbox;
        std::unique_ptr<GuiWindowFileDialogState> fileDialogState;
        std::vector<Window*> windows;
        Vector2 screenSize{};
        EditorSettings* editorSettings;
        Settings* settings;

        void onWindowResize(Vector2 newScreenSize);
        static void drawDebugCollisionText(Cursor* cursor);

      public:
        bool focused = false;
        entt::sigh<void()> saveButtonPressed;
        entt::sigh<void()> onFileOpened;

        void OpenFileDialog();
        void GuiFocused();
        void GuiNotFocused();
        void Update();
        void Draw(const std::string& mode, Cursor* cursor);
        EditorGui(
            EditorSettings* _editorSettings,
            Settings* _settings,
            UserInput* _userInput,
            Camera* camera);
    };
} // namespace sage::editor
// editor
