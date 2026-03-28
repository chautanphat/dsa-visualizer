#define RAYGUI_IMPLEMENTATION
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include "menu.h"
#include "linkedlist.h"
#include "heap.h"
#include <ctime>

const int screenWidth = 1600;
const int screenHeight = 900;
const int targetFPS = 60;

static AppState currentState = LINKED_LIST;

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
            }

        EndDrawing();
    }

    CloseWindow();
    
    return 0;
}