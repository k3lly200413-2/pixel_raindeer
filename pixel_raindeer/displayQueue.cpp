#include "displayQueue.h"

// The actual storage for the queue handle — this is the ONE place it's
// defined. Every other file just sees the `extern` declaration.
QueueHandle_t displayQueue;

void displayQueueBegin() {
  displayQueue = xQueueCreate(5, sizeof(DisplayCommand));
}