#ifndef CODE_VIEWER_H
#define CODE_VIEWER_H

#include "raylib.h"
#include <vector>
#include <string>

struct CodeViewer
{
    std::vector<std::string> lines;
    int activeLine;
    int x; // Tọa độ X
    int y; // Tọa độ Y
};

void InitCodeViewer(CodeViewer& viewer, int x, int y);
void LoadCode(CodeViewer& viewer, const std::vector<std::string>& codeLines);
void SetActiveLine(CodeViewer& viewer, int lineIndex);
void DrawCodeViewer(CodeViewer& viewer);

#endif