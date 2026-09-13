#include "SpriteAnim.h"
#include "screenSetup.h"   // for the shared `tft` object — this is the display all our modules draw to

// Draws a single frame (identified by index) of a sprite at its current
// x/y/scale. Private to this file (static) — outside code only calls
// spriteBegin/spriteUpdate below, which call this internally.
static void drawFrame(SpriteAnim &anim, uint8_t frameIndex) {
  // The generated data array is stored as one long flat block: all of
  // frame 0's pixels, then all of frame 1's, etc. This line calculates
  // the starting address of the requested frame within that block.
  // Cast to uint32_t before multiplying to avoid overflow on larger
  // sprites/frame-counts (plain `int` multiplication could overflow on
  // some platforms; unlikely at 20x20 but a cheap safety habit).
  // Animation data is an array of unsigned 16 bit ints so just add the memory
  // to whatever size of data the frame is
  const uint16_t* frameBase = anim.data + (uint32_t)frameIndex * anim.width * anim.height;

  for (int row = 0; row < anim.height; row++) {
    for (int col = 0; col < anim.width; col++) {
      // pgm_read_word() is required (instead of a plain array read) because
      // the sprite data lives in flash (PROGMEM), not RAM — the AVR/ESP32
      // toolchain needs this special accessor to read flash memory correctly.
      uint16_t color = pgm_read_word(frameBase + row * anim.width + col);
      if (color == anim.transparentKey) continue; // skip drawing this pixel — lets the background show through instead
      // Each source pixel becomes a `scale`x`scale` block on screen —
      // this is what makes small pixel art look crisp instead of blurry
      // when blown up, versus a smoothed/interpolated resize.
      tft.writeFillRect(anim.x + col * anim.scale, anim.y + row * anim.scale,
                        anim.scale, anim.scale, color);
    }
  }
}

// Public entry point — call once per sprite, typically from setup() or
// whenever you want to start showing/restart a given animation.
void spriteBegin(SpriteAnim &anim, int x, int y, int scale) {
  // Store this instance's screen placement — these fields live in the
  // SpriteAnim struct itself, so multiple sprites can each remember their
  // own independent position/scale/timing.
  anim.x = x;
  anim.y = y;
  anim.scale = scale;
  anim.currentFrame = 0;            // always start animations from their first frame
  anim.lastFrameTime = millis();    // reset the frame timer so the first spriteUpdate() call doesn't immediately advance

  tft.startWrite();                 // batch this draw as one SPI transaction
  drawFrame(anim, anim.currentFrame);
  tft.endWrite();
}

// Public entry point — call every loop() for each active sprite. Only
// actually redraws when enough time has passed (frameIntervalMs); cheap
// to call every loop() iteration even when nothing changes.
void spriteUpdate(SpriteAnim &anim) {
  unsigned long now = millis();
  if (now - anim.lastFrameTime < anim.frameIntervalMs) return; // not time for the next frame yet — do nothing
  anim.lastFrameTime = now;

  // Advance to the next frame, wrapping back to 0 after the last one —
  // this is what makes the animation loop forever.
  anim.currentFrame = (anim.currentFrame + 1) % anim.frameCount;

  tft.startWrite();
  // Clear the sprite's full bounding box before drawing the new frame.
  // Simpler than the hearts' "only erase what moved" approach because
  // every sprite frame here is the same fixed size — nothing to track.
  tft.writeFillRect(anim.x, anim.y, anim.width * anim.scale, anim.height * anim.scale, ILI9341_BLACK);
  drawFrame(anim, anim.currentFrame);
  tft.endWrite();
}
