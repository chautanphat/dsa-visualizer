#include "raylib.h"
#include "raygui.h"
#include <cstring>

static int editorCursorIndex = 0;

static int CountEditorLines(const char *text)
{
    int lines = 1;
    for (int i = 0; text[i] != '\0'; i++)
        if (text[i] == '\n') lines++;
    return lines;
}

static int GetLineStart(const char *text, int cursorIndex)
{
    while (cursorIndex > 0 && text[cursorIndex - 1] != '\n') cursorIndex--;
    return cursorIndex;
}

static int GetLineEnd(const char *text, int cursorIndex)
{
    int len = (int)strlen(text);
    while (cursorIndex < len && text[cursorIndex] != '\n') cursorIndex++;
    return cursorIndex;
}

static int GetColumn(const char *text, int cursorIndex)
{
    return cursorIndex - GetLineStart(text, cursorIndex);
}

static int MoveCursorVertically(const char *text, int cursorIndex, int direction)
{
    int len = (int)strlen(text);
    int lineStart = GetLineStart(text, cursorIndex);
    int column = GetColumn(text, cursorIndex);

    if (direction < 0)
    {
        if (lineStart == 0) return cursorIndex;

        int prevEnd = lineStart - 1;
        int prevStart = GetLineStart(text, prevEnd);
        int prevLength = prevEnd - prevStart;
        return prevStart + ((column < prevLength) ? column : prevLength);
    }

    int lineEnd = GetLineEnd(text, cursorIndex);
    if (lineEnd >= len) return cursorIndex;

    int nextStart = lineEnd + 1;
    int nextEnd = GetLineEnd(text, nextStart);
    int nextLength = nextEnd - nextStart;
    return nextStart + ((column < nextLength) ? column : nextLength);
}

static void InsertCharAt(char *buffer, int maxSize, int &cursorIndex, char ch)
{
    int len = (int)strlen(buffer);
    if (len >= maxSize - 1) return;

    memmove(buffer + cursorIndex + 1, buffer + cursorIndex, len - cursorIndex + 1);
    buffer[cursorIndex] = ch;
    cursorIndex++;
}

static void DeleteCharBefore(char *buffer, int &cursorIndex)
{
    int len = (int)strlen(buffer);
    if (cursorIndex <= 0 || len <= 0) return;

    memmove(buffer + cursorIndex - 1, buffer + cursorIndex, len - cursorIndex + 1);
    cursorIndex--;
}

static void DeleteCharAt(char *buffer, int cursorIndex)
{
    int len = (int)strlen(buffer);
    if (cursorIndex < 0 || cursorIndex >= len) return;

    memmove(buffer + cursorIndex, buffer + cursorIndex + 1, len - cursorIndex);
}

