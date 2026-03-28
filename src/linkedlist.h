#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "raylib.h"
#include "common.h"
#include <string>

struct LinkedList
{
    struct Node
    {
        int value;
        Node* next;
        Rectangle box;
        Node(int val);
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

    // int animMode = 0; 
    // Node* animPtr = nullptr;
    // int targetValue = 0; 
    // float animTimer = 0.0f;

    LinkedList();
    ~LinkedList();

    void addToHead(int value);
    void addToTail(int value);
    void update(int index, int value);
    void deleteNode(int index);
    void startSearchAnimation(int value);
    void clear();
    void drawLinkedList(float startX, float startY);
    void randomize();
    void fileUpload();
    void manualUpload(const std::string &input);
    void startAddTailAnimation(int value);
};

void runLinkedList(AppState &currentState);

#endif