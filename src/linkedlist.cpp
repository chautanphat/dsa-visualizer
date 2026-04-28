#include "linkedlist.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "../vendor/tinyfiledialogs.h"

static LinkedList myAppList;
static CodePanel listCodePanel;

static const std::vector<std::string> listAddTailCode =
{
    "if head == null: head = new Node(val)",
    "temp = head",
    "while temp.next != null:",
    "    temp = temp.next",
    "temp.next = new Node(val)"
};
static const std::vector<std::string> listSearchCode =
{
    "temp = head",
    "while temp != null:",
    "    if temp.value == target: return temp",
    "    temp = temp.next",
    "return null"
};
static const std::vector<std::string> listUpdateCode =
{
    "temp = head; i = 0",
    "while temp != null and i < index:",
    "    temp = temp.next; i++",
    "if temp != null: temp.value = newValue"
};
static const std::vector<std::string> listDeleteCode =
{
    "if head == null: return",
    "if index == 0: head = head.next; return",
    "temp = head; i = 0",
    "while temp != null and i < index:",
    "    temp = temp.next; i++",
    "if temp != null: delete temp"
};
static const std::vector<std::string>* listCurrentCode = &listAddTailCode;
static std::string listCurrentCodeTitle = "Linked List Add Tail";

LinkedList::Node::Node(int val) : value(val), next(nullptr), box({0, 0, 0, 0}) {}

LinkedList::LinkedList() : head(nullptr), sz(0) {}
LinkedList::~LinkedList() { clear(); }

void LinkedList::addToHead(int value)
{
    Node* newNode = new Node(value);
    newNode->next = head;
    head = newNode;
    sz++;
    activeLine = -1;
}

void LinkedList::addToTail(int value)
{
    Node* newNode = new Node(value);
    if (!head)
    {
        head = newNode;
        sz++;
        activeLine = -1;
        return;
    }
    Node* temp = head;
    while (temp->next) temp = temp->next;
    temp->next = newNode;
    sz++;
    activeLine = -1;
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
        sz--;
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
        sz--;
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
    sz = 0;
    activeLine = -1;
}

void LinkedList::captureSnapshot()
{
    Snapshot sn;
    sn.animMode = animMode;
    sn.targetValue = targetValue;
    sn.targetIndex = targetIndex;
    sn.currentIndex = currentIndex;
    sn.activeLine = activeLine;

    int ptrIdx = -1;
    int searchIdx = -1;
    Node* curr = head;
    int idx = 0;
    while (curr)
    {
        sn.values.push_back(curr->value);
        if (curr == animPtr) ptrIdx = idx;
        if (curr == searchResult) searchIdx = idx;
        curr = curr->next;
        idx++;
    }
    sn.animPtrIdx = ptrIdx;
    sn.searchResultIdx = searchIdx;
    history.push_back(sn);
}

void LinkedList::restoreSnapshot(const Snapshot& sn)
{
    clear();
    animMode = sn.animMode;
    targetValue = sn.targetValue;
    targetIndex = sn.targetIndex;
    currentIndex = sn.currentIndex;
    activeLine = sn.activeLine;

    for (int val : sn.values) addToTail(val);

    animPtr = nullptr;
    searchResult = nullptr;

    Node* curr = head;
    int idx = 0;
    while (curr)
    {
        if (idx == sn.animPtrIdx) animPtr = curr;
        if (idx == sn.searchResultIdx) searchResult = curr;
        curr = curr->next;
        idx++;
    }
    animTimer = 0.0f;
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
        if (mode == 1) {
            pauseTime = animSpeed;
            slideTime = 0.0f;
        }
        float progress = 0.0f;

        animTimer += GetFrameTime(); 
        
        if (animTimer > pauseTime)
        {
            if (slideTime > 0.0f)
            {
                progress = (animTimer - pauseTime) / slideTime;
                if (progress > 1.0f) progress = 1.0f; 
            } else progress = 1.0f;
        }

        if (animMode == 1) {
            if (progress > 0.0f) activeLine = 3; else activeLine = 2;
        } else if (animMode == 2) {
            if (animPtr->value == targetValue) activeLine = 2;
            else if (progress > 0.0f) activeLine = 3; 
            else activeLine = 1;
        } else if (animMode == 3) {
            if (currentIndex == targetIndex) activeLine = 3;
            else if (progress > 0.0f) activeLine = 2;
            else activeLine = 1;
        } else if (animMode == 4) {
            if (currentIndex == targetIndex) {
                if (targetIndex == 0) activeLine = 1;
                else activeLine = 5;
            }
            else if (progress > 0.0f) activeLine = 4;
            else activeLine = 3;
        }

        float currentX = animPtr->box.x;
        float currentY = animPtr->box.y;
        float slidingX = currentX + (offsetX * progress);
        
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
        if (mode == 1)
        {
            pauseTime = animSpeed;
            slideTime = 0.0f;
        }
        float totalStepTime = pauseTime + slideTime;

        if (animMode == 2 && animPtr->value == targetValue)
        {
            if (animTimer >= pauseTime) 
            {
                captureSnapshot();
                searchResult = animPtr;
                animMode = 0;
                activeLine = 2;
                animPtr = nullptr;
                if (mode == 1) animSpeed = 999999.0f;
            }
        }
        else if ((animMode == 3 || animMode == 4) && currentIndex == targetIndex)
        {
            if (animTimer >= pauseTime) 
            {
                captureSnapshot();
                if (animMode == 3) {
                    animPtr->value = targetValue;
                    activeLine = 3;
                } else if (animMode == 4) {
                    deleteNode(targetIndex);
                    activeLine = (targetIndex == 0) ? 1 : 5;
                }
                animMode = 0;
                animPtr = nullptr;
                if (mode == 1) animSpeed = 999999.0f;
            }
        }
        else if (animTimer >= totalStepTime)
        {
            captureSnapshot();
            animTimer = 0.0f;
            if (mode == 1) animSpeed = 999999.0f;

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
                    sz++;
                    activeLine = 4;
                } else if (animMode == 2) {
                    activeLine = 4;
                }
                animMode = 0;
                animPtr = nullptr; 
            }
        }
    }
}

