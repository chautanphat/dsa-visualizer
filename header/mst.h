#ifndef MST_H
#define MST_H

#include "raylib.h"
#include "common.h"
#include "code_viewer.h"
#include <vector>

struct MST
{
    struct Node
    {
        int value, height, level, id;
        float x, y;
        float vX, vY;
        Node *parent, *left, *right;
        Node(int val, float _x, float _y, int _id, Node* _parent);
    };

    struct Snapshot
    {
        struct NodeState
        {
            int id, val, level;
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

    MST();
    ~MST();

    void clear();
    void drawTree();

    Node* skew(Node* node);
    Node* split(Node* node);
    void decreaseLevel(Node* node);

    Node* insertLogic(Node* node, int val, Node* p);
    void insert(int value);

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

void runMST(AppState &currentState);

#endif