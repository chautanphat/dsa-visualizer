#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <string>
#include <vector>

enum AppState
{
    MENU,
    LINKED_LIST,
    HEAP,
    AVLTREE,
    AATREE,
    MINIMUM_SPANNING_TREE,
    SHORTEST_PATHS
};
struct CodePanel
{
    Rectangle bounds = { 0.0f, 0.0f, 0.0f, 0.0f };
    std::string title;
    std::vector<std::string> lines;
    Vector2 scroll = { 0.0f, 0.0f };
    int activeLine = -1;
    int lastActiveLine = -1;
    int fontSize = 20;
    int lineHeight = 28;
    float padding = 10.0f;
};

bool DrawCustomButton(Rectangle bounds, const char* text);
void makeGuiLabel(float x, float y, const char* text);
void DrawNumberInBox(Rectangle box, int number, int fontSize, Color textColor);
void DrawTextInBox(Rectangle box, const char* text, int fontSize, Color textColor);
void DrawNode(Vector2 center, int value, int fontSize = 20, float radius = 30, float borderThick = 4);
void DrawMultiLineEditor(Rectangle bounds, char *buffer, int maxSize, bool &editMode, Vector2 &scroll);
void DrawCodePanel(CodePanel &panel, Rectangle bounds, const std::string &title, const std::vector<std::string> &lines, int activeLine);

#endif