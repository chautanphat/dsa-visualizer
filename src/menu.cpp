#include "menu.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"

void DrawMenu(AppState &currentState)
{
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    float topY = 220;
    DrawText("DSA VISUALIZER", GetScreenWidth()/2 - 150, topY, 40, DARKBLUE);
    DrawText("Select a structure to visualize", GetScreenWidth()/2 - 130, topY + 50, 20, GRAY);

    float btnWidth = 300;
    float btnHeight = 70;
    float paddingX = 50;  // Gap between columns
    float paddingY = 50;  // Gap between rows

    float totalGridWidth = (3 * btnWidth) + (2 * paddingX);
    float startX = (GetScreenWidth() - totalGridWidth) / 2.0f;
    float startY = topY + 150;

    if (DrawCustomButton((Rectangle){ startX, startY, btnWidth, btnHeight }, "LINKED LIST"))
        currentState = LINKED_LIST;
    if (DrawCustomButton((Rectangle){ startX + (btnWidth + paddingX), startY, btnWidth, btnHeight }, "HEAP"))
        currentState = HEAP;
    if (DrawCustomButton((Rectangle){ startX + (btnWidth + paddingX) * 2, startY, btnWidth, btnHeight }, "AVL TREE"))
        currentState = AVLTREE;

    float secondRowY = startY + btnHeight + paddingY;

    if (DrawCustomButton((Rectangle){ startX, secondRowY, btnWidth, btnHeight }, "AA TREE"))
        currentState = AATREE;
    if (DrawCustomButton((Rectangle){ startX + (btnWidth + paddingX), secondRowY, btnWidth, btnHeight }, "MINIMUM SPANNING TREE"))
        currentState = MINIMUM_SPANNING_TREE;
    if (DrawCustomButton((Rectangle){ startX + (btnWidth + paddingX) * 2, secondRowY, btnWidth, btnHeight }, "SHORTEST PATH"))
        currentState = SHORTEST_PATH;
}