//
// Created by steve on 22/02/2024.
//

#include "Editor.hpp"

#include <iostream>

namespace sage
{
    void Editor::OnCursorClick()
    {
        std::cout << "Cursor has been clicked. \n";
    //    if (colSystem.GetComponent(rayCollisionResultInfo.collidedEntityId).collisionLayer == FLOOR)
    //    {
    //        // Place model
    //    }
    //    else
    //    {
    //        // Select model
    //        // Store entityID of selected model
    //        // Change bounding box colour
    //    }
    }

    void Editor::OnCollisionHit()
    {
        //std::cout << "Collision detected. \n";
        //    if (colSystem.GetComponent(rayCollisionResultInfo.collidedEntityId).collisionLayer == FLOOR)
        //    {
        //        // Place model
        //    }
        //    else
        //    {
        //        // Select model
        //        // Store entityID of selected model
        //        // Change bounding box colour
        //    }
    }
}
