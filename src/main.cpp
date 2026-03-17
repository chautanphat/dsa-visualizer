#define RAYGUI_IMPLEMENTATION
#include "raylib.h"
#include "raygui.h"
#include "linkedlist.h"
#include <ctime>

const int screenWidth = 1600;
const int screenHeight = 900;
const int targetFPS = 60;

int main()
{
    InitWindow(screenWidth, screenHeight, "Data Structures Visualizer");
    SetRandomSeed(time(NULL));
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    SetTargetFPS(targetFPS);

    while (!WindowShouldClose())
    {
        runLinkedList();
    }

    CloseWindow();
    
    return 0;
}