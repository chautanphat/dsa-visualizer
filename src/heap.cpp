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
    float x = 700, y = 100, delta_x = 512, delta_y = 128;
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
        if (sz % 2 == 1)
        {
            parent->left = child;
        }
        else
        {
            parent->right = child;
        }
    } else head = child;
    arr.push_back(child);
    sz++;
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

void runHeap(AppState &currentState)
{
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    myHeap.drawHeap();
    if (GuiButton({100, 100, 200, 50}, "Insert"))
    {
        myHeap.push(10);
    }
}