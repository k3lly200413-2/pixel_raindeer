#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "screenSetup.h"
#include "SpriteAnim.h"
#include "AnimPack.h"
#include "SpriteMenu.h"
#include "jumpDeerSprite.h"
#include "musicDeerSprite.h"
#include "noDeerSprite.h"
#include "rubDeerSprite.h"
#include "sneezeDeerSprite.h"
#include "yesDeerSprite.h"
#include "yesyesDeerSprite.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "displayQueue.h"
#include "mqttMessage.h"
#include "Config.h"

/*  Idea: First block is queue of messagges device has recieved and not yet seen.
    Once message is sent it gets recieved and put in a queue of reciever which can then check them out one by one
    pressing takes you back
    if user is viewing animation no animation can be recieved, they will automatically be placed in queue, otherwise will be shown fullscreen 
    may need text on top saying * other person sent: * to make it clear it was not chosen by accident 
    if device is sleeping, go to queue
*/

SpriteAnim deerJumpAnim   = { (const uint16_t*)jumpdeerData,   DEER_W, DEER_H, 16, DEER_TRANSPARENT_KEY, 80 };
SpriteAnim deermusicAnim  = { (const uint16_t*)musicdeerData,  DEER_W, DEER_H, 13, DEER_TRANSPARENT_KEY, 80 };
SpriteAnim deernoAnim     = { (const uint16_t*)nodeerData,     DEER_W, DEER_H, 17, DEER_TRANSPARENT_KEY, 80 };
SpriteAnim deerrubAnim    = { (const uint16_t*)rubdeerData,    DEER_W, DEER_H, 17, DEER_TRANSPARENT_KEY, 80 };
SpriteAnim deersneezeAnim = { (const uint16_t*)sneezedeerData, DEER_W, DEER_H, 6, DEER_TRANSPARENT_KEY, 80 };
SpriteAnim deeryesAnim    = { (const uint16_t*)yesdeerData,    DEER_W, DEER_H, 9, DEER_TRANSPARENT_KEY, 80 };
SpriteAnim deeryesyesAnim = { (const uint16_t*)yesyesdeerData, DEER_W, DEER_H, 8, DEER_TRANSPARENT_KEY, 80 };


// The 7 deer reactions, same list as before.
SpriteAnim* deerPackItems[] = {
  &deerJumpAnim, &deermusicAnim, &deernoAnim, &deerrubAnim, &deersneezeAnim, &deeryesAnim, &deeryesyesAnim
};

// Wraps that list into one pack. Icon = deerJumpAnim's first frame, just
// picking one of the pack's own animations to represent it — swap this
// for a dedicated "cover" sprite if you make one later.
AnimPack deerPack = {
  &deerJumpAnim, // cover for packs, for now just first frame of first animation
  deerPackItems,
  sizeof(deerPackItems) / sizeof(deerPackItems[0])
};

// --- Adding a second pack later looks like this ---
// #include "catSomethingSprite.h"
// SpriteAnim catXAnim = { (const uint16_t*)catXData, CATX_W, CATX_H, CATX_FRAMES, CATX_TRANSPARENT_KEY, 80 };
// SpriteAnim* catPackItems[] = { &catXAnim, /* ...more... */ };
// AnimPack catPack = { &catXAnim, catPackItems, sizeof(catPackItems) / sizeof(catPackItems[0]) };

// The top-level list menuBegin() actually uses. Add a pack later by
// declaring it above, then adding it to this array — nothing else changes.
AnimPack* allPacks[] = { &deerPack /*, &catPack */ };
const int NUM_PACKS = sizeof(allPacks) / sizeof(allPacks[0]);

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    
    displayQueueBegin();

    screenInit();   // still needed for tft.begin()/setRotation — see earlier note
    menuBegin(allPacks, NUM_PACKS, ENCODER_CLK, ENCODER_DT, ENCODER_SW);
    mqttSetup();
}

void loop() {
    clientLoop();
    menuUpdate();
}