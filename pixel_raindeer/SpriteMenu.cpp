#include "SpriteMenu.h"
#include "Encoder.h"
#include "screenSetup.h"  
#include "mqttMessage.h"
#include "displayQueue.h"


// Three states the menu can be in — determines what turning/clicking does.
enum MenuState { STATE_PACKS, STATE_ITEMS, STATE_PLAYING };
static MenuState state = STATE_PACKS;

static AnimPack** packs;
static int packCount;
static int selectedPack = 0;

// Index 0 in the items grid is ALWAYS the back tile; real animations
// start at index 1. This offset is why you'll see "+ 1" / "- 1" below.
static int selectedItem = 0;

// Layout shared by both grid levels — same as before.
static const int CELL_SIZE   = 70;
static const int THUMB_SCALE = 3;
static const int GRID_COLS   = 3;
static const int MARGIN      = 10;

static void cellPosition(int index, int &cx, int &cy) {
  int col = index % GRID_COLS;
  int row = index / GRID_COLS;
  cx = MARGIN + col * (CELL_SIZE + MARGIN);
  cy = MARGIN + row * (CELL_SIZE + MARGIN);
}

// A left-pointing chevron "<", built the same way as the heart shape --
// a small grid of on/off cells, each drawn as a block via writeFillRect.
// (Deliberately NOT using tft.fillTriangle() here: that's a higher-level
// Adafruit_GFX call that manages its own internal SPI transaction, and
// calling it while we're already inside our own startWrite()/endWrite()
// block caused it to silently fail to render -- see PixelHeart.cpp's
// note about writeFillRect being the one call we've confirmed reliable.)
static const uint8_t backArrowPattern[7][4] = {
  {0,0,0,1},
  {0,0,1,0},
  {0,1,0,0},
  {1,0,0,0},
  {0,1,0,0},
  {0,0,1,0},
  {0,0,0,1},
};

static void drawBackIcon(int cx, int cy, uint16_t color) {
  const int blockSize = 6;
  const int rows = 7, cols = 4;
  int iconW = cols * blockSize;
  int iconH = rows * blockSize;
  int ox = cx + (CELL_SIZE - iconW) / 2;  // center the icon within the cell
  int oy = cy + (CELL_SIZE - iconH) / 2;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (backArrowPattern[r][c]) {
        tft.writeFillRect(ox + c * blockSize, oy + r * blockSize, blockSize, blockSize, color);
      }
    }
  }
}

// Shared by both grid levels: draws one cell's white/black selection
// background, then either a sprite thumbnail (anim) or the back arrow
// (isBack) on top.
static void drawThumbCell(int index, SpriteAnim* anim, bool isBack, bool selected) {
  int cx, cy;
  cellPosition(index, cx, cy);

  uint16_t bg = selected ? ILI9341_WHITE : ILI9341_BLACK;
  tft.writeFillRect(cx, cy, CELL_SIZE, CELL_SIZE, bg);

  if (isBack) {
    // pick whichever color stays visible against the current background
    uint16_t arrowColor = selected ? ILI9341_BLACK : ILI9341_WHITE;
    drawBackIcon(cx, cy, arrowColor);
    return;
  }

  int thumbW = anim->width * THUMB_SCALE;
  int thumbH = anim->height * THUMB_SCALE;
  int tx = cx + (CELL_SIZE - thumbW) / 2;
  int ty = cy + (CELL_SIZE - thumbH) / 2;

  const uint16_t* frame0 = anim->data;
  for (int row = 0; row < anim->height; row++) {
    for (int col = 0; col < anim->width; col++) {
      uint16_t color = pgm_read_word(frame0 + row * anim->width + col);
      if (color == anim->transparentKey) continue;
      tft.writeFillRect(tx + col * THUMB_SCALE, ty + row * THUMB_SCALE,
                        THUMB_SCALE, THUMB_SCALE, color);
    }
  }
}

