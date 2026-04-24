#include "avl.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include "code_viewer.h"
#include <vector>
#include <string.h>
#include <sstream>
#include <fstream>
#include <iostream>

const float x_root = 1000.0f;
const float y_root = 100.0f;
const float delta_x = 256.0f;
const float delta_y = 128.0f;

static AVL myAVL;

AVL ::Node::Node(int val, float _x, float _y, int _id, Node* _parent) : value(val), height(1), bf(0), id(_id), x(_x), y(_y), vX(_x), vY(_y), parent(_parent), left(nullptr), right(nullptr) {}

AVL::AVL() : sz(0), root(nullptr) { arr.clear(); }
AVL::~AVL() { clear(); }

void AVL::clear()
{
    sz = 0;
    for (Node* node : arr)
        if (node != nullptr) delete node;
    arr.clear();
    history.clear(); 
    root = nullptr;
}

AVL::Node* AVL::insertLogic(Node* node, int val, Node* p)
{
    if (node == nullptr)
    {
        Node* newNode = new Node(val, 0.0f, 0.0f, sz, p); 
        
        if (sz < (int)arr.size()) arr[sz] = newNode;
        else arr.push_back(newNode);
        
        sz++;
        return newNode;
    }

    if (val < node->value)
        node->left = insertLogic(node->left, val, node);
    else if (val > node->value)
        node->right = insertLogic(node->right, val, node);
    else 
        return node;

    node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
    node->bf = getHeight(node->left) - getHeight(node->right);

    int balance = node->bf;

    if (balance > 1 && val < node->left->value) return rightRotate(node);

    if (balance < -1 && val > node->right->value) return leftRotate(node);

    if (balance > 1 && val > node->left->value)
    {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    if (balance < -1 && val < node->right->value)
    {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node; 
}

void AVL::insert(int value)
{
    root = insertLogic(root, value, nullptr);
    calculatePositions(root, x_root, y_root, delta_x);
    for (Node* n : arr)
    {
        if (n != nullptr)
        {
            n->vX = n->x;
            n->vY = n->y;
        }
    }
}

int AVL::getHeight(Node* node)
{
    if (node == nullptr) return 0;
    return node->height;
}

int AVL::getBalance(Node* node)
{
    if (node == nullptr) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

AVL::Node* AVL::rightRotate(Node* y)
{
    Node* x = y->left;
    Node* T2 = x->right;
    Node* oldParent = y->parent;

    x->right = y;
    y->left = T2;

    x->parent = oldParent;
    y->parent = x;
    if (T2 != nullptr) T2->parent = y;

    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
    y->bf = getHeight(y->left) - getHeight(y->right);
    x->bf = getHeight(x->left) - getHeight(x->right);

    return x;
}

AVL::Node* AVL::leftRotate(Node* x)
{
    Node* y = x->right;
    Node* T2 = y->left;
    Node* oldParent = x->parent;

    y->left = x;
    x->right = T2;

    x->parent = y;
    y->parent = oldParent;
    if (T2 != nullptr) T2->parent = x;

    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
    x->bf = getHeight(x->left) - getHeight(x->right);
    y->bf = getHeight(y->left) - getHeight(y->right);

    return y;
}

void AVL::calculatePositions(Node* node, float currentX, float currentY, float hGap)
{
    if (node == nullptr) return;

    node->x = currentX;
    node->y = currentY;
    
    if (node->left != nullptr) calculatePositions(node->left, currentX - hGap, currentY + delta_y, hGap / 2.0f);
    if (node->right != nullptr) calculatePositions(node->right, currentX + hGap, currentY + delta_y, hGap / 2.0f);
}

static void DrawForwardButton(float x, float y, AVL& avl)
{
    GuiSetState(STATE_NORMAL);
    if (avl.mode != 1 || avl.animMode == 0) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "Forward >")) 
    {
        avl.animSpeed = 0.0f; 
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawBackwardButton(float x, float y, AVL& avl)
{
    if (avl.mode != 1 || avl.history.empty()) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "< Backward")) 
    {
        AVL::Snapshot lastState = avl.history.back();
        avl.history.pop_back();
        avl.restoreSnapshot(lastState);
        if (avl.mode == 1) avl.animSpeed = 999999.0f;
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawToggle(float x, float y, AVL& avl)
{
    int oldMode = avl.mode;

    makeGuiLabel(x + 45, y - 30, "Animation Mode:");
    GuiToggleGroup((Rectangle){ x, y, 130, 30 }, "Run-at-once;Step-by-step", &avl.mode);

    if (avl.mode != oldMode)
    {
        if (avl.mode == 1) avl.animSpeed = 999999.0f; 
        else avl.animSpeed = 0.8f;
    }
}

static void DrawInitPanel(float x, float y, AVL& avl, char* inputBuf, bool& editMode)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 230 }, 1, BLACK);

    makeGuiLabel(x, y, "Initialize AVL Tree");
    
    if (GuiButton((Rectangle){ x, y + 35, 145, 35 }, "Random"))
    {
        avl.clear(); 

        int n = GetRandomValue(5, 30); 

        for (int i = 0; i < n; i++)
        {
            int v = GetRandomValue(1, 99);
            avl.insert(v);
        }
    }

    if (GuiButton((Rectangle){ x + 155, y + 35, 145, 35 }, "Upload"))
    {
        
    }

    if (GuiButton((Rectangle){ x, y + 90, 300, 35 }, "Manual"))
    { 
        std::istringstream iss(inputBuf);
        int value;
        avl.clear();
        while (iss >> value && avl.sz < 31) avl.insert(value);
    }

    if (GuiTextBox((Rectangle){ x, y + 145, 300, 30 }, inputBuf, 2048, editMode)) editMode = !editMode;
}

