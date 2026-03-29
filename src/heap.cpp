#include "heap.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include <vector>
#include <sstream>
#include <fstream>

static Heap myHeap;

static void deleteTree(Heap::Node* &cur)
{
    if (cur == nullptr) return;
    deleteTree(cur->left);
    deleteTree(cur->right);
    delete cur;
    cur = nullptr;
}

Heap::Node::Node(int val, float _x, float _y, int _level, float _delta_x, Node* _parent) : value(val), x(_x), y(_y), level(_level), delta_x(_delta_x), parent(_parent), left(nullptr), right(nullptr), box({0, 0, 0, 0}) {}

Heap::Heap() : sz(0), head(nullptr) {}
Heap::~Heap() { clear(); }

void Heap::push(int value)
{
    float x = 950, y = 100, delta_x = 512, delta_y = 128;
    int level = 0;
    Node* parent = nullptr;

    if (sz > 0)
    {
        parent = arr[(sz-1)/2];
        delta_x = parent->delta_x;
        x = parent->x + (sz % 2 == 1 ? -1 : 1)*delta_x;
        y = parent->y+delta_y;
        level = parent->level+1;

    }

    Node* child = new Node(value, x, y, level, delta_x/2, parent);
    if (parent != nullptr)
    {
        if (sz % 2 == 1) parent->left = child;
        else parent->right = child;
    } else head = child;

    arr.push_back(child);
    sz++;

    int cur = sz - 1;
    while (cur > 0)
    {
        int parentId = (cur-1)/2;
        if (arr[cur]->value > arr[parentId]->value)
        {
            std::swap(arr[cur]->value, arr[parentId]->value);
            cur = parentId;
        } else break;
    }
}

void Heap::pop()
{
    if (sz == 0) return;
    std::swap(arr[0]->value, arr[sz-1]->value);

    Node* lastNode = arr[sz - 1];
    Node* p = lastNode->parent;
    if (p != nullptr)
    {
        if (p->left == lastNode) p->left = nullptr;
        else if (p->right == lastNode) p->right = nullptr;
    } else head = nullptr;

    delete lastNode;
    arr.pop_back();
    sz--;

    if (sz <= 1) return;

    int cur = 0;
    while (cur < sz)
    {
        int leftId = cur*2 + 1;
        int rightId = cur*2 + 2;
        int largestId = cur;
        if (leftId < sz && arr[leftId]->value > arr[largestId]->value) largestId = leftId;
        if (rightId < sz && arr[rightId]->value > arr[largestId]->value) largestId = rightId;
        if (largestId == cur) break;
        std::swap(arr[cur]->value, arr[largestId]->value);
        cur = largestId;
    }
}

int Heap::top()
{
    if (sz == 0) return -1;
    return head->value;
}

void Heap::clear()
{
    sz = 0;
    deleteTree(head);
}

static void draw(Heap::Node* cur)
{
    if (!cur) return;
    
    if (cur->left != nullptr)
    {
        DrawLineEx((Vector2){cur->x, cur->y}, (Vector2){cur->left->x, cur->left->y}, 3., BLACK);
        draw(cur->left);
    }
    if (cur->right != nullptr)
    {
        DrawLineEx((Vector2){cur->x, cur->y}, (Vector2){cur->right->x, cur->right->y}, 3., BLACK);
        draw(cur->right);
    }
    DrawNode((Vector2){cur->x, cur->y}, cur->value);
}

void Heap::drawHeap()
{
    draw(head);
}

static void DrawUpdatePanel(float x, float y, Heap& heap, char* valBuf, bool& editModeVal)
{
    makeGuiLabel(x, y, "Insert a node");
    makeGuiLabel(x, y + 35, "Value:");
    
    if (GuiTextBox((Rectangle){ x + 110, y + 35, 70, 25 }, valBuf, 16, editModeVal))
    {
        editModeVal = !editModeVal;
    }
    
    if (GuiButton((Rectangle){ x, y + 75, 140, 35 }, "Insert"))
    { 
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value) heap.push(value);
    }

    if (GuiButton((Rectangle){ x + 150, y + 75, 140, 35 }, "Pop"))
    { 
        heap.pop();
    }
}

void runHeap(AppState &currentState)
{
    static char valBuffer[16] = "10";
    // static char indexBuffer[16] = "7";
    // static char valSearchBuffer[16] = "7";
    // static char inputBuffer[256] = "1 2 3 4 5";
    // static char updateValBuffer[16] = "5";

    static bool editModeValue = false;
    // static bool editModeIndex = false;
    // static bool editMode = false;
    // static bool valSearchEditMode = false;
    // static bool updateValEditMode = false;

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    myHeap.drawHeap();

    float opsX = 20, opsY = 250;
    DrawRectangleLinesEx((Rectangle){ opsX, opsY, 330, 450 }, 1, BLACK);
    DrawUpdatePanel(opsX + 20, opsY + 25, myHeap, valBuffer, editModeValue);
    // DrawPopPanel()
}