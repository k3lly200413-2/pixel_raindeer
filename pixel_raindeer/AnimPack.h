#pragma once
#include "SpriteAnim.h"

// One "pack" of related animations, shown as a single icon in the
// top-level menu. Clicking it opens a second grid containing just this
// pack's animations.
struct AnimPack {
  SpriteAnim* icon;    // whose frame 0 is used as this pack's thumbnail — usually just its first animation
  SpriteAnim** items;  // the animations belonging to this pack
  int itemCount;
};