static void DrawOperationPanel(float x, float y, AVL& avl, char* valBuf, bool& editModeVal)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 280 }, 1, BLACK);

    makeGuiLabel(x, y, "Operations");
    makeGuiLabel(x, y + 35, "Value:");
    
    if (GuiTextBox((Rectangle){ x + 110, y + 35, 70, 25 }, valBuf, 16, editModeVal)) editModeVal = !editModeVal;
    
    int curState = GuiGetState();
    if (avl.sz >= 31) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y + 75, 145, 35 }, "Insert"))
    { 
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value && avl.sz < 31)
        {
            avl.startInsertAnimation(value);
            strcpy(valBuf, TextFormat("%d", GetRandomValue(1, 99)));
        }
    }
    GuiSetState(curState);

    if (avl.sz == 0) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x + 155, y + 75, 145, 35 }, "Delete"))
    {
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value)
        {
            avl.startDeleteAnimation(value);
            strcpy(valBuf, TextFormat("%d", GetRandomValue(1, 99)));
        }
    }

    if (GuiButton((Rectangle){ x, y + 130, 300, 35 }, "Search"))
    {
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value)
        {
            avl.startSearchAnimation(value);
            strcpy(valBuf, TextFormat("%d", GetRandomValue(1, 99)));
        }
    }
    if (GuiButton((Rectangle){ x, y + 185, 300, 35 }, "Clear")) avl.clear();
}

void AVL::startInsertAnimation(int value)
{
    if (sz >= 31) return;
    history.clear();
    pendingValue = value; 

    if (root == nullptr) 
    {
        arr.push_back(new Node(value, x_root, y_root, sz, nullptr)); 
        root = arr[sz];
        curIdx = root->id;
        sz++;
        
        animMode = 0;
    } 
    else 
    {
        animMode = 1; 
        curIdx = root->id;
        targetIdx = -1;
        animTimer = 0.0f;
    }

    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.8f;
}

void AVL::startDeleteAnimation(int value)
{
    if (sz == 0 || root == nullptr) return;
    history.clear(); 
    pendingValue = value; 
    isMoving = false;
    moveTimer = 0.0f;

    animMode = 10;
    curIdx = root->id;
    targetIdx = -1;
    animTimer = 0.0f;

    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.8f;
}

void AVL::startSearchAnimation(int value)
{
    if (sz == 0 || root == nullptr) return;
    history.clear();
    pendingValue = value;
    isMoving = false;

    moveTimer = 0.0f;
    animTimer = 0.0f;
    animMode = 20;
    curIdx = root->id;
    targetIdx = -1;

    if (mode == 1) animSpeed = 999999.0f; 
    else animSpeed = 0.8f;
}

