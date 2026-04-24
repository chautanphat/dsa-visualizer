#define RAYGUI_IMPLEMENTATION
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include "menu.h"
#include "linkedlist.h"
#include "heap.h"
#include "avl.h"
#include "aa.h"
#include <ctime>

const int screenWidth = 1600;
const int screenHeight = 900;
const int targetFPS = 60;

static AppState currentState = AATREE;
int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetRandomSeed(time(NULL));
    InitWindow(screenWidth, screenHeight, "Data Structures Visualizer");
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    SetTargetFPS(targetFPS);

    while (!WindowShouldClose())
    {   
        BeginDrawing();
            switch (currentState)
            {
                case MENU:
                    DrawMenu(currentState);
                    break;
                case LINKED_LIST:
                    runLinkedList(currentState);
                    break;
                case HEAP:
                    runHeap(currentState);
                    break;
                case AVLTREE:
                    runAVL(currentState);
                    break;
                case AATREE:
                    runAA(currentState);
                    break;
            }
            if (currentState != MENU)
            {
                if (GuiButton((Rectangle){ 20, 20, 60, 30 }, "Back")) currentState = MENU;
            }

        EndDrawing();
    }

    CloseWindow();
    
    return 0;
}