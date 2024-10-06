//
// Created by Steve Wheeler on 03/10/2024.
//

#include "GameUiFactory.hpp"
#include "GameUiEngine.hpp"

namespace sage
{

    void GameUiFactory::CreateExampleWindow(GameUIEngine* engine, Vector2 pos)
    {
        // TODO: Have a "CreateTitleBar" with a textbox and a image of a cross for closing
        // When the title bar is clicked, you can have it get the parent's window pos and update it based on mouse
        // location (and call update children)
        // TODO: Be able to specify a cell's width

        auto window = engine->CreateWindow(pos, 500, 200);
        auto table = window->CreateTable();

        auto row = table->CreateTableRow();
        // auto row0 = table->CreateTableRow();
        auto cell = row->CreateTableCell();
        cell->CreateTextbox("Number 11111111111!");
        auto cell2 = row->CreateTableCell();
        auto cell3 = row->CreateTableCell();
        auto imagebox = cell2->CreateImagebox(LoadImage("resources/icon.png"));
        cell2->SetPaddingPercent({10, 10, 10, 10});
        imagebox->SetHoriAlignment(HoriAlignment::CENTER);
        cell3->CreateImagebox(LoadImage("resources/icon.png"));

        // auto cell0 = row->CreateTableCell();

        // cell->padding.left = 10;
        // cell2->CreateTextbox("Number 2!");

        // cell3->CreateTextbox("Number 3!");
        cell3->SetPaddingPercent({0, 0, 5, 0});

        auto row2 = table->CreateTableRow();
        auto cell4 = row2->CreateTableCell();
        auto cell5 = row2->CreateTableCell();
        auto textbox = cell4->CreateTextbox("Number 4!");
        textbox->SetHoriAlignment(HoriAlignment::LEFT);
        textbox->SetVertAlignment(VertAlignment::BOTTOM);

        cell5->CreateTextbox("Number 5!");
    }

    void GameUiFactory::CreateInventoryWindow(GameUIEngine* engine, Vector2 pos, float w, float h)
    {
        engine->CreateWindow(pos, w, h);
    }
} // namespace sage