void AVL::updateAnimation()
{
    if (isMoving) 
    {
        moveTimer += GetFrameTime();
        float t = moveTimer / moveDuration;

        if (t >= 1.0f)
        {
            t = 1.0f;
            isMoving = false;
            moveTimer = 0.0f;
        }

        for (Node* n : arr)
        {
            if (n != nullptr)
            {
                float easeOut = 1 - (1 - t) * (1 - t);
                n->vX += (n->x - n->vX) * easeOut;
                n->vY += (n->y - n->vY) * easeOut;
            }
        }
        return;
    }

    if (animMode == 0) return;

    animTimer += GetFrameTime();
    float safeSpeed = (animSpeed >= 0.0f) ? animSpeed : 0.8f;

    if (animTimer >= safeSpeed) 
    {
        captureSnapshot();

        animTimer = 0.0f;
        if (mode == 0) animSpeed = 0.8f;
        else animSpeed = 999999.0f;

        Node* curNode = (curIdx != -1) ? arr[curIdx] : nullptr;

        if (animMode == 1)
        {
            if (curNode == nullptr) return;
            
            if (pendingValue < curNode->value)
            {
                if (curNode->left == nullptr) animMode = 2;
                else curIdx = curNode->left->id;
            } else if (pendingValue > curNode->value)
            {
                if (curNode->right == nullptr) animMode = 2;
                else curIdx = curNode->right->id;
            } else
            {
                targetIdx = curNode->id;
                animMode = 0;
                return;
            }
        }
        if (animMode == 2) 
        {
            if (sz >= 31)
            {
                animMode = 0;
                return;
            }

            float startX = curNode->vX; 
            float startY = curNode->vY;
            
            Node* newNode = new Node(pendingValue, startX, startY, sz, curNode);
            
            if (sz < (int)arr.size()) arr[sz] = newNode;
            else arr.push_back(newNode);
            
            if (pendingValue < curNode->value) curNode->left = newNode;
            else curNode->right = newNode;

            sz++;
            curIdx = newNode->id;
            
            calculatePositions(root, x_root, y_root, delta_x);
            isMoving = true; 
            
            animMode = 3;
        } else if (animMode == 3) 
        {
            if (curNode == nullptr || curNode->parent == nullptr)
            {
                animMode = 0, targetIdx = -1;
                return;
            }
            curIdx = targetIdx = curNode->parent->id;
            curNode = arr[curIdx]; 
            curNode->height = 1 + std::max(getHeight(curNode->left), getHeight(curNode->right));
            curNode->bf = getHeight(curNode->left) - getHeight(curNode->right);

            if (abs(curNode->bf) > 1) animMode = 4;
        } else if (animMode == 4)
        {
            Node* y = arr[targetIdx];
            Node* p = y->parent;
            Node* newRoot = nullptr;

            int balance = getBalance(y);

            if (balance > 1 && getBalance(y->left) >= 0) newRoot = rightRotate(y); // LL
            else if (balance < -1 && getBalance(y->right) <= 0) newRoot = leftRotate(y); // RR
            else if (balance > 1 && getBalance(y->left) < 0) // LR
            {
                y->left = leftRotate(y->left);
                newRoot = rightRotate(y);
            }
            else if (balance < -1 && getBalance(y->right) > 0) // RL
            {
                y->right = rightRotate(y->right);
                newRoot = leftRotate(y);
            }

            if (p == nullptr) root = newRoot;
            else
            {
                if (p->left == y) p->left = newRoot;
                else p->right = newRoot;
            }

            calculatePositions(root, x_root, y_root, delta_x);
            isMoving = true; 
            moveTimer = 0.0f;
            
            curIdx = targetIdx = newRoot->id;
            animMode = (curIdx != -1) ? 3 : 0; 
        } else if (animMode == 10)
        {
            if (curNode == nullptr) { animMode = 0; return; }
            
            if (pendingValue < curNode->value)
            {
                if (curNode->left) curIdx = curNode->left->id;
                else animMode = 0;
            } else if (pendingValue > curNode->value)
            {
                if (curNode->right) curIdx = curNode->right->id;
                else animMode = 0;
            } else
            { 
                targetIdx = curIdx;
                animMode = 11;
            }
        } else if (animMode == 11)
        {
            Node* delNode = arr[targetIdx];
            if (delNode->left != nullptr && delNode->right != nullptr)
            {
                curIdx = delNode->right->id; 
                animMode = 14; 
            } else
            {
                Node* child = (delNode->left != nullptr) ? delNode->left : delNode->right;
                Node* p = delNode->parent;

                if (child != nullptr) child->parent = p;

                if (p == nullptr) root = child;
                else if (p->left != nullptr && p->left->id == delNode->id) p->left = child;
                else p->right = child;

                if (child != nullptr)
                {
                    curIdx = child->id;
                    child->x = child->vX = delNode->vX;
                    child->y = child->vY = delNode->vY;
                }
                else curIdx = (p != nullptr) ? p->id : -1;
                
                targetIdx = curIdx;

                arr[delNode->id] = nullptr;
                delete delNode;

                calculatePositions(root, x_root, y_root, delta_x);
                isMoving = true;
                moveTimer = 0.0f;

                if (curIdx != -1)
                {
                    arr[curIdx]->height = 1 + std::max(getHeight(arr[curIdx]->left), getHeight(arr[curIdx]->right));
                    arr[curIdx]->bf = getHeight(arr[curIdx]->left) - getHeight(arr[curIdx]->right);
                }
                animMode = 13;
            }
        } else if (animMode == 14)
        {
            if (curNode->left != nullptr) curIdx = curNode->left->id; 
            else
            {
                arr[targetIdx]->value = curNode->value;
                std::swap(curIdx, targetIdx);
                animMode = 12;
            }
        } else if (animMode == 12)
        {
            Node* delNode = arr[targetIdx];
            Node* child = (delNode->left != nullptr) ? delNode->left : delNode->right;
            Node* p = delNode->parent;

            if (child != nullptr) child->parent = p;

            if (p == nullptr) root = child;
            else if (p->left != nullptr && p->left->id == delNode->id) p->left = child;
            else p->right = child;

            if (child != nullptr)
            {
                curIdx = child->id;
                child->x = child->vX = delNode->vX;
                child->y = child->vY = delNode->vY;
            }
            else curIdx = (p != nullptr) ? p->id : -1;

            targetIdx = curIdx;

            arr[delNode->id] = nullptr;
            delete delNode;

            calculatePositions(root, x_root, y_root, delta_x);
            isMoving = true;
            moveTimer = 0.0f;

            if (curIdx != -1)
            {
                arr[curIdx]->height = 1 + std::max(getHeight(arr[curIdx]->left), getHeight(arr[curIdx]->right));
                arr[curIdx]->bf = getHeight(arr[curIdx]->left) - getHeight(arr[curIdx]->right);
            }
            animMode = 13;
        } else if (animMode == 13)
        {
            if (curIdx == -1 || arr[curIdx] == nullptr) { animMode = 0; targetIdx = -1; return; }

            Node* backtrackNode = arr[curIdx];

            if (abs(backtrackNode->bf) > 1) animMode = 15, animSpeed = 0;
            else
            {
                curIdx = (backtrackNode->parent != nullptr) ? backtrackNode->parent->id : -1;
                curNode = (curIdx != -1) ? arr[curIdx] : nullptr;
                if (curNode != nullptr)
                {
                    curNode->height = 1 + std::max(getHeight(curNode->left), getHeight(curNode->right));
                    curNode->bf = getHeight(curNode->left) - getHeight(curNode->right);
                }
                if (curIdx == -1) animMode = 0;
            }
            targetIdx = curIdx;
        } else if (animMode == 15)
        {
            Node* y = arr[targetIdx]; Node* p = y->parent; Node* newRoot = nullptr;
            int balance = getBalance(y);

            if (balance > 1 && getBalance(y->left) >= 0) newRoot = rightRotate(y); 
            else if (balance < -1 && getBalance(y->right) <= 0) newRoot = leftRotate(y); 
            else if (balance > 1 && getBalance(y->left) < 0) y->left = leftRotate(y->left), newRoot = rightRotate(y);
            else if (balance < -1 && getBalance(y->right) > 0) y->right = rightRotate(y->right), newRoot = leftRotate(y);

            if (p == nullptr) root = newRoot;
            else { if (p->left == y) p->left = newRoot; else p->right = newRoot; }

            calculatePositions(root, x_root, y_root, delta_x);
            isMoving = 1, moveTimer = 0.0f;
            
            curIdx = targetIdx = newRoot->id;
            animMode = (curIdx != -1) ? 13 : 0; 
        } else if (animMode == 20) 
        {
            if (curNode == nullptr || targetIdx == curIdx)
            { 
                animMode = 0;
                return; 
            }
            if (pendingValue < curNode->value) 
            {
                if (curNode->left) curIdx = curNode->left->id; 
                else animMode = 0;
            } else if (pendingValue > curNode->value) 
            {
                if (curNode->right) curIdx = curNode->right->id; 
                else animMode = 0;
            } else targetIdx = curIdx;
        }
    }
}

