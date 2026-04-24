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
        int value, height, bf, id;
        float x, y;
        float vX, vY;
        Node *parent, *left, *right;
        Node(int val, float _x, float _y, int _id, Node* _parent);
    };

    struct Snapshot
    {
        struct NodeState
        {
            int id, val, h, bf;
            int pId, lId, rId;
            float vx, vy;
        };

        std::vector<NodeState> states;
        int rootId, pendingValue;
        int animMode;
        int curIdx, targetIdx;
    };

    std::vector<Node*> arr;
    int sz, pendingValue;
    Node* root;

    AVL();
    ~AVL();

    void clear();
    void drawTree();

    Node* insertLogic(Node* node, int val, Node* p);
    void insert(int value);

    int getHeight(Node* node);
    int getBalance(Node* node);
    Node* rightRotate(Node* y);
    Node* leftRotate(Node* x);

    void calculatePositions(Node* node, float currentX, float currentY, float hGap);

    int animMode = 0;
    int curIdx = -1, targetIdx = -1;
    float animTimer = 0.0f, animSpeed = 0.8f;
    bool isMoving = false;
    float moveTimer = 0.0f, moveDuration = 0.8f;

    int mode = 0;

    void startInsertAnimation(int value);
    void startDeleteAnimation(int value);
    void startSearchAnimation(int value);
    
    void updateAnimation();
    
    std::vector<Snapshot> history;
    void captureSnapshot();
    void restoreSnapshot(const Snapshot& sn);
};

void runAVL(AppState &currentState);

#endif