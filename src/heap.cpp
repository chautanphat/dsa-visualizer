#include "heap.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include <vector>
#include<string.h>
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

Heap::Node::Node(int val, float _x, float _y, int _level, float _delta_x, Node* _parent) : value(val), x(_x), y(_y), level(_level), delta_x(_delta_x), parent(_parent), left(nullptr), right(nullptr) {}

Heap::Heap() : sz(0), head(nullptr) {}
Heap::~Heap() { clear(); }

void Heap::push(int value)
{
    insertNodeOnly(value);
    
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

void Heap::insertNodeOnly(int value)
{
    float x = 1000, y = 100, delta_x = 512, delta_y = 128;
    int level = 0;
    Node* parent = nullptr;

    if (sz > 0)
    {
        parent = arr[(sz-1)/2];
        delta_x = parent->delta_x;
        x = parent->x + (sz % 2 == 1 ? -1 : 1) * delta_x;
        y = parent->y + delta_y;
        level = parent->level + 1;
    }

    Node* child = new Node(value, x, y, level, delta_x/2, parent);
    if (parent != nullptr)
    {
        if (sz % 2 == 1) parent->left = child;
        else parent->right = child;
    } else head = child;

    arr.push_back(child);
    sz++;
}

void Heap::removeLastNodeOnly()
{
    std::swap(arr[0]->value, arr[sz - 1]->value);
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
}

void Heap::pop()
{
    if (sz == 0) return;

    removeLastNodeOnly();

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
    arr.clear();
}

static void DrawForwardButton(float x, float y, Heap& heap)
{
    GuiSetState(STATE_NORMAL);
    if (heap.mode != 1) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 100, 30 }, "Forward >")) 
    {
        heap.animSpeed = 0.0f; 
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawToggle(float x, float y, Heap& heap)
{
    int oldMode = heap.mode;

    makeGuiLabel(x + 45, y - 30, "Animation Mode:");
    GuiToggleGroup((Rectangle){ x, y, 130, 30 }, "Run-at-once;Step-by-step", &heap.mode);

    if (heap.mode != oldMode)
    {
        if (heap.mode == 1) heap.animSpeed = 999999.0f; 
        else heap.animSpeed = 0.6f;
    }
}

static void DrawInitPanel(float x, float y, Heap& heap, char* inputBuf, bool& editMode)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 230 }, 1, BLACK);

    makeGuiLabel(x, y, "Initialize Heap");
    
    if (GuiButton((Rectangle){ x, y + 35, 145, 35 }, "Random"))
    {
        heap.clear(); 

        int n = GetRandomValue(5, 10); 

        for (int i = 0; i < n; i++)
        {
            int v = GetRandomValue(1, 99);
            heap.push(v);
        }
    }

    if (GuiButton((Rectangle){ x + 155, y + 35, 145, 35 }, "Upload"))
    { 
        
    }

    if (GuiButton((Rectangle){ x, y + 90, 300, 35 }, "Manual"))
    { 
        std::istringstream iss(inputBuf);
        int value;
        heap.clear();
        while (iss >> value && heap.sz < 31)
        {
            heap.push(value);
        }
    }

    if (GuiTextBox((Rectangle){ x, y + 145, 300, 30 }, inputBuf, 16, editMode)) editMode = !editMode;
}

static void DrawUpdatePanel(float x, float y, Heap& heap, char* valBuf, bool& editModeVal)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 250 }, 1, BLACK);

    makeGuiLabel(x, y, "Insert a node");
    makeGuiLabel(x, y + 35, "Value:");
    
    if (GuiTextBox((Rectangle){ x + 110, y + 35, 70, 25 }, valBuf, 16, editModeVal)) editModeVal = !editModeVal;
    
    if (GuiButton((Rectangle){ x, y + 75, 145, 35 }, "Insert"))
    { 
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value && heap.sz < 31)
        {
            heap.startPushAnimation(value);
            strcpy(valBuf, TextFormat("%d", GetRandomValue(1, 99)));
        }
    }

    if (GuiButton((Rectangle){ x + 155, y + 75, 145, 35 }, "Pop")) heap.startPopAnimation();

    if (GuiButton((Rectangle){ x, y + 130, 300, 35 }, "Clear")) heap.clear();

}

void Heap::startPushAnimation(int value)
{
    insertNodeOnly(value);
    animMode = 1;
    curIdx = sz - 1;
    targetIdx = (curIdx > 0) ? (curIdx - 1) / 2 : -1;
    animTimer = 0.0f;

    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.6f;
}

void Heap::startPopAnimation()
{
    if (sz <= 0 || animMode != 0 || isMoving) return;

    if (sz == 1)
    { 
        pop();
        return; 
    }

    animMode = 3;
    curIdx = 0; 
    targetIdx = -1; 
    animTimer = 0.0f;

    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.6f;
}

