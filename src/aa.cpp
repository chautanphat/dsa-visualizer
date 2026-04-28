#include "AA.h"
#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include "tinyfiledialogs.h"
#include <vector>
#include <string.h>
#include <sstream>
#include <fstream>
#include <iostream>

const float x_root = 1000.0f;
const float y_root = 100.0f;
const float delta_x = 256.0f;
const float delta_y = 128.0f;

static AA myAA;
static CodePanel aaCodePanel;

static const std::vector<std::string> insertCode =
{
    "if node == null: return new Node(val)",
    "if val < node.val:",
    "    node.left = insert(node.left, val)",
    "else if val > node.val:",
    "    node.right = insert(node.right, val)",
    "node = skew(node)",
    "node = split(node)",
    "return node"
};

static const std::vector<std::string> deleteCode =
{
    "if val < node.val: node.left = delete(node.left, val)",
    "else if val > node.val: node.right = delete(node.right, val)",
    "else:",
    "    if node is a leaf or has one child: delete node",
    "    else: replace with successor and delete successor",
    "decreaseLevel(node)",
    "node = skew(node); node.right = skew(node.right)",
    "node.right.right = skew(node.right.right)",
    "node = split(node); node.right = split(node.right)",
    "return node"
};

static const std::vector<std::string> searchCode =
{
    "if node == null: return false",
    "if val == node.val: return true",
    "if val < node.val: return search(node.left, val)",
    "else: return search(node.right, val)"
};

static const std::vector<std::string>* aaCurrentCode = &insertCode;
static std::string aaCurrentCodeTitle = "AA Tree Insert";

static int speedActive = 2;
static const float speedValues[] = { 0.25f, 0.5f, 1.0f, 1.5f, 2.0f };

AA ::Node::Node(int val, float _x, float _y, int _id, Node* _parent) : value(val), height(1), level(1), id(_id), x(_x), y(_y), vX(_x), vY(_y), parent(_parent), left(nullptr), right(nullptr) {}

AA::AA() : sz(0), root(nullptr) { arr.clear(); }
AA::~AA() { clear(); }

void AA::clear()
{
    sz = 0;
    for (Node* node : arr)
        if (node != nullptr) delete node;
    arr.clear();
    history.clear(); 
    root = nullptr;
    activeLine = -1;
}

AA::Node* AA::skew(Node* node) 
{
    if (node == nullptr || node->left == nullptr) return node;
    
    if (node->left->level == node->level) 
    {
        Node* leftChild = node->left;
        node->left = leftChild->right;
        if (leftChild->right != nullptr) leftChild->right->parent = node;
        leftChild->parent = node->parent;
        leftChild->right = node;
        node->parent = leftChild;
        return leftChild; 
    }
    
    return node;
}

AA::Node* AA::split(Node* node) 
{
    if (node == nullptr || node->right == nullptr || node->right->right == nullptr) return node;
    
    if (node->right->right->level == node->level) 
    {
        Node* rightChild = node->right;
        node->right = rightChild->left;
        if (rightChild->left != nullptr) rightChild->left->parent = node;
        rightChild->parent = node->parent;
        rightChild->left = node;
        node->parent = rightChild;
        rightChild->level++;
        return rightChild;
    }
    
    return node;
}

void AA::decreaseLevel(Node* node) 
{
    if (node == nullptr) return;
    int leftLevel = (node->left != nullptr) ? node->left->level : 0;
    int rightLevel = (node->right != nullptr) ? node->right->level : 0;
    int newLevel = std::min(leftLevel, rightLevel) + 1;
    if (newLevel < node->level) 
    {
        node->level = newLevel;
        if (node->right != nullptr && newLevel < node->right->level)
            node->right->level = newLevel;
    }
}

AA::Node* AA::insertLogic(Node* node, int val, Node* p)
{
    if (node == nullptr)
    {
        Node* newNode = new Node(val, 0.0f, 0.0f, sz, p); 
        if (sz < (int)arr.size()) arr[sz] = newNode;
        else arr.push_back(newNode);
        sz++;
        return newNode;
    }

    if (val < node->value) node->left = insertLogic(node->left, val, node);
    else if (val > node->value) node->right = insertLogic(node->right, val, node);
    else return node;

    return split(skew(node)); 
}

void AA::insert(int value)
{
    root = insertLogic(root, value, nullptr);
    calculatePositions(root, x_root, y_root, delta_x);
    for (Node* n : arr)
        if (n != nullptr) n->vX = n->x, n->vY = n->y;
}

