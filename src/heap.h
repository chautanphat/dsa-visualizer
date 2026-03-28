#ifndef HEAP_H
#define HEAP_H

#include "raylib.h"
#include "common.h"
#include <vector>

struct Heap
{
    struct Node
    {
        int value;
        float x;
        float y;
        int level;
        float delta_x;
        Node* parent;
        Node* left;
        Node* right;
        Rectangle box;
        Node(int val, float _x, float _y, int _level, float delta_x, Node* _parent);
    };

    std::vector<Node*> arr;
    int sz;
    Node* head;

    Heap();
    ~Heap();

    void clear();
    void drawHeap();
    void push(int value);
    void pop();
    int top();
};

void runHeap(AppState &currentState);

#endif