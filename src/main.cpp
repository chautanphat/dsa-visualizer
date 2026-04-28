#define RAYGUI_IMPLEMENTATION
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include "menu.h"
#include "linkedlist.h"
#include "heap.h"
#include "avl.h"
#include "aa.h"
#include "mst.h"
#include "shortest_path.h"
#include <ctime>

Font regularFont;
Font monoFont;

const int screenWidth = 1600;
const int screenHeight = 900;
const int targetFPS = 60;

static AppState currentState = MENU;

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetRandomSeed(time(NULL));
    InitWindow(screenWidth, screenHeight, "Data Structures Visualizer");
    
    regularFont = LoadFontEx("fonts/Inter.ttf", 20, 0, 0);
    monoFont = LoadFontEx("fonts/JetBrainsMono.ttf", 20, 0, 0);

    GuiSetFont(regularFont);
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
                case MINIMUM_SPANNING_TREE:
                    runMST(currentState);
                    break;
                case SHORTEST_PATH:
                    runDijkstra(currentState);
                    break;
            }
            if (currentState != MENU)
            {
                if (DrawCustomButton((Rectangle){ 20, 20, 80, 30 }, "#72#Back")) currentState = MENU;
            }

        EndDrawing();
    }

    UnloadFont(regularFont);
    UnloadFont(monoFont);
    CloseWindow();
    
    return 0;
}