void AA::calculatePositions(Node* node, float currentX, float currentY, float hGap)
{
    if (node == nullptr) return;

    node->x = currentX;
    node->y = currentY;
    
    if (node->left != nullptr) calculatePositions(node->left, currentX - hGap, currentY + delta_y, hGap / 2.0f);
    if (node->right != nullptr) calculatePositions(node->right, currentX + hGap, currentY + delta_y, hGap / 2.0f);
}

static void DrawAnimationControls(float centerX, float y, AA& AA)
{
    float btnW = 60, gap = 10, startX = centerX - (4 * btnW + 3 * gap) / 2.0f;

    GuiSetState((AA.mode == 1 && !AA.history.empty()) ? STATE_NORMAL : STATE_DISABLED);
    if (DrawCustomButton((Rectangle){ startX, y, btnW, 30 }, "#129#")) 
    {
        AA::Snapshot firstState = AA.history.front();
        AA.history.clear();
        AA.restoreSnapshot(firstState);
        if (AA.mode == 1) AA.animSpeed = 999999.0f;
    }

    startX += btnW + gap;
    if (DrawCustomButton((Rectangle){ startX, y, btnW, 30 }, "#130#")) 
    {
        AA::Snapshot lastState = AA.history.back();
        AA.history.pop_back();
        AA.restoreSnapshot(lastState);
        if (AA.mode == 1) AA.animSpeed = 999999.0f;
    }

    startX += btnW + gap;
    GuiSetState((AA.mode == 1 && AA.animMode != 0) ? STATE_NORMAL : STATE_DISABLED);
    if (DrawCustomButton((Rectangle){ startX, y, btnW, 30 }, "#131#")) AA.animSpeed = 0.0f; 

    startX += btnW + gap;
    if (DrawCustomButton((Rectangle){ startX, y, btnW, 30 }, "#134#")) 
    {
        while (AA.animMode != 0 || AA.isMoving)
        {
            AA.animSpeed = 0.0f;
            AA.moveTimer = 99999.0f;
            AA.animTimer = 99999.0f;
            AA.updateAnimation();
        }
        if (AA.mode == 1) AA.animSpeed = 999999.0f;
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawToggle(float x, float y, AA& AA)
{
    int oldMode = AA.mode;

    makeGuiLabel(x + 45, y - 30, "Animation Mode:");
    GuiToggleGroup((Rectangle){ x, y, 130, 30 }, "Run-at-once;Step-by-step", &AA.mode);

    if (AA.mode != oldMode)
    {
        if (AA.mode == 1) AA.animSpeed = 999999.0f; 
        else AA.animSpeed = 0.8f;
    }

    makeGuiLabel(120, 25, "Speed:");
    GuiToggleGroup((Rectangle){ 190, 20, 55, 30 }, "0.25x;0.5x;1x;1.5x;2x", &speedActive);
}

static void DrawInitPanel(float x, float y, AA& AA, char* inputBuf, bool& editMode)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 230 }, 1, BLACK);

    makeGuiLabel(x, y, "Initialize AA Tree");
    
    if (DrawCustomButton((Rectangle){ x, y + 35, 145, 35 }, "Random"))
    {
        AA.clear(); 

        int n = GetRandomValue(5, 30); 

        for (int i = 0; i < n; i++)
        {
            int v = GetRandomValue(1, 99);
            AA.insert(v);
        }
    }

    if (DrawCustomButton((Rectangle){ x + 155, y + 35, 145, 35 }, "Upload"))
    {
        const char* filters[] = { "*.txt" };
        const char* filepath = tinyfd_openFileDialog("Select File", "", 1, filters, "Text Files", 0);
        if (filepath)
        {
            std::ifstream file(filepath);
            if (file.is_open())
            {
                int value;
                AA.clear();
                while (file >> value && AA.sz < 31) AA.insert(value);
                file.close();
            }
        }
    }

    if (DrawCustomButton((Rectangle){ x, y + 90, 300, 35 }, "Manual"))
    { 
        std::istringstream iss(inputBuf);
        int value;
        AA.clear();
        while (iss >> value && AA.sz < 31) AA.insert(value);
    }

    if (GuiTextBox((Rectangle){ x, y + 145, 300, 30 }, inputBuf, 2048, editMode)) editMode = !editMode;
}

