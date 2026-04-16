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
        float x, y;
        float vX, vY;
        int level;
        float delta_x;
        Node* parent;
        Node* left;
        Node* right;
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
    void insertNodeOnly(int value);
    void pop();
    void removeLastNodeOnly();
    int top();

    int animMode = 0;      // 0: Idle, 1: Heapify Up (Push), 2: Heapify Down (Pop)
    int curIdx = -1;       // Chỉ số đang xét
    int targetIdx = -1;    // Chỉ số cha/con để so sánh
    float animTimer = 0.0f;
    float animSpeed = 0.8f; // Thời gian chờ mỗi bước (giây)

    bool isMoving = false;    // Trạng thái đang di chuyển tọa độ
    int moveIdxA = -1, moveIdxB = -1; // Hai chỉ số mảng đang đổi chỗ
    Vector2 startPosA, startPosB;    // Vị trí bắt đầu của 2 node
    float moveTimer = 0.0f;
    float moveDuration = 0.4f;       // Tốc độ bay (0.4 giây là vừa đẹp)

    void startPushAnimation(int value);
    void startPopAnimation();
    void updateAnimation(); // Hàm xử lý logic chuyển động mỗi khung hình
};

void runHeap(AppState &currentState);

#endif