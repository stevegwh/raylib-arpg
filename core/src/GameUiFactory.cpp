//
// Created by Steve Wheeler on 03/10/2024.
//

#include "GameUiFactory.hpp"
#include "GameUiEngine.hpp"

namespace sage
{

    void GameUiFactory::CreateLootWindow(GameUIEngine* engine, Vector2 pos)
    {
        auto window = engine->CreateWindow(pos, 200, 200);
        auto table = window->CreateTable();
        auto row = table->CreateTableRow();
        auto cell = row->CreateTableCell();
        cell->CreateTextbox("Number 1!");
        auto cell2 = row->CreateTableCell();
        cell2->CreateTextbox("Number 2!");
        auto cell3 = row->CreateTableCell();
        cell3->CreateTextbox("Number 3!");
    }

    void GameUiFactory::CreateInventoryWindow(GameUIEngine* engine, Vector2 pos, float w, float h)
    {
        engine->CreateWindow(pos, w, h);
    }
} // namespace sage