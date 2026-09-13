#ifndef DISPLAY_QUEUE_H
#define DISPLAY_QUEUE_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// One queued command. Kept small and copyable — FreeRTOS queues copy
// data in/out, they don't pass pointers to shared memory.
struct DisplayCommand {
  int packIndex;
  int animIndex;
};

// Declared here, defined once in DisplayQueue.cpp. `extern` tells every
// other file "this exists somewhere, don't allocate it here too."
extern QueueHandle_t displayQueue;

// Call once from setup() before anything tries to use the queue.
void displayQueueBegin();

#endif