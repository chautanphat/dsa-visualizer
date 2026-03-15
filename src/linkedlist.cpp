#include "linkedlist.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"

struct LinkedList
{
    struct Node
    {
        int value;
        Node* next;
        Node(int val) : value(val), next(nullptr) {}
    };
    Node* head;

    LinkedList() : head(nullptr) {}

    void addToHead(int value)
    {
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
    }
    
    void addToTail(int value)
    {
        Node* newNode = new Node(value);
        if (!head)
        {
            head = newNode;
            return;
        }
        Node* temp = head;
        while (temp->next) temp = temp->next;
        temp->next = newNode;
    }
};

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

            int startX = 20, startY = 200, offsetX = 10;
            GuiGroupBox((Rectangle){ startX, startY, 240, 320 }, "Linked List Operations");
            makeGuiLabel(startX + offsetX + 10, startY + 25, "ADD A NODE");
            makeGuiLabel(startX + offsetX + 10, startY + 60, "Value:");
            if (GuiTextBox((Rectangle){ startX + offsetX + 120, startY + 60, 70, 25 }, valBuffer, 16, editModeValue)) editModeValue = !editModeValue;
            if (GuiButton((Rectangle){ startX + offsetX + 10, startY + 100, 90, 35 }, "Head")) { /* Logic */ }
            if (GuiButton((Rectangle){ startX + offsetX + 110, startY + 100, 90, 35 }, "Tail")) { /* Logic */ }

            makeGuiLabel(startX + offsetX + 10, startY + 175, "SELECTION ACTIONS");
            makeGuiLabel(startX + offsetX + 10, startY + 210, "Edit Value:");
            if (GuiTextBox((Rectangle){ startX + offsetX + 120, startY + 210, 70, 25 }, editBuffer, 16, editModeEditValue)) editModeEditValue = !editModeEditValue;
            if (GuiButton((Rectangle){ startX + offsetX + 10, startY + 250, 90, 35 }, "Update")) { /* Logic */ }
            if (GuiButton((Rectangle){ startX + offsetX + 110, startY + 250, 90, 35 }, "Delete")) { /* Logic */ }
        EndDrawing();
    }
}