void AVL::captureSnapshot()
{
    Snapshot sn;
    sn.animMode = animMode;
    sn.curIdx = curIdx;
    sn.targetIdx = targetIdx;
    sn.rootId = (root != nullptr) ? root->id : -1;
    sn.pendingValue = pendingValue;

    for (Node* n : arr) 
    {
        if (n == nullptr) continue;
        
        Snapshot::NodeState ns;
        ns.id = n->id;
        ns.val = n->value;
        ns.h = n->height;
        ns.bf = n->bf;
        ns.vx = n->x; 
        ns.vy = n->y;
        ns.lId = (n->left != nullptr) ? n->left->id : -1;
        ns.rId = (n->right != nullptr) ? n->right->id : -1;
        ns.pId = (n->parent != nullptr) ? n->parent->id : -1;
        
        sn.states.push_back(ns);
    }
    
    history.push_back(sn);
}

void AVL::restoreSnapshot(const Snapshot& sn)
{
    animMode = sn.animMode;
    curIdx = sn.curIdx;
    targetIdx = sn.targetIdx;
    pendingValue = sn.pendingValue;

    for (Node* n : arr) 
        if (n != nullptr) n->left = n->right = n->parent = nullptr;

    for (const Snapshot::NodeState& ns : sn.states) 
    {
        if (arr[ns.id] == nullptr) arr[ns.id] = new Node(ns.val, ns.vx, ns.vy, ns.id, nullptr);
        Node* n = arr[ns.id];
        n->value = ns.val;
        n->height = ns.h;
        n->bf = ns.bf;
        n->x = n->vX = ns.vx; 
        n->y = n->vY = ns.vy; 
    }

    for (const Snapshot::NodeState& ns : sn.states) 
    {
        Node* n = arr[ns.id];
        if (ns.lId != -1) n->left = arr[ns.lId];
        if (ns.rId != -1) n->right = arr[ns.rId];
        if (ns.pId != -1) n->parent = arr[ns.pId];
    }

    root = (sn.rootId != -1) ? arr[sn.rootId] : nullptr;

    isMoving = false;
    moveTimer = 0.0f;
}