void LinkedList::startAddTailAnimation(int value)
{
    listCurrentCode = &listAddTailCode;
    listCurrentCodeTitle = "Linked List Add Tail";
    if (!head) { addToHead(value); activeLine = 0; return; }
    
    history.clear();
    animMode = 1;
    animPtr = head;
    targetValue = value;
    animTimer = 0.0f;
    searchResult = nullptr;
    activeLine = 1;
    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.5f;
}

void LinkedList::startSearchAnimation(int value)
{
    listCurrentCode = &listSearchCode;
    listCurrentCodeTitle = "Linked List Search";
    if (!head) return;
    
    history.clear();
    animMode = 2;
    animPtr = head;
    targetValue = value;
    animTimer = 0.0f;
    searchResult = nullptr;
    activeLine = 0;
    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.5f;
}

void LinkedList::startUpdateAnimation(int index, int value)
{
    listCurrentCode = &listUpdateCode;
    listCurrentCodeTitle = "Linked List Update";
    if (!head || index < 0) return;
    
    history.clear();
    animMode = 3;
    animPtr = head;
    targetIndex = index;
    targetValue = value;
    currentIndex = 0;
    animTimer = 0.0f;
    searchResult = nullptr;
    activeLine = 0;
    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.5f;
}

void LinkedList::startDeleteAnimation(int index)
{
    listCurrentCode = &listDeleteCode;
    listCurrentCodeTitle = "Linked List Delete";
    if (!head || index < 0) return; 
    
    history.clear();
    animMode = 4;
    animPtr = head;
    targetIndex = index;
    currentIndex = 0;
    animTimer = 0.0f;
    searchResult = nullptr;
    activeLine = 2;
    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.5f;
}

void LinkedList::randomize()
{
    clear();
    int nodeCount = GetRandomValue(3, 8);
    for (int i = 0; i < nodeCount; ++i) addToTail(GetRandomValue(1, 100));
}

void LinkedList::fileUpload()
{
    const char* filters[] = { "*.txt" };
    const char* filepath = tinyfd_openFileDialog("Select File", "", 1, filters, "Text Files", 0);
    if (filepath)
    {
        std::ifstream file(filepath);
        if (file.is_open())
        {
            int value;
            clear();
            while (file >> value && sz < 9) addToTail(value);
            file.close();
        }
    }
}

void LinkedList::manualUpload(const std::string &input)
{
    clear();
    std::istringstream iss(input);
    int value;
    while (iss >> value && sz < 9) {
        addToTail(value);
    }
}

static void DrawToggle(float x, float y, LinkedList& list)
{
    int oldMode = list.mode;

    makeGuiLabel(x + 45, y - 30, "Animation Mode:");
    GuiToggleGroup((Rectangle){ x, y, 130, 30 }, "Run-at-once;Step-by-step", &list.mode);

    if (list.mode != oldMode)
    {
        if (list.mode == 1) list.animSpeed = 999999.0f; 
        else list.animSpeed = 0.5f;
    }
}

