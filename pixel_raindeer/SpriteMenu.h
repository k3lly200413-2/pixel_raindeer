#pragma once
#include "AnimPack.h"

// packs: array of pointers to AnimPack you've declared, e.g.
//   AnimPack* allPacks[] = { &deerPack, &catPack };
//   menuBegin(allPacks, 2, 8, 9, 10);
// Adding a new pack later = declare one more AnimPack, add it here.
void menuBegin(AnimPack* packs[], int packCount, int clkPin, int dtPin, int swPin);

// Call every loop(). Handles pack browsing, item browsing within a pack,
// and playing whichever animation was clicked.
void menuUpdate();