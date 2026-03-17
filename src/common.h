#ifndef COMMON_H
#define COMMON_H

bool DrawCustomButton(Rectangle bounds, const char* text);
void makeGuiLabel(float x, float y, const char* text);
void DrawNumberInBox(Rectangle box, int number, int fontSize, Color textColor);
void DrawTextInBox(Rectangle box, const char* text, int fontSize, Color textColor);

#endif