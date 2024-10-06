//
// Created by Steve Wheeler on 03/10/2024.
//

#include "GameUiFactory.hpp"
#include "GameUiEngine.hpp"
#include "ResourceManager.hpp"

namespace sage
{

    void GameUiFactory::CreateExampleWindow(GameUIEngine* engine, Vector2 pos)
    {
        // TODO: textbox word wrap
        // TODO: Have a "CreateTitleBar" with a textbox and a image of a cross for closing
        // When the title bar is clicked, you can have it get the parent's window pos and update it based on mouse
        // location (and call update children)

        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().GetImage("resources/textures/ninepatch_button.png");
        //        info1 = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        //        info2 = {Rectangle{0.0f, 128.0f, 64.0f, 64.0f}, 16, 16, 16, 16, NPATCH_NINE_PATCH};
        //        info3 = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};

        auto window =
            engine->CreateWindow(nPatchTexture.GetImage(), 0, 0, 40, 38, WindowTableAlignment::STACK_VERTICAL);
        window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
        window->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        window->tex = window->mainNPatchTexture;
        window->SetPaddingPercent({14, 5, 3, 3});

        {
            auto table = window->CreateTable();

            auto row = table->CreateTableRow(75);
            // auto row0 = table->CreateTableRow();
            auto cell = row->CreateTableCell(50);
            cell->CreateTextbox("Number 11111111111!");
            cell->SetPaddingPercent({2, 2, 2, 2});

            cell->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
            cell->tex = window->mainNPatchTexture;

            auto cell2 = row->CreateTableCell();
            auto cell3 = row->CreateTableCell();
            cell3->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
            cell3->tex = window->mainNPatchTexture;
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

        //        {
        //            auto table = window->CreateTable();
        //
        //            auto row = table->CreateTableRow();
        //            // auto row0 = table->CreateTableRow();
        //            auto cell = row->CreateTableCell();
        //            cell->CreateTextbox("Number 11111111111!");
        //
        //            cell->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        //            cell->tex = window->mainNPatchTexture;
        //
        //            auto cell2 = row->CreateTableCell();
        //            auto cell3 = row->CreateTableCell();
        //            cell3->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        //            cell3->tex = window->mainNPatchTexture;
        //            auto imagebox = cell2->CreateImagebox(LoadImage("resources/icon.png"));
        //            cell2->SetPaddingPercent({10, 10, 10, 10});
        //            imagebox->SetHoriAlignment(HoriAlignment::CENTER);
        //            cell3->CreateImagebox(LoadImage("resources/icon.png"));
        //
        //            // auto cell0 = row->CreateTableCell();
        //
        //            // cell->padding.left = 10;
        //            // cell2->CreateTextbox("Number 2!");
        //
        //            // cell3->CreateTextbox("Number 3!");
        //            cell3->SetPaddingPercent({0, 0, 5, 0});
        //
        //            auto row2 = table->CreateTableRow();
        //            auto cell4 = row2->CreateTableCell();
        //            auto cell5 = row2->CreateTableCell();
        //            auto textbox = cell4->CreateTextbox("Number 4!");
        //            textbox->SetHoriAlignment(HoriAlignment::LEFT);
        //            textbox->SetVertAlignment(VertAlignment::BOTTOM);
        //
        //            cell5->CreateTextbox("Number 5!");
        //        }
    }

    void GameUiFactory::CreateInventoryWindow(GameUIEngine* engine, Vector2 pos, float w, float h)
    {
        // engine->CreateWindow(pos, w, h);
    }
} // namespace sage