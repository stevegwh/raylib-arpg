//
// Created by Steve Wheeler on 12/05/2024.
//

#include "DialogueWindow.hpp"

#include "raygui.h"

namespace sage
{
	void DialogueWindow::Update()
	{
	}

	void DialogueWindow::Draw()
	{
		GuiWindowBox({
			             static_cast<float>(settings->screenWidth) - contentSize.x,
			             static_cast<float>(settings->screenHeight) - contentSize.y,
			             contentSize.x, contentSize.y
		             }, "#198# PORTABLE WINDOW");
	}

	DialogueWindow::DialogueWindow(Settings* _settings) :
		settings(_settings), contentSize({300, 200})
	{
	}
} // sage
