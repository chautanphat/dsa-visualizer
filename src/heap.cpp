#include "heap.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include "code_viewer.h"
#include <vector>
#include <string.h>
#include <sstream>
#include <fstream>

const float startX = 1000.0f;
const float startY = 100.0f;
const float delta_x = 512.0f;
const float delta_y = 128.0f;

// std::vector<std::string> pushCode =
// {
//     "void Push(int val):",                   // 0
//     "   arr.push_back(val)",                  // 1 (Thêm vào cuối)
//     "   cur = sz - 1",                        // 2
//     "   while (cur > 0):",                    // 3
//     "       p = (cur - 1) / 2",                 // 4 (Tìm nút cha)
//     "       if (arr[cur] < arr[p]):",           // 5 (So sánh - Giả sử Min Heap)
//     "           swap(arr[cur], arr[p])",          // 6 (Hoán đổi)
//     "           cur = p",                         // 7 (Cập nhật vị trí)
//     "       else: break"                        // 8 (Dừng lại)
// };

static Heap myHeap;

Heap::Node::Node(int val, float _x, float _y, float _delta_x, Node* _parent) : value(val), x(_x), y(_y), delta_x(_delta_x), parent(_parent), left(nullptr), right(nullptr) {}

Heap::Heap() : arr(31, nullptr), sz(0), root(nullptr) {}
Heap::~Heap() { clear(); }

void Heap::insertNodeOnly(int value)
{
    if (sz >= 31) return;
    Node* p = (sz > 0) ? arr[(sz - 1) / 2] : nullptr;
    float dx = p ? p->delta_x : delta_x;
    float x  = p ? p->x + (sz % 2 ? -1 : 1) * dx : startX;
    float y  = p ? p->y + delta_y : startY;

    float spawnX = p ? p->x : x; 
    float spawnY = p ? p->y : y;

    if (!arr[sz])
    {
        arr[sz] = new Node(value, x, y, dx / 2, p);
        arr[sz]->vX = spawnX; 
        arr[sz]->vY = spawnY;
    } else
    {
        Node* n = arr[sz];
        n->value = value;
        n->x = x; n->y = y;
        n->vX = spawnX; n->vY = spawnY;
        n->delta_x = dx / 2; n->parent = p;
        n->left = n->right = nullptr;
    }

    if (p) (sz % 2 ? p->left : p->right) = arr[sz];
    else root = arr[0];

    sz++;
}

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

void Heap::removeLastNodeOnly()
{
    std::swap(arr[0]->value, arr[sz - 1]->value);
    Node* lastNode = arr[sz - 1];
    Node* p = lastNode->parent;
    if (p != nullptr)
    {
        if (p->left == lastNode) p->left = nullptr;
        else if (p->right == lastNode) p->right = nullptr;
    } else root = nullptr;
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
    return root->value;
}

void Heap::clear()
{
    sz = 0;
    for (int i = 0; i < 31; i++) 
        if (arr[i]) 
        {
            delete arr[i];
            arr[i] = nullptr;
        }
    root = nullptr;
}

static void DrawForwardButton(float x, float y, Heap& heap)
{
    GuiSetState(STATE_NORMAL);
    if (heap.mode != 1 || heap.animMode == 0) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "Forward >")) 
    {
        heap.animSpeed = 0.0f; 
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawBackwardButton(float x, float y, Heap& heap)
{
    if (heap.mode != 1 || heap.history.empty()) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "< Backward")) 
    {
        Heap::Snapshot lastState = heap.history.back();
        heap.history.pop_back();
        heap.restoreSnapshot(lastState);
        
        if (heap.mode == 1) heap.animSpeed = 999999.0f;
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

        int n = GetRandomValue(5, 30); 

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

    if (GuiTextBox((Rectangle){ x, y + 145, 300, 30 }, inputBuf, 2048, editMode)) editMode = !editMode;
}

static void DrawUpdatePanel(float x, float y, Heap& heap, char* valBuf, bool& editModeVal)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 250 }, 1, BLACK);

    makeGuiLabel(x, y, "Insert a node");
    makeGuiLabel(x, y + 35, "Value:");
    
    if (GuiTextBox((Rectangle){ x + 110, y + 35, 70, 25 }, valBuf, 16, editModeVal)) editModeVal = !editModeVal;
    
    int curState = GuiGetState();
    if (heap.sz >= 31) GuiSetState(STATE_DISABLED);
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
    GuiSetState(curState);

    if (heap.sz == 0) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x + 155, y + 75, 145, 35 }, "Pop")) heap.startPopAnimation();

    if (GuiButton((Rectangle){ x, y + 130, 300, 35 }, "Clear")) heap.clear();
}

void Heap::startPushAnimation(int value)
{
    history.clear();
    insertNodeOnly(value);

    isMoving = true;
    moveTimer = 0.0f;
    moveIdxA = sz - 1; 
    moveIdxB = -1;

    animMode = 1;
    curIdx = sz - 1;
    targetIdx = (curIdx > 0) ? (curIdx - 1) / 2 : -1;
    animTimer = 0.0f;

    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.6f;
}

