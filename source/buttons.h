#ifndef BUTTONS_H
#define BUTTONS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>
#include <stdarg.h>
#include <citro2d.h>
typedef enum {
    SCENE_MENU = 4,
    SCENE_GAME = 6
} SceneType;
typedef void (*ButtonFunction)();
typedef struct {
    float x, y, width, height;
    C2D_Image imageNormal;
    C2D_Image imagePressed;
    bool isPressed;
    SceneType scene; 
	SceneType scene2;
	SceneType scene3;
	SceneType scene4;
	SceneType scene5;
	float sizenmachen;
	ButtonFunction onClick;
} Button;

void drawButton(Button* button, SceneType currentScene);

bool isButtonPressed(Button* button, touchPosition touch, int currentScene);

void removeButtonEntries(int max);
#endif