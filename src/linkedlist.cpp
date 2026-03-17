#include "linkedlist.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include <string>
#include <sstream>
#include <fstream>

static LinkedList myAppList;

LinkedList::Node::Node(int val) : value(val), next(nullptr), box({0, 0, 0, 0}) {}

LinkedList::LinkedList() : head(nullptr) {}
LinkedList::~LinkedList() { clear(); }

void LinkedList::addToHead(int value)
{
    Node* newNode = new Node(value);
    newNode->next = head;
    head = newNode;
}

void LinkedList::addToTail(int value)
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

void LinkedList::clear()
{
    Node* current = head;
    while (current)
    {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
}

void LinkedList::drawLinkedList(float startX, float startY)
{
    Node* cur = head;
    float offsetX = 120;
    while (cur)
    {
        cur->box = {startX, startY, 100.0f, 50.0f};
        DrawRectangleRec(cur->box, LIGHTGRAY);
        DrawRectangleLinesEx(cur->box, 2, DARKGRAY);
        DrawNumberInBox(cur->box, cur->value, 20, BLACK);

        if (cur->next)
        {
            DrawLine(startX + 100, startY + 25, startX + offsetX, startY + 25, BLACK);
            DrawTriangle((Vector2){startX + offsetX - 10, startY + 20}, (Vector2){startX + offsetX - 10, startY + 30}, (Vector2){startX + offsetX, startY + 25}, BLACK);
        }

        cur = cur->next;
        startX += offsetX;
    }
}

void LinkedList::randomize()
{
    this->clear();
    int nodeCount = GetRandomValue(3, 10);
    for (int i = 0; i < nodeCount; ++i)
    {
        this->addToTail(GetRandomValue(1, 100));
    }
}

void LinkedList::fileUpload()
{
    
}

void LinkedList::manualUpload(const std::string &input)
{
    this->clear();
    std::istringstream iss(input);
    int value;
    while (iss >> value)
    {
        this->addToTail(value);
    }
}

void runLinkedList()
{
    static char valBuffer[16] = "10";
    static char editBuffer[16] = "7";
    static char inputBuffer[256] = "1 2 3 4 5";

    bool editModeValue = false;
    bool editModeEditValue = false;
    bool editMode = false;

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            myAppList.drawLinkedList(400, 250);

            float startX = 600, startY = 20, offsetX = 10;

            GuiGroupBox((Rectangle){ startX, startY, 800, 80 }, "Initialize Linked List");
            if (GuiButton((Rectangle){ startX + 50, startY + 30, 120, 30 }, "Random"))
            {
                myAppList.randomize();
            }
            if (GuiButton((Rectangle){ startX + 200, startY + 30, 120, 30 }, "Upload"))
            {
                myAppList.fileUpload();
            }
            if (GuiButton((Rectangle){ startX + 350, startY + 30, 120, 30 }, "Manual"))
            {
                myAppList.manualUpload(inputBuffer);
            }
            
            if (GuiTextBox((Rectangle){ startX + 500, startY + 30, 250, 30 }, inputBuffer, 256, editMode)) editMode = !editMode;

            startX = 20, startY = 250, offsetX = 10;
            GuiGroupBox((Rectangle){ startX, startY, 330, 320 }, "Linked List Operations");
            makeGuiLabel(startX + offsetX + 10, startY + 25, "ADD A NODE");
            makeGuiLabel(startX + offsetX + 10, startY + 60, "Value:");
            if (GuiTextBox((Rectangle){ startX + offsetX + 120, startY + 60, 70, 25 }, valBuffer, 16, editModeValue)) editModeValue = !editModeValue;
            if (GuiButton((Rectangle){ startX + offsetX + 10, startY + 100, 140, 35 }, "Add to Head"))
            { 
                std::istringstream iss(valBuffer);
                int value;
                if (iss >> value) myAppList.addToHead(value);
            }
            if (GuiButton((Rectangle){ startX + offsetX + 160, startY + 100, 140, 35 }, "Add to Tail"))
            {
                std::istringstream iss(valBuffer);
                int value;
                if (iss >> value) myAppList.addToTail(value);
            }

            makeGuiLabel(startX + offsetX + 10, startY + 175, "SELECTION ACTIONS");
            makeGuiLabel(startX + offsetX + 10, startY + 210, "Edit Value:");
            if (GuiTextBox((Rectangle){ startX + offsetX + 120, startY + 210, 70, 25 }, editBuffer, 16, editModeEditValue)) editModeEditValue = !editModeEditValue;
            if (GuiButton((Rectangle){ startX + offsetX + 10, startY + 250, 90, 35 }, "Update")) { /* Logic */ }
            if (GuiButton((Rectangle){ startX + offsetX + 110, startY + 250, 90, 35 }, "Delete")) { /* Logic */ }
            if (GuiButton((Rectangle){ startX + offsetX + 210, startY + 250, 90, 35 }, "Clear")) { myAppList.clear(); }
        EndDrawing();
    }
}