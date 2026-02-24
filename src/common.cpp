#include "raylib.h"
#include "raygui.h"

void makeGuiLabel(float x, float y, const char* text)
{
    GuiLabel((Rectangle){ x, y, (float)MeasureText(text, GuiGetStyle(DEFAULT, TEXT_SIZE)), 30 }, text);
}