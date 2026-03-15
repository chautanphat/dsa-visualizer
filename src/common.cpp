#include "raylib.h"
#include "raygui.h"

void makeGuiLabel(float x, float y, const char* text)
{
    GuiLabel((Rectangle){ x, y, (float)MeasureText(text, GuiGetStyle(DEFAULT, TEXT_SIZE)), 30 }, text);
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