// ---- top level: which pack ----
static void drawPackGrid() {
  tft.startWrite();
  tft.writeFillRect(0, 0, tft.width(), tft.height(), ILI9341_BLACK);
  for (int i = 0; i < packCount; i++) {
    drawThumbCell(i, packs[i]->icon, false, i == selectedPack);
  }
  tft.endWrite();
}

// ---- second level: which animation within the selected pack ----
static void drawItemGrid() {
  AnimPack &pack = *packs[selectedPack];
  tft.startWrite();
  tft.writeFillRect(0, 0, tft.width(), tft.height(), ILI9341_BLACK);
  drawThumbCell(0, nullptr, true, selectedItem == 0);   // the back tile
  for (int i = 0; i < pack.itemCount; i++) {
    drawThumbCell(i + 1, pack.items[i], false, selectedItem == i + 1);
  }
  tft.endWrite();
}

void menuBegin(AnimPack* p[], int count, int clkPin, int dtPin, int swPin) {
  packs = p;
  packCount = count;
  selectedPack = 0;
  selectedItem = 0;
  state = STATE_PACKS;
  encoderBegin(clkPin, dtPin, swPin);
  drawPackGrid();
}

void drawSetUp(AnimPack* pack, int selectedItem) {
    SpriteAnim &anim = *pack->items[selectedItem];
    int scale = 8;
    int x = (tft.width()  - anim.width  * scale) / 2;
    int y = (tft.height() - anim.height * scale) / 2;

    tft.startWrite();
    tft.writeFillRect(0, 0, tft.width(), tft.height(), ILI9341_BLACK);
    tft.endWrite();
    spriteBegin(anim, x, y, scale);
    state = STATE_PLAYING;
}

void menuUpdate() {
  if (state == STATE_PACKS) {
    // TODO: add queue check to see if message is in queue
    DisplayCommand cmd;
    if (xQueueReceive(displayQueue, &cmd, 0)) {
        if (cmd.packIndex >= 0 && cmd.packIndex < packCount) {
            AnimPack &pack = *packs[cmd.packIndex];
            if (cmd.animIndex >= 0 && cmd.animIndex < pack.itemCount) {

                selectedPack = cmd.packIndex;
                selectedItem = cmd.animIndex + 1; // +1 to match your existing back-tile offset convention

                drawSetUp(&pack, cmd.animIndex);
            }
        }
    }
    int delta = encoderRead();
    if (delta != 0) {
        // TODO: Understand this
        selectedPack = ((selectedPack + delta) % packCount + packCount) % packCount;
        drawPackGrid(); // full redraw — fine since this only happens on an actual turn, and pack counts are small
    }
    if (encoderClicked()) {
        selectedItem = 0; // always land on "back" first, so one click after entering a pack never accidentally plays something
        state = STATE_ITEMS;
        drawItemGrid();
    }

  } 
  else if (state == STATE_ITEMS) 
  {
    AnimPack &pack = *packs[selectedPack];
    int totalCells = pack.itemCount + 1; // +1 for the back tile
    int delta = encoderRead();
    if (delta != 0) {
      selectedItem = ((selectedItem + delta) % totalCells + totalCells) % totalCells;
      drawItemGrid();
    }
    if (encoderClicked()) {
      if (selectedItem == 0) {
        state = STATE_PACKS;   // back tile chosen
        drawPackGrid();
      } else {

        drawSetUp(&pack, selectedItem - 1);

        char buf[24];
        snprintf(buf, sizeof(buf), "%d,%d", selectedPack, selectedItem - 1);
        publish("Tonina/esp32", reinterpret_cast<const uint8_t*>(buf));
      }
    }
  } 
  else { // STATE_PLAYING
    AnimPack &pack = *packs[selectedPack];
    spriteUpdate(*pack.items[selectedItem - 1]);
    if (encoderClicked()) {
      state = STATE_ITEMS;
      drawItemGrid();
    }
  }
}

