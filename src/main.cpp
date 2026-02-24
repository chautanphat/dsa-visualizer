#define RAYGUI_IMPLEMENTATION
#include "raylib.h"
#include "raygui.h"
#include "linkedlist.h"

const int screenWidth = 1400;
const int screenHeight = 1000;

int main()
{
    InitWindow(screenWidth, screenHeight, "Data Structures Visualizer");
    SetTargetFPS(60);

    RunLinkedList();

    CloseWindow();
    
    return 0;
}