void Heap::startPopAnimation()
{
    history.clear();
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
        float t = moveTimer / moveDuration;

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
                if (moveIdxA != -1 && moveIdxB != -1) 
                {
                    std::swap(arr[moveIdxA]->value, arr[moveIdxB]->value);
                    arr[moveIdxA]->vX = arr[moveIdxA]->x; arr[moveIdxA]->vY = arr[moveIdxA]->y;
                    arr[moveIdxB]->vX = arr[moveIdxB]->x; arr[moveIdxB]->vY = arr[moveIdxB]->y;
                    curIdx = moveIdxB;
                } else if (moveIdxA != -1)
                {
                    arr[moveIdxA]->vX = arr[moveIdxA]->x; 
                    arr[moveIdxA]->vY = arr[moveIdxA]->y;
                }
                
                isMoving = false;
                moveTimer = 0.0f;; 
                targetIdx = -1;
            }
        } 
        else
        {
            if (moveIdxA != -1 && moveIdxB != -1) 
            {
                float easeOutT = 1.0f - (1.0f - t) * (1.0f - t);
                arr[moveIdxA]->vX = arr[moveIdxA]->x + (arr[moveIdxB]->x - arr[moveIdxA]->x) * easeOutT;
                arr[moveIdxA]->vY = arr[moveIdxA]->y + (arr[moveIdxB]->y - arr[moveIdxA]->y) * easeOutT;
                arr[moveIdxB]->vX = arr[moveIdxB]->x + (arr[moveIdxA]->x - arr[moveIdxB]->x) * easeOutT;
                arr[moveIdxB]->vY = arr[moveIdxB]->y + (arr[moveIdxA]->y - arr[moveIdxB]->y) * easeOutT;
            }
            else
            {
                float easeOutT = 1.0f - (1.0f - t) * (1.0f - t);
                arr[moveIdxA]->vX += (arr[moveIdxA]->x - arr[moveIdxA]->vX) * easeOutT;
                arr[moveIdxA]->vY += (arr[moveIdxA]->y - arr[moveIdxA]->vY) * easeOutT;
            }
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
        captureSnapshot();

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

void Heap::captureSnapshot()
{
    Snapshot snapshot;
    snapshot.sz = sz;
    snapshot.animMode = animMode;
    snapshot.curIdx = curIdx;
    snapshot.targetIdx = targetIdx;

    for (int i = 0; i < sz; i++) snapshot.values.push_back(arr[i]->value);
    
    history.push_back(snapshot);
}

void Heap::restoreSnapshot(const Snapshot& snapshot)
{
    sz = snapshot.sz;
    animMode = snapshot.animMode;
    curIdx = snapshot.curIdx;
    targetIdx = snapshot.targetIdx;

    for (int i = 0; i < sz; i++) arr[i]->value = snapshot.values[i];

    for (int i = 0; i < sz; i++)
    {
        arr[i]->left = (2 * i + 1 < sz) ? arr[2 * i + 1] : nullptr;
        arr[i]->right = (2 * i + 2 < sz) ? arr[2 * i + 2] : nullptr;
        if (i > 0) arr[i]->parent = arr[(i - 1) / 2];
    }
    root = (sz > 0) ? arr[0] : nullptr;
}

static void draw(Heap::Node* cur, Heap& heap)
{
    if (!cur) return;
    
    if (cur->left != nullptr)
    {
        bool isDeletingEdge = (heap.sz > 0 && heap.animMode == 5 && cur->left == heap.arr[heap.sz - 1]);
        if (!isDeletingEdge) 
        {
            bool isSpawning = (heap.isMoving && heap.moveIdxB == -1 && cur->left == heap.arr[heap.moveIdxA]);
            float endX = isSpawning ? cur->left->vX : cur->left->x;
            float endY = isSpawning ? cur->left->vY : cur->left->y;
            DrawLineEx({cur->x, cur->y}, {endX, endY}, 3.0f, BLACK);
        }
        draw(cur->left, heap);
    }
    
    if (cur->right != nullptr)
    {
        bool isDeletingEdge = (heap.sz > 0 && heap.animMode == 5 && cur->right == heap.arr[heap.sz - 1]);
        if (!isDeletingEdge) 
        {
            bool isSpawning = (heap.isMoving && heap.moveIdxB == -1 && cur->right == heap.arr[heap.moveIdxA]);
            float endX = isSpawning ? cur->right->vX : cur->right->x;
            float endY = isSpawning ? cur->right->vY : cur->right->y;
            DrawLineEx({cur->x, cur->y}, {endX, endY}, 3.0f, BLACK);
        }
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
    if (!root) return;
    updateAnimation(); 
    draw(root, *this);
}

void runHeap(AppState &currentState)
{
    static char valBuffer[16] = "10";
    static char inputBuffer[2048] = "1 2 3 4 5";

    static bool editModeValue = false;
    static bool editMode = false;

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    myHeap.drawHeap();

    float X = 60, Y = 150;
    
    DrawForwardButton(875, 800, myHeap);
    DrawBackwardButton(725, 800, myHeap);

    bool isBusy = (myHeap.animMode != 0 || myHeap.isMoving);
    if (isBusy) GuiSetState(STATE_DISABLED);

    DrawToggle(X, Y, myHeap);
    DrawInitPanel(X, Y + 125, myHeap, inputBuffer, editMode);
    DrawUpdatePanel(X, Y + 425, myHeap, valBuffer, editModeValue);

    if (!isBusy) GuiSetState(STATE_NORMAL);
}