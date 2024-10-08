//
// Created by Steve Wheeler on 03/10/2024.
//

#include "GameUiFactory.hpp"
#include "components/Ability.hpp"
#include "GameUiEngine.hpp"
#include "ResourceManager.hpp"
#include "systems/PlayerAbilitySystem.hpp"

namespace sage
{

    void GameUiFactory::CreateExampleWindow(GameUIEngine* engine)
    {
        // TODO: Have a "CreateTitleBar" with a textbox and a image of a cross for closing
        // When the title bar is clicked, you can have it get the parent's window pos and update it based on mouse
        // location (and call update children)

        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        //        info1 = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        //        info2 = {Rectangle{0.0f, 128.0f, 64.0f, 64.0f}, 16, 16, 16, 16, NPATCH_NINE_PATCH};
        //        info3 = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};

        auto window =
            engine->CreateWindowDocked(nPatchTexture, 0, 0, 40, 20, WindowTableAlignment::STACK_VERTICAL);
        window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPaddingPercent({2, 2, 2, 2});

        auto table = window->CreateTable();

        auto row0 = table->CreateTableRow();
        auto cell0 = row0->CreateTableCell(95);
        cell0->CreateTitleBar("Title Bar", 12);
        auto cell01 = row0->CreateTableCell();
        cell01->CreateCloseButton(LoadImage("resources/icon.png"));

        auto row = table->CreateTableRow(75);
        auto cell = row->CreateTableCell(50);
        cell->CreateTextbox(
            "This is a word wrap test with significantly long words.", 24, TextBox::OverflowBehaviour::WORD_WRAP);

        cell->SetPaddingPercent({2, 2, 2, 2});

        cell->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        cell->tex = window->tex;

        auto cell2 = row->CreateTableCell();
        auto cell3 = row->CreateTableCell();
        cell3->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        cell3->tex = window->tex;
        auto imagebox = cell2->CreateImagebox(LoadImage("resources/icon.png"));
        imagebox->SetGrayscale();
        // cell2->SetPaddingPercent({10, 10, 10, 10});
        // imagebox->SetHoriAlignment(HoriAlignment::CENTER);
        auto image2 = cell3->CreateImagebox(LoadImage("resources/test.png"));
        image2->SetGrayscale();
        image2->SetHoriAlignment(HoriAlignment::CENTER);

        // auto cell0 = row->CreateTableCell();

        // cell->padding.left = 10;
        // cell2->CreateTextbox("Number 2!");

        // cell3->CreateTextbox("Number 3!");
        cell3->SetPaddingPercent({0, 0, 5, 0});

        auto row2 = table->CreateTableRow();
        auto cell4 = row2->CreateTableCell();
        auto cell5 = row2->CreateTableCell();
        auto textbox2 = cell4->CreateTextbox("Bottom Left Alignment");
        textbox2->SetVertAlignment(VertAlignment::BOTTOM);
        textbox2->SetHoriAlignment(HoriAlignment::LEFT);

        cell5->CreateTextbox("This is an example of shrinking!", 42);
    }

    void GameUiFactory::CreateAbilityRow(GameUIEngine* engine, PlayerAbilitySystem* playerAbilitySystem)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        {
            // TODO: Bug: Hover does not disappear when you go directly from one image to another
            // TODO: Bug: Only one image gets scaled down to fit in the row. If you want all images to be the same
            // scale then it doesn't work as intended
            auto window =
                engine->CreateWindowDocked(nPatchTexture, 0, 0, 40, 20, WindowTableAlignment::STACK_VERTICAL);
            window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
            window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
            window->SetPaddingPercent({5, 2, 2, 2});

            auto table = window->CreateTable();

            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            auto slot = cell->CreateAbilitySlot(playerAbilitySystem, 0);
            // cell->SetPaddingPercent({2, 2, 2, 2});
            auto cell1 = row->CreateTableCell();
            auto slot1 = cell1->CreateAbilitySlot(playerAbilitySystem, 1);
            // cell1->SetPaddingPercent({2, 2, 2, 2});
            auto cell2 = row->CreateTableCell();
            auto slot2 = cell2->CreateAbilitySlot(playerAbilitySystem, 2);
            // cell2->SetPaddingPercent({2, 2, 2, 2});
            auto cell3 = row->CreateTableCell();
            auto slot3 = cell3->CreateAbilitySlot(playerAbilitySystem, 3);
            // cell3->SetPaddingPercent({2, 2, 2, 2});

            {
                entt::sink sink{slot->onMouseClicked};
                sink.connect<&PlayerAbilitySystem::AbilityOnePressed>(playerAbilitySystem);
                entt::sink sink1{slot1->onMouseClicked};
                sink1.connect<&PlayerAbilitySystem::AbilityTwoPressed>(playerAbilitySystem);
                entt::sink sink2{slot2->onMouseClicked};
                sink2.connect<&PlayerAbilitySystem::AbilityThreePressed>(playerAbilitySystem);
                entt::sink sink3{slot3->onMouseClicked};
                sink3.connect<&PlayerAbilitySystem::AbilityFourPressed>(playerAbilitySystem);
            }
        }

        {
            auto window =
                engine->CreateWindow(nPatchTexture, 300, 300, 15, 10, WindowTableAlignment::STACK_VERTICAL);
            window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
            window->SetPaddingPercent({10, 2, 5, 5});
            auto table = window->CreateTable();

            {
                auto row = table->CreateTableRow(20);
                auto cell = row->CreateTableCell(80);
                auto cell2 = row->CreateTableCell(20);
                cell->CreateTitleBar("Floating Window", 12);
                auto closeButton = cell2->CreateCloseButton(LoadImage("resources/icon.png"));
                closeButton->SetHoriAlignment(HoriAlignment::RIGHT);
                closeButton->SetVertAlignment(VertAlignment::TOP);
            }
            {
                auto row = table->CreateTableRow();
                row->SetPaddingPixel({15, 0, 0, 0});
                auto cell = row->CreateTableCell();
                cell->CreateTextbox("Example text here.", 10, TextBox::OverflowBehaviour::WORD_WRAP);
            }
        }
    }

    Window* GameUiFactory::CreateAbilityToolTip(GameUIEngine* engine, Ability& ability, Vector2 pos)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto* window =
            engine->CreateWindow(nPatchTexture, pos.x, pos.y, 15, 10, WindowTableAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPaddingPercent({10, 2, 5, 5});

        {
            auto table = window->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            auto textbox = cell0->CreateTextbox(ability.name, 10, TextBox::OverflowBehaviour::WORD_WRAP);
            textbox->SetVertAlignment(VertAlignment::BOTTOM);
            auto row = table->CreateTableRow();
            row->SetPaddingPercent({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            cell->CreateTextbox(ability.description, 10, TextBox::OverflowBehaviour::WORD_WRAP);
        }

        return window;
    }

    void GameUiFactory::CreateInventoryWindow(GameUIEngine* engine, Vector2 pos, float w, float h)
    {
    }
} // namespace sage