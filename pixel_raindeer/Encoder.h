#pragma once
#include <Arduino.h>

// Call once from setup().
void encoderBegin(int clkPin, int dtPin, int swPin);

// Call anytime. Returns the number of detents turned since the last call
// (+1 per clockwise click, -1 per counter-clockwise, usually 0).
int encoderRead();

// Call anytime. Returns true exactly once per confirmed button press
// (debounced internally).
bool encoderClicked();