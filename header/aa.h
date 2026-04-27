#ifndef AA_H
#define AA_H

#include "raylib.h"
#include "common.h"
#include <vector>

struct AA
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
        int activeLine;
    };

    std::vector<Node*> arr;
    int sz, pendingValue;
    Node* root;

    AA();
    ~AA();

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
    int activeLine = -1;

    int mode = 0;

    void startInsertAnimation(int value);
    void startDeleteAnimation(int value);
    void startSearchAnimation(int value);
    
    void updateAnimation();
    
    std::vector<Snapshot> history;
    void captureSnapshot();
    void restoreSnapshot(const Snapshot& sn);
};

void runAA(AppState &currentState);

#endif