#include "Editor.hpp"
#include <iostream>

int main()
{
    std::cout << "Editor main!" << std::endl;
    sage::Editor::GetInstance().Update();
    return 0;
}