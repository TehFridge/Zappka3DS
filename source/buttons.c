#include "buttons.h"

Button buttonsy[100];
char tileNames[100][256];  // Array to store names/titles
int tileCount = 0;               // Track number of extracted names
void drawButton(Button* button, SceneType currentScene) {
    // Skip drawing if button is "removed"
    if (button->width == 0 || button->height == 0 || button->imageNormal.tex == NULL) {
        return;
    }

    // Check if button belongs to the current scene
    if (button->scene == currentScene || 
        button->scene2 == currentScene || 
		button->scene3 == currentScene ||
        button->scene4 == currentScene || 
        button->scene5 == currentScene) {

        if (button->isPressed) {
            C2D_DrawImageAt(button->imagePressed, button->x, button->y, 0.0f, NULL, button->sizenmachen, button->sizenmachen);
        } else {
            C2D_DrawImageAt(button->imageNormal, button->x, button->y, 0.0f, NULL, button->sizenmachen, button->sizenmachen);
        }
    }
}

bool isButtonPressed(Button* button, touchPosition touch, int currentScene) {
    if (button->scene != currentScene && button->scene2 != currentScene && button->scene3 != currentScene && button->scene4 != currentScene && button->scene5 != currentScene) return false;
    return touch.px >= button->x && touch.px <= button->x + button->width &&
           touch.py >= button->y && touch.py <= button->y + button->height;
}

void removeButtonEntries(int max) {
    if (max < 5 || max >= 100) return; // Ensure valid range

    for (int i = max; i >= 5; i--) {
        buttonsy[i].isPressed = false;
        buttonsy[i].width = 0;
        buttonsy[i].height = 0;
        buttonsy[i].x = -9999;
        buttonsy[i].y = -9999;
        buttonsy[i].sizenmachen = 0.0f;
        buttonsy[i].onClick = NULL;

        // Remove the image references without deleting the actual textures
        buttonsy[i].imageNormal = (C2D_Image){0};
        buttonsy[i].imagePressed = (C2D_Image){0};

        // Mark scenes as invalid so the button isn't drawn
        buttonsy[i].scene = -1;
        buttonsy[i].scene2 = -1;
        buttonsy[i].scene3 = -1;
        buttonsy[i].scene4 = -1;
		buttonsy[i].scene5 = -1;
    }
}
