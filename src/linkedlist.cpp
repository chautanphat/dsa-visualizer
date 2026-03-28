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

void LinkedList::update(int index, int value)
{
    Node* temp = head;
    int currentIndex = 0;
    while (temp && currentIndex < index)
    {
        temp = temp->next;
        currentIndex++;
    }
    if (temp) temp->value = value;
}

void LinkedList::deleteNode(int index)
{
    searchResult = nullptr;
    if (!head) return;
    if (index == 0)
    {
        Node* toDelete = head;
        head = head->next;
        delete toDelete;
        return;
    }
    Node* temp = head;
    int currentIndex = 0;
    while (temp->next && currentIndex < index - 1)
    {
        temp = temp->next;
        currentIndex++;
    }
    if (temp->next)
    {
        Node* toDelete = temp->next;
        temp->next = temp->next->next;
        delete toDelete;
    }
}

void LinkedList::clear()
{
    searchResult = nullptr;
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
        DrawRectangleRec(cur->box, WHITE);

        if (cur == searchResult)
            DrawRectangleLinesEx(cur->box, 5, RED);
        else
            DrawRectangleLinesEx(cur->box, 2, BLACK);

        DrawNumberInBox(cur->box, cur->value, 20, BLACK);

        if (cur->next)
        {
            DrawLine(startX + 100, startY + 25, startX + offsetX, startY + 25, BLACK);
            DrawTriangle((Vector2){startX + offsetX - 10, startY + 20}, (Vector2){startX + offsetX - 10, startY + 30}, (Vector2){startX + offsetX, startY + 25}, BLACK);
        }

        cur = cur->next;
        startX += offsetX;
    }

    if (animMode != 0 && animPtr != nullptr)
    {
        float pauseTime = 0.5f;
        float slideTime = 0.15f;
        float progress = 0.0f;

        animTimer += GetFrameTime(); 
        
        if (animTimer > pauseTime)
        {
            progress = (animTimer - pauseTime) / slideTime;
            if (progress > 1.0f) progress = 1.0f; 
        }

        float currentX = animPtr->box.x;
        float currentY = animPtr->box.y;
        float slidingX = currentX + (120.0f * progress);
        
        Rectangle windowBox = { slidingX - 5, currentY - 5, 110.0f, 60.0f };
        
        Color boxColor = ORANGE;
        if (animMode == 2 && animPtr->value == targetValue) boxColor = RED;
        if ((animMode == 3 || animMode == 4) && currentIndex == targetIndex)
            boxColor = (animMode == 3) ? GREEN : RED;

        DrawRectangleLinesEx(windowBox, 4, boxColor);
        DrawText("cur", windowBox.x + 45, windowBox.y - 20, 16, boxColor);
    }

    if (animMode != 0 && animPtr != nullptr)
    {
        float pauseTime = 0.5f;
        float slideTime = 0.15f;
        float totalStepTime = pauseTime + slideTime;

        if (animMode == 2 && animPtr->value == targetValue)
        {
            if (animTimer >= pauseTime) 
            {
                searchResult = animPtr;
                animMode = 0;
                animPtr = nullptr;
            }
        }
        else if ((animMode == 3 || animMode == 4) && currentIndex == targetIndex)
        {
            if (animTimer >= pauseTime) 
            {
                if (animMode == 3) {
                    animPtr->value = targetValue;
                } else if (animMode == 4) deleteNode(targetIndex);
                animMode = 0;
                animPtr = nullptr;
            }
        }
        else if (animTimer >= totalStepTime)
        {
            animTimer = 0.0f;

            if (animPtr->next != nullptr)
            {
                animPtr = animPtr->next;
                currentIndex++;
            }
            else
            {
                if (animMode == 1)
                {
                    Node* newNode = new Node(targetValue);
                    animPtr->next = newNode;
                }
                animMode = 0;
                animPtr = nullptr; 
            }
        }
    }
}

void LinkedList::startAddTailAnimation(int value)
{
    if (!head) { addToHead(value); return; }
    
    animMode = 1;
    animPtr = head;
    targetValue = value;
    animTimer = 0.0f;
    searchResult = nullptr;
}

void LinkedList::startSearchAnimation(int value)
{
    if (!head) return;
    
    animMode = 2;
    animPtr = head;
    targetValue = value;
    animTimer = 0.0f;
    searchResult = nullptr;
}

void LinkedList::startUpdateAnimation(int index, int value)
{
    if (!head || index < 0) return;
    
    animMode = 3;
    animPtr = head;
    targetIndex = index;
    targetValue = value;
    currentIndex = 0;
    animTimer = 0.0f;
    searchResult = nullptr;
}