void Heap::updateAnimation() 
{
    if (animMode == 0) return;

    if (isMoving) 
    {
        moveTimer += GetFrameTime();
        float safeDuration = (moveDuration > 0.0f) ? moveDuration : 0.4f; 
        float t = moveTimer / safeDuration;

        if (t >= 1.0f) 
        {
            if (animMode == 5)
            {
                removeLastNodeOnly(); 
                
                if (sz > 0)
                {
                    arr[0]->vX = arr[0]->x; 
                    arr[0]->vY = arr[0]->y;
                }

                isMoving = false;
                moveTimer = 0.0f;
                
                animMode = 6; 
                animTimer = 0.0f;
            } 
            else 
            {
                std::swap(arr[moveIdxA]->value, arr[moveIdxB]->value);
                
                arr[moveIdxA]->vX = arr[moveIdxA]->x; arr[moveIdxA]->vY = arr[moveIdxA]->y;
                arr[moveIdxB]->vX = arr[moveIdxB]->x; arr[moveIdxB]->vY = arr[moveIdxB]->y;

                isMoving = false;
                moveTimer = 0.0f;
                curIdx = moveIdxB; 
                targetIdx = -1;
            }
        } 
        else 
        {
            arr[moveIdxA]->vX = arr[moveIdxA]->x + (arr[moveIdxB]->x - arr[moveIdxA]->x) * t;
            arr[moveIdxA]->vY = arr[moveIdxA]->y + (arr[moveIdxB]->y - arr[moveIdxA]->y) * t;
            
            arr[moveIdxB]->vX = arr[moveIdxB]->x + (arr[moveIdxA]->x - arr[moveIdxB]->x) * t;
            arr[moveIdxB]->vY = arr[moveIdxB]->y + (arr[moveIdxA]->y - arr[moveIdxB]->y) * t;
        }
        return;
    }

    if (targetIdx == -1) 
    {
        if (animMode == 1 && curIdx > 0) targetIdx = (curIdx - 1) / 2;
        else if (animMode == 2) 
        {
            int left = curIdx * 2 + 1, right = curIdx * 2 + 2, largest = curIdx;
            if (left < sz && arr[left]->value > arr[largest]->value) largest = left;
            if (right < sz && arr[right]->value > arr[largest]->value) largest = right;
            if (largest != curIdx) targetIdx = largest;
        }
    }

    animTimer += GetFrameTime();
    float safeSpeed = (animSpeed >= 0.0f) ? animSpeed : 0.6f;

    if (animTimer >= safeSpeed) 
    {
        animTimer = 0.0f;

        if (mode == 1) animSpeed = 999999.0f; 

        if (animMode == 3) animMode = 4;
        else if (animMode == 4)
        {
            animMode = 5;
            isMoving = true;
            moveIdxA = sz - 1;
            moveIdxB = 0;
            curIdx = sz - 1;
            targetIdx = 0;
        }
        else if (animMode == 6)
        {
            animMode = 2; 
            curIdx = 0;
            targetIdx = -1;
        }
        else if (animMode == 1)
        {
            if (curIdx > 0 && targetIdx != -1) 
            {
                if (arr[curIdx]->value > arr[targetIdx]->value)
                {
                    isMoving = true;
                    moveIdxA = curIdx;
                    moveIdxB = targetIdx;
                } else animMode = 0,targetIdx = -1;
            } else animMode = 0, targetIdx = -1;
        }
        else if (animMode == 2)
        {
            if (targetIdx != -1 && arr[targetIdx]->value > arr[curIdx]->value) 
            {
                isMoving = true;
                moveIdxA = curIdx;
                moveIdxB = targetIdx;
            } 
            else animMode = 0, targetIdx = -1;
        }
    }
}

static void draw(Heap::Node* cur, Heap& heap)
{
    if (!cur) return;
    
    if (cur->left != nullptr)
    {
        bool isMovingEdge = (heap.animMode == 5 && cur->left == heap.arr[heap.sz - 1]);
        if (!isMovingEdge) DrawLineEx({cur->x, cur->y}, {cur->left->x, cur->left->y}, 3.0f, BLACK);
        draw(cur->left, heap);
    }
    
    if (cur->right != nullptr)
    {
        bool isMovingEdge = (heap.animMode == 5 && cur->right == heap.arr[heap.sz - 1]);
        if (!isMovingEdge) DrawLineEx({cur->x, cur->y}, {cur->right->x, cur->right->y}, 3.0f, BLACK);
        draw(cur->right, heap);
    }

    int myIdx = -1;
    for (int i = 0; i < heap.sz; ++i)
        if (heap.arr[i] == cur) { myIdx = i; break; }

    if (!heap.isMoving || (myIdx != heap.moveIdxA && myIdx != heap.moveIdxB)) cur->vX = cur->x, cur->vY = cur->y;
    Vector2 drawPos = {cur->vX, cur->vY};

    bool isRootHidden = (heap.animMode == 4 || heap.animMode == 5) && (myIdx == 0);

    if (!isRootHidden) 
    {
        if (heap.animMode != 0)
        {
            if (heap.animMode == 3 && myIdx == 0) DrawCircleV(drawPos, 35, RED);
            else if (heap.animMode == 5 && myIdx == heap.moveIdxA) DrawCircleV(drawPos, 35, RED);
            else if (myIdx == heap.curIdx) DrawCircleV(drawPos, 35, ORANGE);
            else if (myIdx == heap.targetIdx) DrawCircleV(drawPos, 35, RED);
        }
        DrawNode(drawPos, cur->value, 20, 30, 4);
    }
}

void Heap::drawHeap()
{
    if (!head) return;
    updateAnimation(); 
    draw(head, *this);
}

void runHeap(AppState &currentState)
{
    static char valBuffer[16] = "10";
    static char inputBuffer[256] = "1 2 3 4 5";

    static bool editModeValue = false;
    static bool editMode = false;

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    myHeap.drawHeap();

    float X = 60, Y = 150;
    
    DrawForwardButton(800, 800, myHeap);

    bool isBusy = (myHeap.animMode != 0 || myHeap.isMoving);
    if (isBusy) GuiSetState(STATE_DISABLED);

    DrawToggle(X, Y, myHeap);
    DrawInitPanel(X, Y + 125, myHeap, inputBuffer, editMode);
    DrawUpdatePanel(X, Y + 425, myHeap, valBuffer, editModeValue);
    
    if (!isBusy) GuiSetState(STATE_NORMAL);
}