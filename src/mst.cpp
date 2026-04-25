#include "mst.h"
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

static MST myMST;

MST ::Node::Node(int val, float _x, float _y, int _id, Node* _parent) : value(val), height(1), level(1), id(_id), x(_x), y(_y), vX(_x), vY(_y), parent(_parent), left(nullptr), right(nullptr) {}

MST::MST() : sz(0), root(nullptr) { arr.clear(); }
MST::~MST() { clear(); }

void MST::clear()
{
    sz = 0;
    for (Node* node : arr)
        if (node != nullptr) delete node;
    arr.clear();
    history.clear(); 
    root = nullptr;
}

MST::Node* MST::skew(Node* node) 
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

MST::Node* MST::split(Node* node) 
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

void MST::decreaseLevel(Node* node) 
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

MST::Node* MST::insertLogic(Node* node, int val, Node* p)
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

void MST::insert(int value)
{
    root = insertLogic(root, value, nullptr);
    calculatePositions(root, x_root, y_root, delta_x);
    for (Node* n : arr)
        if (n != nullptr) n->vX = n->x, n->vY = n->y;
}

void MST::calculatePositions(Node* node, float currentX, float currentY, float hGap)
{
    if (node == nullptr) return;

    node->x = currentX;
    node->y = currentY;
    
    if (node->left != nullptr) calculatePositions(node->left, currentX - hGap, currentY + delta_y, hGap / 2.0f);
    if (node->right != nullptr) calculatePositions(node->right, currentX + hGap, currentY + delta_y, hGap / 2.0f);
}

static void DrawForwardButton(float x, float y, MST& MST)
{
    GuiSetState(STATE_NORMAL);
    if (MST.mode != 1 || MST.animMode == 0) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "Forward >")) 
    {
        MST.animSpeed = 0.0f; 
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawBackwardButton(float x, float y, MST& MST)
{
    if (MST.mode != 1 || MST.history.empty()) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "< Backward")) 
    {
        MST::Snapshot lastState = MST.history.back();
        MST.history.pop_back();
        MST.restoreSnapshot(lastState);
        if (MST.mode == 1) MST.animSpeed = 999999.0f;
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawToggle(float x, float y, MST& MST)
{
    int oldMode = MST.mode;

    makeGuiLabel(x + 45, y - 30, "Animation Mode:");
    GuiToggleGroup((Rectangle){ x, y, 130, 30 }, "Run-at-once;Step-by-step", &MST.mode);

    if (MST.mode != oldMode)
    {
        if (MST.mode == 1) MST.animSpeed = 999999.0f; 
        else MST.animSpeed = 0.8f;
    }
}

void MST::randomize()
{
    clear();

    int nodeCount = GetRandomValue(5, 8);
    nodes.reserve(nodeCount);
    for (int i = 0; i < nodeCount; i++)
        nodes.push_back(Node(i, i + 1));

    int nextEdgeId = 0;
    for (int i = 1; i < nodeCount; i++)
    {
        int j = GetRandomValue(0, i - 1);
        edges.push_back({ nextEdgeId++, j, i, GetRandomValue(1, 99), 0 });
    }

    for (int i = 0; i < nodeCount; i++)
        for (int j = i + 1; j < nodeCount; j++)
        {
            bool exists = false;
            for (const auto &e : edges)
                if ((e.u == i && e.v == j) || (e.u == j && e.v == i))
                {
                    exists = true;
                    break;
                }
            if (!exists && GetRandomValue(0, 1))
                edges.push_back({ nextEdgeId++, i, j, GetRandomValue(1, 99), 0 });
        }

    calculateNodePositions();
    rebuildSortedEdges();
    parent.resize(nodeCount);
    std::fill(parent.begin(), parent.end(), 0);
    std::iota(parent.begin(), parent.end(), 0);
    rank.assign(nodeCount, 0);
}

bool MST::manualUpload(const std::string &input)
{
    std::istringstream iss(input);
    std::map<int, int> labelToIndex;
    std::vector<std::tuple<int, int, int>> parsedEdges;

    int u, v, w;
    while (iss >> u >> v >> w)
    {
        if (u == v || w < 1) continue;
        if (labelToIndex.size() >= 12 && !labelToIndex.count(u) && !labelToIndex.count(v)) continue;

        if (!labelToIndex.count(u)) labelToIndex[u] = labelToIndex.size();
        if (!labelToIndex.count(v)) labelToIndex[v] = labelToIndex.size();

        parsedEdges.emplace_back(u, v, w);
    }

    if (parsedEdges.empty()) return false;

    clear();
    nodes.resize(labelToIndex.size());
    std::vector<int> labels(labelToIndex.size());
    for (auto &p : labelToIndex) labels[p.second] = p.first;
    for (int i = 0; i < (int)labels.size(); i++) nodes[i] = Node(i, labels[i]);

    int nextId = 0;
    for (auto &[lu, lv, ww] : parsedEdges)
    {
        int uu = labelToIndex[lu], vv = labelToIndex[lv];
        bool dup = false;
        for (auto &e : edges) if ((e.u == uu && e.v == vv) || (e.u == vv && e.v == uu)) { dup = true; break; }
        if (!dup) edges.push_back({ nextId++, uu, vv, ww, 0 });
    }

    if (edges.empty()) return false;

    calculateNodePositions();
    rebuildSortedEdges();
    parent.resize(nodes.size());
    std::iota(parent.begin(), parent.end(), 0);
    rank.assign(nodes.size(), 0);

    return true;
}

static void DrawInitPanel(float x, float y, MST &mst, char *inputBuf, bool &editMode)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 230 }, 1, BLACK);
    makeGuiLabel(x, y, "Initialize MST Graph");

    if (GuiButton((Rectangle){ x, y + 35, 145, 35 }, "Random"))
        mst.randomize();

    if (GuiButton((Rectangle){ x + 155, y + 35, 145, 35 }, "Upload"))
    {
        // Upload support can be added later.
    }

    if (GuiButton((Rectangle){ x, y + 90, 300, 35 }, "Manual"))
        mst.manualUpload(inputBuf);

    if (GuiTextBox((Rectangle){ x, y + 145, 300, 30 }, inputBuf, 2048, editMode))
        editMode = !editMode;
}

