#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "raylib.h"
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

    LinkedList();
    ~LinkedList();

    void addToHead(int value);
    void addToTail(int value);
    void clear();
    void drawLinkedList(float startX, float startY);
    void randomize();
    void fileUpload();
    void manualUpload(const std::string &input);
};

void runLinkedList();

#endif