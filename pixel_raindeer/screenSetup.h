#pragma once
#include <Adafruit_ILI9341.h>

// Public interface for the pixel heart-beat animation.
// Call heartsBegin() once from setup(), heartsUpdate() every loop().

void screenInit();

// Shared display object, defined in PixelHeart.cpp — other modules
// (like DeerSprite) draw to the same screen through this.
extern Adafruit_ILI9341 tft;