static void DrawOperationPanel(float x, float y, AA& AA, char* valBuf, bool& editModeVal)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 280 }, 1, BLACK);

    makeGuiLabel(x, y, "Operations");
    makeGuiLabel(x, y + 35, "Value:");
    
    if (GuiTextBox((Rectangle){ x + 110, y + 35, 70, 25 }, valBuf, 16, editModeVal)) editModeVal = !editModeVal;
    
    int curState = GuiGetState();
    if (AA.sz >= 31)
    {
        GuiSetState(STATE_DISABLED);
        DrawText("Maximum 31 nodes reached.", (int)x, (int)y + 112, 16, RED);
    }
    if (DrawCustomButton((Rectangle){ x, y + 75, 145, 35 }, "Insert"))
    { 
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value && AA.sz < 31)
        {
            AA.startInsertAnimation(value);
            strcpy(valBuf, TextFormat("%d", GetRandomValue(1, 99)));
        }
    }
    GuiSetState(curState);

    if (AA.sz == 0) GuiSetState(STATE_DISABLED);
    if (DrawCustomButton((Rectangle){ x + 155, y + 75, 145, 35 }, "Delete"))
    {
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value)
        {
            AA.startDeleteAnimation(value);
            strcpy(valBuf, TextFormat("%d", GetRandomValue(1, 99)));
        }
    }

    if (DrawCustomButton((Rectangle){ x, y + 130, 300, 35 }, "Search"))
    {
        std::istringstream iss(valBuf);
        int value;
        if (iss >> value)
        {
            AA.startSearchAnimation(value);
            strcpy(valBuf, TextFormat("%d", GetRandomValue(1, 99)));
        }
    }
    if (DrawCustomButton((Rectangle){ x, y + 185, 300, 35 }, "Clear")) AA.clear();
}