static void DrawOperationPanel(float x, float y, MST &mst)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 200 }, 1, BLACK);
    makeGuiLabel(x, y, "Operations");

    int curState = GuiGetState();
    if (mst.nodes.empty() || mst.edges.empty()) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y + 35, 300, 35 }, "Generate MST"))
        mst.startMSTAnimation();
    GuiSetState(curState);

    if (GuiButton((Rectangle){ x, y + 90, 300, 35 }, "Clear"))
        mst.clear();
}

void MST::startInsertAnimation(int value)
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

void MST::startDeleteAnimation(int value)
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

void MST::startSearchAnimation(int value)
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

void MST::updateAnimation()
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
            Node* y = arr[targetIdx];

            bool needRotate = false;
            if (y->left != nullptr && y->left->level == y->level) needRotate = true; 
            if (y->right != nullptr && y->right->right != nullptr && y->right->right->level == y->level) needRotate = true; 

            if (needRotate) animMode = 4;
            else animMode = 3;
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

                arr[delNode->id] = nullptr;
                delete delNode;

                calculatePositions(root, x_root, y_root, delta_x);
                isMoving = true;
                moveTimer = 0.0f;

                if (child != nullptr) curIdx = child->id; 
                else curIdx = (p != nullptr) ? p->id : -1;
                
                targetIdx = curIdx;
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

            arr[delNode->id] = nullptr;
            delete delNode;

            calculatePositions(root, x_root, y_root, delta_x);
            isMoving = true;
            moveTimer = 0.0f;

            if (child != nullptr) curIdx = child->id; 
            else curIdx = (p != nullptr) ? p->id : -1;
            
            targetIdx = curIdx;
            animMode = 13;
        } else if (animMode == 13)
        {
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
            } else animMode = 0; 
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

void MST::captureSnapshot()
{
    Snapshot sn;
    sn.animMode = animMode;
    sn.curIdx = curIdx;
    sn.targetIdx = targetIdx;
    
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

void MST::restoreSnapshot(const Snapshot& sn)
{
    animMode = sn.animMode;
    curIdx = sn.curIdx;
    targetIdx = sn.targetIdx;
    pendingValue = sn.pendingValue;

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

static void draw(MST::Node* cur, MST& tree)
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

    DrawText(TextFormat("%d", cur->level), drawPos.x - 5, drawPos.y - 55, 20, RED);
    DrawNode(drawPos, cur->value, 20, 30, 4);
}

void MST::drawTree()
{
    if (!root) return;
    updateAnimation();
    draw(root, *this);
}

void runMST(AppState &currentState)
{
    static char valBuffer[16] = "10";
    static char inputBuffer[2048] = "1 2 3 4 5";

    static bool editModeValue = false;
    static bool editMode = false;

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    myMST.drawTree();

    float X = 60, Y = 150;
    
    DrawForwardButton(875, 800, myMST);
    DrawBackwardButton(725, 800, myMST);

    bool isBusy = (myMST.animMode != 0 || myMST.isMoving);
    if (isBusy) GuiSetState(STATE_DISABLED);

    DrawToggle(X, Y, myMST);
    DrawInitPanel(X, Y + 125, myMST, inputBuffer, editMode);
    DrawOperationPanel(X, Y + 425, myMST, valBuffer, editModeValue);

    if (!isBusy) GuiSetState(STATE_NORMAL);
}