static void draw(AVL::Node* cur, AVL& tree)
{
    if (!cur) return;
    
    if (cur->left != nullptr)
    {
        DrawLineEx({cur->vX, cur->vY}, {cur->left->vX, cur->left->vY}, 3.0f, BLACK);
        draw(cur->left, tree);
    }
    
    if (cur->right != nullptr)
    {
        DrawLineEx({cur->vX, cur->vY}, {cur->right->vX, cur->right->vY}, 3.0f, BLACK);
        draw(cur->right, tree);
    }

    int myIdx = cur->id; 
    Vector2 drawPos = {cur->vX, cur->vY};

    if (tree.animMode != 0)
    {
        if (myIdx == tree.curIdx) DrawCircleV(drawPos, 35, ORANGE);
        if (myIdx == tree.targetIdx) DrawCircleV(drawPos, 35, RED);
    }

    DrawText(TextFormat("%d", cur->bf), drawPos.x - 5, drawPos.y - 55, 20, RED);
    DrawNode(drawPos, cur->value, 20, 30, 4);
}

void AVL::drawTree()
{
    if (!root) return;
    updateAnimation();
    draw(root, *this);
}

void runAVL(AppState &currentState)
{
    static char valBuffer[16] = "10";
    static char inputBuffer[2048] = "1 2 3 4 5";

    static bool editModeValue = false;
    static bool editMode = false;

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    myAVL.drawTree();

    float X = 60, Y = 150;
    
    DrawForwardButton(875, 800, myAVL);
    DrawBackwardButton(725, 800, myAVL);

    bool isBusy = (myAVL.animMode != 0 || myAVL.isMoving);
    if (isBusy) GuiSetState(STATE_DISABLED);

    DrawToggle(X, Y, myAVL);
    DrawInitPanel(X, Y + 125, myAVL, inputBuffer, editMode);
    DrawOperationPanel(X, Y + 425, myAVL, valBuffer, editModeValue);

    if (!isBusy) GuiSetState(STATE_NORMAL);
}