void AA::startInsertAnimation(int value)
{
    aaCurrentCode = &insertCode;
    aaCurrentCodeTitle = "AA Tree Insert";
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

void AA::startDeleteAnimation(int value)
{
    aaCurrentCode = &deleteCode;
    aaCurrentCodeTitle = "AA Tree Delete";
    if (sz == 0 || root == nullptr) return;
    history.clear(); 
    pendingValue = value; 
    isMoving = false;
    moveTimer = 0.0f;

    animMode = 10;
    curIdx = root->id;
    targetIdx = -1;
    animTimer = 0.0f;
    activeLine = 0;

    if (mode == 1) animSpeed = 999999.0f;
    else animSpeed = 0.8f;
}

void AA::startSearchAnimation(int value)
{
    aaCurrentCode = &searchCode;
    aaCurrentCodeTitle = "AA Tree Search";
    if (sz == 0 || root == nullptr) return;
    history.clear();
    pendingValue = value;
    isMoving = false;

    moveTimer = 0.0f;
    animTimer = 0.0f;
    animMode = 20;
    curIdx = root->id;
    targetIdx = -1;
    activeLine = 0;

    if (mode == 1) animSpeed = 999999.0f; 
    else animSpeed = 0.8f;
}

void AA::updateAnimation()
{
    if (isMoving) 
    {
        moveTimer += GetFrameTime() * speedValues[speedActive];
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

    animTimer += GetFrameTime() * speedValues[speedActive];
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
                if (curNode->left == nullptr)
                {
                    animMode = 2;
                    activeLine = 2;
                } else
                {
                    curIdx = curNode->left->id;
                    activeLine = 1;
                }
            } else if (pendingValue > curNode->value)
            {
                if (curNode->right == nullptr)
                {
                    animMode = 2;
                    activeLine = 4;
                } else
                {
                    curIdx = curNode->right->id;
                    activeLine = 3;
                }
            } else
            {
                activeLine = 7;
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
            activeLine = 0;
        } else if (animMode == 3) 
        {
            if (curNode == nullptr || curNode->parent == nullptr)
            {
                animMode = 0, targetIdx = -1;
                activeLine = 7;
                return;
            }
            
            curIdx = targetIdx = curNode->parent->id;
            Node* y = arr[targetIdx];

            bool needRotate = false;
            if (y->left != nullptr && y->left->level == y->level) needRotate = true; 
            if (y->right != nullptr && y->right->right != nullptr && y->right->right->level == y->level) needRotate = true; 

            if (needRotate)
            {
                animMode = 4;
                activeLine = 5;
            } else
            {
                animMode = 3;
                activeLine = 7;
            }
        } else if (animMode == 4)
        {
            Node* y = arr[targetIdx];
            Node* p = y->parent;
            
            Node* newRoot = skew(y);
            newRoot = split(newRoot);

            if (p == nullptr) root = newRoot;
            else
            {
                if (p->left == y) p->left = newRoot;
                else p->right = newRoot;
            }

            calculatePositions(root, x_root, y_root, delta_x);
            if (y != newRoot) isMoving = true; 
            moveTimer = 0.0f;
            
            curIdx = targetIdx = newRoot->id;
            animMode = (curIdx != -1) ? 3 : 0;
            if (animMode == 0) activeLine = 7;
            else activeLine = 6;
        } else if (animMode == 10)
        {
            if (curNode == nullptr) { animMode = 0; activeLine = 9; return; }
            
            if (pendingValue < curNode->value)
            {
                activeLine = 0;
                if (curNode->left) curIdx = curNode->left->id;
                else { animMode = 0; activeLine = 9; }
            } else if (pendingValue > curNode->value)
            {
                activeLine = 1;
                if (curNode->right) curIdx = curNode->right->id;
                else { animMode = 0; activeLine = 9; }
            } else
            { 
                activeLine = 2;
                targetIdx = curIdx;
                animMode = 11;
            }
        } else if (animMode == 11)
        {
            activeLine = 3;
            Node* delNode = arr[targetIdx];
            if (delNode->left != nullptr && delNode->right != nullptr)
            {
                curIdx = delNode->right->id; 
                animMode = 14; 
                activeLine = 4;
            } else
            {
                Node* child = (delNode->left != nullptr) ? delNode->left : delNode->right;
                Node* p = delNode->parent;

                if (child != nullptr) child->parent = p;

                if (p == nullptr) root = child;
                else if (p->left != nullptr && p->left->id == delNode->id) p->left = child;
                else p->right = child;

                arr[delNode->id] = nullptr;
                delete delNode;

                calculatePositions(root, x_root, y_root, delta_x);
                isMoving = true;
                moveTimer = 0.0f;

                if (child != nullptr) curIdx = child->id; 
                else curIdx = (p != nullptr) ? p->id : -1;
                
                targetIdx = curIdx;
                animMode = 13;
                activeLine = 5;
            }
        } else if (animMode == 14)
        {
            activeLine = 4;
            if (curNode->left != nullptr) curIdx = curNode->left->id; 
            else
            {
                arr[targetIdx]->value = curNode->value;
                std::swap(curIdx, targetIdx);
                animMode = 12;
            }
        } else if (animMode == 12)
        {
            activeLine = 4;
            Node* delNode = arr[targetIdx];
            Node* child = (delNode->left != nullptr) ? delNode->left : delNode->right;
            Node* p = delNode->parent;

            if (child != nullptr) child->parent = p;

            if (p == nullptr) root = child;
            else if (p->left != nullptr && p->left->id == delNode->id) p->left = child;
            else p->right = child;

            arr[delNode->id] = nullptr;
            delete delNode;

            calculatePositions(root, x_root, y_root, delta_x);
            isMoving = true;
            moveTimer = 0.0f;

            if (child != nullptr) curIdx = child->id; 
            else curIdx = (p != nullptr) ? p->id : -1;
            
            targetIdx = curIdx;
            animMode = 13;
            activeLine = 5;
        } else if (animMode == 13)
        {
            activeLine = 6;
            bool treeChanged = false;
            while (curIdx != -1 && arr[curIdx] != nullptr && !treeChanged)
            {
                Node* y = arr[curIdx];
                Node* p = y->parent;
                Node* originalRoot = y;

                decreaseLevel(y);
                y = skew(y);

                if (y->right != nullptr) 
                {
                    y->right = skew(y->right);
                    if (y->right->right != nullptr) y->right->right = skew(y->right->right);
                }
                
                y = split(y);
                if (y->right != nullptr) y->right = split(y->right);

                Node* newRoot = y;

                if (p == nullptr) root = newRoot;
                else if (p->left != nullptr && p->left->id == originalRoot->id) p->left = newRoot;
                else p->right = newRoot;
                if (originalRoot != newRoot) treeChanged = true;
                curIdx = (newRoot->parent != nullptr) ? newRoot->parent->id : -1;
                targetIdx = curIdx;
            }

            if (treeChanged)
            {
                calculatePositions(root, x_root, y_root, delta_x);
                isMoving = true; 
                moveTimer = 0.0f;
            } else {
                animMode = 0; 
                activeLine = 9;
            } 
        } else if (animMode == 20) 
        {
            if (curNode == nullptr || targetIdx == curIdx)
            { 
                animMode = 0;
                activeLine = (targetIdx == curIdx && targetIdx != -1) ? 1 : 0;
                return; 
            }
            if (pendingValue < curNode->value) 
            {
                activeLine = 2;
                if (curNode->left) curIdx = curNode->left->id; 
                else { animMode = 0; activeLine = 0; }
            } else if (pendingValue > curNode->value) 
            {
                activeLine = 3;
                if (curNode->right) curIdx = curNode->right->id; 
                else { animMode = 0; activeLine = 0; }
            } else {
                targetIdx = curIdx;
                activeLine = 1;
            }
        }
    }
}