static void DrawForwardButton(float x, float y, LinkedList& list)
{
    GuiSetState(STATE_NORMAL);
    if (list.mode != 1 || list.animMode == 0) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "Forward >")) list.animSpeed = 0.0f; 
    GuiSetState(STATE_NORMAL);
}

static void DrawBackwardButton(float x, float y, LinkedList& list)
{
    if (list.mode != 1 || list.history.empty()) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "< Backward")) 
    {
        LinkedList::Snapshot lastState = list.history.back();
        list.history.pop_back();
        list.restoreSnapshot(lastState);
        if (list.mode == 1) list.animSpeed = 999999.0f;
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawInitPanel(float x, float y, LinkedList& list, char* inputBuf, bool& editMode)
{
    DrawRectangleLinesEx((Rectangle){ x, y, 800, 80 }, 1, BLACK);
    
    if (GuiButton((Rectangle){ x + 50, y + 30, 120, 30 }, "Random")) list.randomize();
    if (GuiButton((Rectangle){ x + 200, y + 30, 120, 30 }, "Upload")) list.fileUpload();
    if (GuiButton((Rectangle){ x + 350, y + 30, 120, 30 }, "Manual")) list.manualUpload(inputBuf);
    
    if (GuiTextBox((Rectangle){ x + 500, y + 30, 250, 30 }, inputBuf, 256, editMode))
    {
        editMode = !editMode;
    }
}

static void DrawAddPanel(float x, float y, LinkedList& list, char* valBuf, bool& editModeVal)
{
    makeGuiLabel(x, y, "Add a node");
    makeGuiLabel(x, y + 35, "Value:");

    if (GuiTextBox((Rectangle){ x + 110, y + 35, 70, 25 }, valBuf, 16, editModeVal))
    {
        editModeVal = !editModeVal;
    }
    
    int curState = GuiGetState();
    if (list.sz >= 9) {
        GuiSetState(STATE_DISABLED);
        DrawText("Maximum 9 nodes reached.", (int)x, (int)y + 112, 16, RED);
    }

    if (GuiButton((Rectangle){ x, y + 75, 140, 35 }, "Add to Head"))
    { 
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value && list.sz < 9) list.addToHead(value);
    }
    
    if (GuiButton((Rectangle){ x + 150, y + 75, 140, 35 }, "Add to Tail"))
    {
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value && list.animMode == 0 && list.sz < 9) list.startAddTailAnimation(value);
    }
    GuiSetState(curState);
    
}

static void DrawUpdatePanel(float x, float y, LinkedList& list, char* idxBuf, bool& editModeIdx, char* valBuf, bool& editModeVal)
{
    makeGuiLabel(x, y, "Update actions");
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
    
    int curState = GuiGetState();
    if (list.head == nullptr) GuiSetState(STATE_DISABLED);

    if (GuiButton((Rectangle){ x + 100, y + 75, 90, 35 }, "Delete"))
    {
        std::istringstream issIndex(idxBuf);
        int index;
        if (issIndex >> index && index >= 0 && list.animMode == 0) list.startDeleteAnimation(index);
    }
    
    if (GuiButton((Rectangle){ x + 200, y + 75, 90, 35 }, "Clear")) list.clear(); 

    GuiSetState(curState);
}

static void DrawSearchPanel(float x, float y, LinkedList& list, char* searchBuf, bool& editModeSearch)
{
    makeGuiLabel(x, y, "Search value");
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

    myAppList.drawLinkedList(430, 250);
    float X = 60, Y = 250;

    DrawForwardButton(875, 800, myAppList);
    DrawBackwardButton(725, 800, myAppList);

    DrawCodePanel(listCodePanel, code_panel, listCurrentCodeTitle, *listCurrentCode, myAppList.activeLine);

    if (myAppList.animMode == 0) GuiSetState(STATE_NORMAL);
    else GuiSetState(STATE_DISABLED);

    DrawToggle(X, Y - 100, myAppList);
    DrawInitPanel(600, 20, myAppList, inputBuffer, editMode);
    DrawRectangleLinesEx((Rectangle){ X - 20, Y, 330, 450 }, 1, BLACK);
    DrawAddPanel(X, Y + 25, myAppList, valBuffer, editModeValue);
    DrawUpdatePanel(X, Y + 150, myAppList, indexBuffer, editModeIndex, updateValBuffer, updateValEditMode);
    DrawSearchPanel(X, Y + 300, myAppList, valSearchBuffer, valSearchEditMode);

    GuiSetState(STATE_NORMAL);
}