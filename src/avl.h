#ifndef AVL_H
#define AVL_H

#include "raylib.h"
#include "common.h"
#include "code_viewer.h"
#include <vector>

struct AVL
{
    struct Node
    {
        int value;
        float x, y;
        float vX, vY;
        int level;
        float delta_x;
        Node* parent;
        Node* left;
        Node* right;
        Node(int val, float _x, float _y, int _level, float delta_x, Node* _parent);
    };

    struct Snapshot
    {
        std::vector<int> values;
        int sz;
        int animMode;
        int curIdx, targetIdx;
        // int activeLine;
    };

    std::vector<Node*> arr;
    int sz;
    Node* head;

    AVL();
    ~AVL();

    void clear();
    void drawHeap();
    void push(int value);
    void insertNodeOnly(int value);
    void pop();
    void removeLastNodeOnly();
    int top();

    int animMode = 0;
    int curIdx = -1;
    int targetIdx = -1;
    float animTimer = 0.0f;
    float animSpeed = 0.8f;

    bool isMoving = false;
    int moveIdxA = -1, moveIdxB = -1;
    Vector2 startPosA, startPosB;
    float moveTimer = 0.0f;
    float moveDuration = 0.4f;

    int mode = 0;

    void startPushAnimation(int value);
    void startPopAnimation();
    void updateAnimation();
    
    std::vector<Snapshot> history;
    void captureSnapshot();
    void restoreSnapshot(const Snapshot& sn);

    // CodeViewer codeUI;
};

void runAVL(AppState &currentState);

#endif