void AA::captureSnapshot()
{
    Snapshot sn;
    sn.animMode = animMode;
    sn.curIdx = curIdx;
    sn.targetIdx = targetIdx;
    sn.activeLine = activeLine;
    
    if (root != nullptr) sn.rootId = root->id;
    else sn.rootId = -1;
    
    sn.pendingValue = pendingValue;

    for (Node* n : arr) 
    {
        if (n == nullptr) continue;
        Snapshot::NodeState ns;
        ns.id = n->id;
        ns.val = n->value;
        ns.level = n->level;
        ns.vx = n->x; 
        ns.vy = n->y;
        ns.lId = (n->left != nullptr) ? n->left->id : -1;
        ns.rId = (n->right != nullptr) ? n->right->id : -1;
        ns.pId = (n->parent != nullptr) ? n->parent->id : -1;
        sn.states.push_back(ns);
    }
    
    history.push_back(sn);
}

void AA::restoreSnapshot(const Snapshot& sn)
{
    animMode = sn.animMode;
    curIdx = sn.curIdx;
    targetIdx = sn.targetIdx;
    pendingValue = sn.pendingValue;
    activeLine = sn.activeLine;

    for (Node* n : arr) 
        if (n != nullptr)
        {
            n->left = nullptr;
            n->right = nullptr;
            n->parent = nullptr;
        }

    for (const Snapshot::NodeState& ns : sn.states) 
    {
        if (arr[ns.id] == nullptr) arr[ns.id] = new Node(ns.val, ns.vx, ns.vy, ns.id, nullptr);
        Node* n = arr[ns.id];
        n->value = ns.val;
        n->level = ns.level;
        n->x = ns.vx; 
        n->y = ns.vy;
        n->vX = ns.vx; 
        n->vY = ns.vy; 
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

static void draw(AA::Node* cur, AA& tree)
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

    Color bgColor = WHITE;
    Color txtColor = BLACK;

    if (tree.animMode != 0)
    {
        if (myIdx == tree.curIdx) bgColor = ORANGE;
        if (myIdx == tree.targetIdx) { bgColor = RED; txtColor = WHITE; }
    }

    DrawText(TextFormat("%d", cur->level), drawPos.x - 5, drawPos.y - 55, 20, RED);
    DrawCircleV(drawPos, 30, bgColor);
    DrawRing(drawPos, 26, 30, 0.0f, 360.0f, 40, BLACK);
    const char* text = TextFormat("%d", cur->value);
    DrawText(text, drawPos.x - MeasureText(text, 20)/2, drawPos.y - 10, 20, txtColor);
}

void AA::drawTree()
{
    if (!root) return;
    updateAnimation();
    draw(root, *this);
}

void runAA(AppState &currentState)
{
    static char valBuffer[16] = "10";
    static char inputBuffer[2048] = "1 2 3 4 5";

    static bool editModeValue = false;
    static bool editMode = false;

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    myAA.drawTree();

    float X = 60, Y = 150;
    
    DrawAnimationControls(800, 800, myAA);

    DrawCodePanel(aaCodePanel, code_panel, aaCurrentCodeTitle, *aaCurrentCode, myAA.activeLine);

    bool isBusy = (myAA.animMode != 0 || myAA.isMoving);
    if (isBusy) GuiSetState(STATE_DISABLED);

    DrawToggle(X, Y, myAA);
    DrawInitPanel(X, Y + 125, myAA, inputBuffer, editMode);
    DrawOperationPanel(X, Y + 425, myAA, valBuffer, editModeValue);

    if (!isBusy) GuiSetState(STATE_NORMAL);
}