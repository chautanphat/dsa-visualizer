#include "linkedlist.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"

void runLinkedList()
{
    int inputValue = 10;
    int editValue = 7;
    bool editModeValue = false;
    bool editModeEditValue = false;

    char valBuffer[16] = "10";
    char editBuffer[16] = "7";
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            GuiGroupBox((Rectangle){ 800, 20, 500, 100 }, "Initialize Linked List");
            if (GuiButton((Rectangle){ 850, 50, 120, 30 }, "Manual"))
            {
                // Code to initialize linked list visualization
            }
            if (GuiButton((Rectangle){ 1000, 50, 120, 30 }, "Upload"))
            {
                // Code to initialize linked list visualization with random values
            }
            if (GuiButton((Rectangle){ 1150, 50, 120, 30 }, "Random"))
            {
                // Code to initialize linked list visualization with random values
            }

            GuiGroupBox((Rectangle){ 10, 125, 230, 320 }, "Linked List Operations");
            makeGuiLabel(15, 150, "GLOBAL ACTIONS");
            makeGuiLabel(15, 190, "Value:");
            if (GuiTextBox((Rectangle){ 130, 185, 70, 25 }, valBuffer, 16, editModeValue)) editModeValue = !editModeValue;
            if (GuiButton((Rectangle){ 15, 225, 185, 35 }, "Add to Tail")) { /* Logic */ }

            makeGuiLabel(15, 300, "SELECTION ACTIONS");
            makeGuiLabel(15, 335, "Edit Value:");
            if (GuiTextBox((Rectangle){ 130, 335, 70, 25 }, editBuffer, 16, editModeEditValue)) editModeEditValue = !editModeEditValue;
            if (GuiButton((Rectangle){ 15, 380, 90, 35 }, "Update")) { /* Logic */ }
            if (GuiButton((Rectangle){ 110, 380, 90, 35 }, "Delete")) { /* Logic */ }
        EndDrawing();
    }
}