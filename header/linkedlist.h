#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "raylib.h"
#include "common.h"
#include <string>
#include <vector>

struct LinkedList
{
    struct Node
    {
        int value;
        Node* next;
        Rectangle box;
        Node(int val);
    };

    struct Snapshot
    {
        std::vector<int> values;
        int animMode;
        int targetValue;
        int targetIndex;
        int currentIndex;
        int animPtrIdx;
        int searchResultIdx;
        int activeLine;
    };

    Node* head;
    bool isAnimating = false;
    Node* animPtr = nullptr;
    int pendingValue = 0;
    float animTimer = 0.0f;
    float animSpeed = 0.5f;
    Node* searchResult = nullptr;

    int animMode = 0; 
    int targetValue = 0; 
    int targetIndex = 0;
    int currentIndex = 0;

    std::vector<Snapshot> history;
    int mode = 0;
    int activeLine = -1;
    int sz = 0;

    void captureSnapshot();
    void restoreSnapshot(const Snapshot& sn);

    LinkedList();
    ~LinkedList();

    void addToHead(int value);
    void addToTail(int value);
    void update(int index, int value);
    void deleteNode(int index);
    void clear();
    void drawLinkedList(float startX, float startY);
    void randomize();
    void fileUpload();
    void manualUpload(const std::string &input);
    void startAddTailAnimation(int value);
    void startSearchAnimation(int value);
    void startUpdateAnimation(int index, int value);
    void startDeleteAnimation(int index);
};

void runLinkedList(AppState &currentState);

#endif