#include "raylib.h"
#include "raygui.h"

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
    DrawText(text, x, y, 20, DARKGRAY);
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