static int GetCursorFromMouse(Rectangle bounds, Vector2 scroll, const char *buffer, float padding, int lineHeight)
{
    Vector2 mouse = GetMousePosition();
    float localX = mouse.x - bounds.x - padding - scroll.x;
    float localY = mouse.y - bounds.y - padding - scroll.y;
    if (localX < 0) localX = 0;
    if (localY < 0) localY = 0;

    int targetLine = (int)(localY / lineHeight);
    int len = (int)strlen(buffer);
    int currentLine = 0;
    int lineStart = 0;

    while (currentLine < targetLine && lineStart < len)
    {
        if (buffer[lineStart] == '\n') currentLine++;
        lineStart++;
    }

    if (currentLine < targetLine) return len;

    int lineEnd = GetLineEnd(buffer, lineStart);
    int bestIndex = lineStart;
    int bestDistance = (int)localX;

    for (int i = lineStart; i <= lineEnd; i++)
    {
        int width = MeasureText(TextFormat("%.*s", i - lineStart, buffer + lineStart), 20);
        int distance = width - (int)localX;
        if (distance < 0) distance = -distance;
        if (i == lineStart || distance <= bestDistance)
        {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return bestIndex;
}

static void EnsureCursorVisible(Rectangle bounds, Vector2 &scroll, const char *buffer, int cursorIndex, float padding, int lineHeight)
{
    int lineStart = GetLineStart(buffer, cursorIndex);
    int cursorLine = 0;
    for (int i = 0; i < lineStart; i++)
        if (buffer[i] == '\n') cursorLine++;

    int cursorX = MeasureText(TextFormat("%.*s", cursorIndex - lineStart, buffer + lineStart), 20);
    float visibleLeft = -scroll.x;
    float visibleRight = -scroll.x + bounds.width - 24.0f - padding;
    float visibleTop = -scroll.y;
    float visibleBottom = -scroll.y + bounds.height - padding;
    float drawX = (float)cursorX;
    float drawY = (float)(cursorLine * lineHeight);

    if (drawX < visibleLeft) scroll.x = -drawX;
    else if (drawX > visibleRight) scroll.x = -(drawX - (bounds.width - 24.0f - padding));

    if (drawY < visibleTop) scroll.y = -drawY;
    else if (drawY + lineHeight > visibleBottom) scroll.y = -(drawY + lineHeight - (bounds.height - padding));
}

static void HandleMultiLineInput(char *buffer, int maxSize, bool editMode, Rectangle bounds, Vector2 &scroll, float padding, int lineHeight)
{
    if (!editMode) return;

    int len = (int)strlen(buffer);
    if (editorCursorIndex > len) editorCursorIndex = len;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Rectangle textArea = { bounds.x + 1, bounds.y + 1, bounds.width - 18, bounds.height - 2 };
        if (CheckCollisionPointRec(GetMousePosition(), textArea))
            editorCursorIndex = GetCursorFromMouse(bounds, scroll, buffer, padding, lineHeight);
    }

    if (IsKeyPressed(KEY_LEFT) && editorCursorIndex > 0) editorCursorIndex--;
    if (IsKeyPressed(KEY_RIGHT) && editorCursorIndex < len) editorCursorIndex++;
    if (IsKeyPressed(KEY_UP)) editorCursorIndex = MoveCursorVertically(buffer, editorCursorIndex, -1);
    if (IsKeyPressed(KEY_DOWN)) editorCursorIndex = MoveCursorVertically(buffer, editorCursorIndex, 1);
    if (IsKeyPressed(KEY_HOME)) editorCursorIndex = GetLineStart(buffer, editorCursorIndex);
    if (IsKeyPressed(KEY_END)) editorCursorIndex = GetLineEnd(buffer, editorCursorIndex);

    if (IsKeyPressed(KEY_BACKSPACE)) DeleteCharBefore(buffer, editorCursorIndex);
    if (IsKeyPressed(KEY_DELETE)) DeleteCharAt(buffer, editorCursorIndex);
    if (IsKeyPressed(KEY_ENTER)) InsertCharAt(buffer, maxSize, editorCursorIndex, '\n');

    int key = GetCharPressed();
    while (key > 0)
    {
        if (key >= 32 && key <= 125) InsertCharAt(buffer, maxSize, editorCursorIndex, (char)key);
        key = GetCharPressed();
    }

    EnsureCursorVisible(bounds, scroll, buffer, editorCursorIndex, padding, lineHeight);
}

bool DrawCustomButton(Rectangle bounds, const char* text)
{
    Vector2 mousePoint = GetMousePosition();
    bool hovering = CheckCollisionPointRec(mousePoint, bounds);
    bool clicked = false;

    Color buttonColor = SKYBLUE;
    if (hovering)
    {
        buttonColor = BLUE;
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) buttonColor = DARKBLUE;
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) clicked = true;
    }

    DrawRectangleRounded(bounds, 0.3, 10, buttonColor);
    DrawRectangleRoundedLines(bounds, 0.3, 10, WHITE);
    int textWidth = MeasureText(text, 20);
    DrawText(text, bounds.x + (bounds.width/2) - (textWidth/2), bounds.y + (bounds.height/2) - 10, 20, WHITE);

    return clicked;
}

