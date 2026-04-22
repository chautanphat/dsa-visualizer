#include "code_viewer.h"

void InitCodeViewer(CodeViewer& viewer, int x, int y)
{
    viewer.x = x;
    viewer.y = y;
    viewer.activeLine = -1;
}

void LoadCode(CodeViewer& viewer, const std::vector<std::string>& codeLines)
{
    viewer.lines = codeLines;
    viewer.activeLine = -1;
}

void SetActiveLine(CodeViewer& viewer, int lineIndex)
{
    viewer.activeLine = lineIndex;
}

void DrawCodeViewer(CodeViewer& viewer)
{
    if (viewer.lines.empty()) return;

    int lineHeight = 30; // Chiều cao mỗi dòng
    int boxWidth = 350;  // Chiều dài vệt highlight vàng

    for (int i = 0; i < (int)viewer.lines.size(); i++)
    {
        int drawY = viewer.y + (i * lineHeight);

        // 1. Nếu là dòng active -> Vẽ cục màu vàng chìm làm nền
        if (i == viewer.activeLine)
        {
            DrawRectangle(viewer.x, drawY, boxWidth, lineHeight, Fade(YELLOW, 0.4f));
        }

        // 2. Vẽ chữ lên trên
        Color textColor = (i == viewer.activeLine) ? RED : BLACK; // Active màu đỏ, bình thường màu đen
        DrawText(viewer.lines[i].c_str(), viewer.x + 10, drawY + 5, 20, textColor);
    }
}