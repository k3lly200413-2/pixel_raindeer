#include "screenSetup.h"       // our own header — declares heartsBegin(), heartsUpdate(), and `extern tft`
#include <Adafruit_GFX.h>     // base graphics library (shapes, text) that Adafruit_ILI9341 builds on
#include <Adafruit_ILI9341.h> // driver for this specific display controller chip
#include <SPI.h>              // low-level serial bus the display communicates over
#include "Config.h"


// ESP32-S3 doesn't have fixed VSPI/HSPI pins like classic ESP32,
// so we build a SPIClass on the exact pins you wired.
// FSPI = one of the S3's flexible SPI peripheral instances; picking it
// explicitly (rather than the default SPI object) lets us route it to
// whichever GPIOs we want via tftSPI.begin() later in heartsBegin().
static SPIClass tftSPI = SPIClass(FSPI);

// The actual display driver object. Everything we draw goes through this.
// Deliberately NOT `static` — PixelHeart.h declares it `extern`, which is
// what lets DeerSprite.cpp / SpriteAnim.cpp / SpriteMenu.cpp all draw to
// the same physical screen instead of each needing their own copy.
Adafruit_ILI9341 tft = Adafruit_ILI9341(&tftSPI, TFT_DC, TFT_CS, TFT_RST);

// Public entry point — call once from the sketch's setup().
void screenInit() {
  tftSPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS); // start the SPI bus on our specific wired pins (must happen before tft.begin())

  tft.begin();          // initialize the display driver chip itself (sends its init command sequence)
  tft.setRotation(1);   // 1 = landscape orientation; without this the panel defaults to portrait
  tft.fillScreen(COLOR_BG); // start with a clean black screen
}