void makeGuiLabel(float x, float y, const char* text)
{
    DrawText(text, x, y, 20, BLACK);
    // GuiLabel((Rectangle){ x, y, (float)MeasureText(text, GuiGetStyle(DEFAULT, TEXT_SIZE)), 30 }, text);
}

void DrawTextInBox(Rectangle box, const char* text, int fontSize, Color textColor)
{
    int textWidth = MeasureText(text, fontSize);
    float textX = box.x + (box.width / 2) - (textWidth / 2.0f);
    float textY = box.y + (box.height / 2) - (fontSize / 2.0f);
    DrawText(text, (int)textX, (int)textY, fontSize, textColor);
}

void DrawNumberInBox(Rectangle box, int number, int fontSize, Color textColor)
{
    const char* text = TextFormat("%d", number);
    int textWidth = MeasureText(text, fontSize);
    float textX = box.x + (box.width / 2) - (textWidth / 2.0f);
    float textY = box.y + (box.height / 2) - (fontSize / 2.0f);
    DrawText(text, (int)textX, (int)textY, fontSize, textColor);
}

void DrawNode(Vector2 center, int value, int fontSize, float radius, float borderThick)
{
    DrawCircleV(center, radius, WHITE);
    DrawRing(center, radius - borderThick, radius, 0.0f, 360.0f, 40, BLACK);
    const char* text = TextFormat("%d", value);
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, center.x - textWidth/2, center.y - fontSize/2, fontSize, BLACK);
}

void DrawMultiLineEditor(Rectangle bounds, char *buffer, int maxSize, bool &editMode, Vector2 &scroll)
{
    const float padding = 8.0f;
    const int lineHeight = 24;
    const int visibleLines = 5;
    const float contentWidth = bounds.width - 24.0f;
    Rectangle content = { 0, 0, contentWidth, (float)(CountEditorLines(buffer) * lineHeight + 12) };
    if (content.height < visibleLines * lineHeight + 12) content.height = visibleLines * lineHeight + 12;

    Rectangle view = { 0 };
    GuiScrollPanel(bounds, NULL, content, &scroll, &view);

    Rectangle textArea = { bounds.x + 1, bounds.y + 1, bounds.width - 18, bounds.height - 2 };
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        editMode = CheckCollisionPointRec(GetMousePosition(), textArea);
        if (editMode)
        {
            int len = (int)strlen(buffer);
            if (editorCursorIndex > len) editorCursorIndex = len;
        }
    }

    HandleMultiLineInput(buffer, maxSize, editMode, bounds, scroll, padding, lineHeight);

    BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);

    float drawX = bounds.x + padding + scroll.x;
    float drawY = bounds.y + padding + scroll.y;

    int start = 0;
    int line = 0;
    int len = (int)strlen(buffer);

    for (int i = 0; i <= len; i++)
    {
        if (buffer[i] == '\n' || buffer[i] == '\0')
        {
            int count = i - start;
            DrawText(TextFormat("%.*s", count, buffer + start), (int)drawX, (int)(drawY + line * lineHeight), 20, BLACK);
            start = i + 1;
            line++;
        }
    }

    if (editMode)
    {
        int cursorLine = 0;
        for (int i = 0; i < editorCursorIndex; i++)
            if (buffer[i] == '\n') cursorLine++;

        int lastLineStart = GetLineStart(buffer, editorCursorIndex);
        int cursorOffset = MeasureText(TextFormat("%.*s", editorCursorIndex - lastLineStart, buffer + lastLineStart), 20);
        DrawLine((int)(drawX + cursorOffset), (int)(drawY + cursorLine * lineHeight), (int)(drawX + cursorOffset), (int)(drawY + cursorLine * lineHeight + 20), BLACK);
    }

    EndScissorMode();
}