void LinkedList::startDeleteAnimation(int index)
{
    if (!head || index < 0) return; 
    
    animMode = 4;
    animPtr = head;
    targetIndex = index;
    currentIndex = 0;
    animTimer = 0.0f;
    searchResult = nullptr;
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

static void DrawInitPanel(float x, float y, LinkedList& list, char* inputBuf, bool& editMode)
{
    GuiGroupBox((Rectangle){ x, y, 800, 80 }, "Initialize Linked List");
    
    if (GuiButton((Rectangle){ x + 50, y + 30, 120, 30 }, "Random")) list.randomize();
    if (GuiButton((Rectangle){ x + 200, y + 30, 120, 30 }, "Upload")) list.fileUpload();
    if (GuiButton((Rectangle){ x + 350, y + 30, 120, 30 }, "Manual")) list.manualUpload(inputBuf);
    
    if (GuiTextBox((Rectangle){ x + 500, y + 30, 250, 30 }, inputBuf, 256, editMode)) {
        editMode = !editMode;
    }
}

static void DrawAddPanel(float x, float y, LinkedList& list, char* valBuf, bool& editModeVal)
{
    makeGuiLabel(x, y, "ADD A NODE");
    makeGuiLabel(x, y + 35, "Value:");
    
    if (GuiTextBox((Rectangle){ x + 110, y + 35, 70, 25 }, valBuf, 16, editModeVal)) {
        editModeVal = !editModeVal;
    }
    
    if (GuiButton((Rectangle){ x, y + 75, 140, 35 }, "Add to Head")) { 
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value) list.addToHead(value);
    }
    
    if (GuiButton((Rectangle){ x + 150, y + 75, 140, 35 }, "Add to Tail")) {
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value && list.animMode == 0) list.startAddTailAnimation(value);
    }
}

static void DrawUpdatePanel(float x, float y, LinkedList& list, char* idxBuf, bool& editModeIdx, char* valBuf, bool& editModeVal)
{
    makeGuiLabel(x, y, "UPDATE ACTIONS");
    makeGuiLabel(x, y + 35, "Index:");
    makeGuiLabel(x + 160, y + 35, "Value:");
    
    if (GuiTextBox((Rectangle){ x + 70, y + 35, 65, 25 }, idxBuf, 16, editModeIdx)) editModeIdx = !editModeIdx;
    if (GuiTextBox((Rectangle){ x + 230, y + 35, 65, 25 }, valBuf, 16, editModeVal)) editModeVal = !editModeVal;
    
    if (GuiButton((Rectangle){ x, y + 75, 90, 35 }, "Update"))
    {
        std::istringstream issIndex(idxBuf);
        std::istringstream issValue(valBuf);
        int index, value;
        if (issIndex >> index && issValue >> value && index >= 0 && list.animMode == 0) list.startUpdateAnimation(index, value);
    }
    
    if (GuiButton((Rectangle){ x + 100, y + 75, 90, 35 }, "Delete"))
    {
        std::istringstream issIndex(idxBuf);
        int index;
        if (issIndex >> index && index >= 0 && list.animMode == 0) list.startDeleteAnimation(index);
    }
    
    if (GuiButton((Rectangle){ x + 200, y + 75, 90, 35 }, "Clear")) list.clear(); 
}

static void DrawSearchPanel(float x, float y, LinkedList& list, char* searchBuf, bool& editModeSearch)
{
    makeGuiLabel(x, y, "SEARCH VALUE");
    makeGuiLabel(x, y + 35, "Value:");
    
    if (GuiTextBox((Rectangle){ x + 110, y + 35, 70, 25 }, searchBuf, 16, editModeSearch)) {
        editModeSearch = !editModeSearch;
    }
    
    if (GuiButton((Rectangle){ x + 100, y + 75, 90, 35 }, "Search")) {
        std::istringstream issValue(searchBuf);
        int value;
        if (issValue >> value && list.animMode == 0) list.startSearchAnimation(value);
    }
}

void runLinkedList(AppState &currentState)
{
    static char valBuffer[16] = "10";
    static char indexBuffer[16] = "2";
    static char valSearchBuffer[16] = "7";
    static char inputBuffer[256] = "1 2 3 4 5";
    static char updateValBuffer[16] = "5";

    static bool editModeValue = false;
    static bool editModeIndex = false;
    static bool editMode = false;
    static bool valSearchEditMode = false;
    static bool updateValEditMode = false;
   
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    
    if (GuiButton((Rectangle){ 10, 10, 60, 30 }, "Back"))
        currentState = MENU;

    myAppList.drawLinkedList(400, 250);
    DrawInitPanel(600, 20, myAppList, inputBuffer, editMode);
    float opsX = 20, opsY = 250;
    GuiGroupBox((Rectangle){ opsX, opsY, 330, 450 }, "Linked List Operations");
    DrawAddPanel(opsX + 20, opsY + 25, myAppList, valBuffer, editModeValue);
    DrawUpdatePanel(opsX + 20, opsY + 150, myAppList, indexBuffer, editModeIndex, updateValBuffer, updateValEditMode);
    DrawSearchPanel(opsX + 20, opsY + 300, myAppList, valSearchBuffer, valSearchEditMode);
}