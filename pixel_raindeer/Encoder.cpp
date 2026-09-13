#include "Encoder.h"

// File-scope (not visible outside this .cpp) state for the currently
// configured encoder. Only one encoder is supported at a time with this
// design — fine for this project, since there's only one physical knob.
static int pinCLK, pinDT, pinSW;

// `volatile` is required here because this variable is written inside an
// interrupt handler (handleEncoderISR) and read from normal code
// (encoderRead()) — without volatile, the compiler might cache a stale
// value in a register and never notice the ISR changed it.
static volatile int rawDelta = 0;
static volatile int lastCLKState;

static bool lastSwState = HIGH;         // previous button reading, for edge-detecting a fresh press
static unsigned long lastClickTime = 0; // timestamp of the last accepted click, for debouncing

// Interrupt Service Routine — runs automatically whenever the CLK pin's
// voltage changes (see attachInterrupt in encoderBegin). IRAM_ATTR keeps
// this function's code in fast internal RAM rather than flash, which ISRs
// on the ESP32 require for reliable timing.
static void IRAM_ATTR handleEncoderISR() {
  int clkState = digitalRead(pinCLK);
  if (clkState != lastCLKState) {          // guards against the ISR firing for a change we already processed
    int dtState = digitalRead(pinDT);
    // Comparing DT's state to CLK's state at this exact instant is the
    // core trick of quadrature decoding: which pin "leads" the other
    // tells you the direction of rotation.
    rawDelta += (dtState != clkState) ? -1 : 1;
  }
  lastCLKState = clkState;
}

// Public entry point — call once from setup().
void encoderBegin(int clkPin, int dtPin, int swPin) {
  pinCLK = clkPin;
  pinDT = dtPin;
  pinSW = swPin;

  // INPUT_PULLUP: enables the ESP32's internal pull-up resistor, so each
  // pin reads HIGH by default and only goes LOW when the encoder actively
  // pulls it down. Matches how these encoder modules are wired (they
  // connect to ground when active, not to power).
  pinMode(pinCLK, INPUT_PULLUP);
  pinMode(pinDT, INPUT_PULLUP);
  pinMode(pinSW, INPUT_PULLUP);

  lastCLKState = digitalRead(pinCLK); // capture the starting state before interrupts are live, so the first real change is detected correctly

  // Attach the ISR to fire on ANY change (rising or falling) of the CLK
  // pin. digitalPinToInterrupt() converts a plain GPIO number into
  // whatever internal interrupt ID the platform needs.
  attachInterrupt(digitalPinToInterrupt(pinCLK), handleEncoderISR, CHANGE);
}

// Public entry point — call anytime (typically once per loop()).
// Returns how many full detent "clicks" the knob turned since the last
// call: positive for one direction, negative for the other, usually 0.
int encoderRead() {
  // Briefly disable interrupts while we snapshot and reset rawDelta —
  // otherwise the ISR could fire in the middle of this read/reset and
  // corrupt the value (a classic race condition with shared variables).
  noInterrupts();
  int delta = rawDelta;
  rawDelta = 0;
  interrupts();

  // This encoder produces 2 CLK transitions per physical detent click
  // (confirmed from Serial log). We accumulate raw transitions and only
  // report a step once a full detent's worth has arrived.
  static int accum = 0;    // persists between calls (static local) — carries over leftover "partial" transitions
  accum += delta;
  int steps = accum / 2;   // integer division: how many complete detents' worth we've accumulated
  accum -= steps * 2;      // keep the remainder for next time, so partial transitions aren't lost
  return steps;
}

// Public entry point — call anytime (typically once per loop()).
// Returns true exactly once per confirmed button press (not held-down —
// you get one `true` per press, not a stream of them).
bool encoderClicked() {
  bool state = digitalRead(pinSW);   // LOW = currently pressed (because of the pull-up wiring)
  unsigned long now = millis();
  bool clicked = false;
  // Only count it as a click if: it's currently pressed, it WASN'T
  // pressed last time we checked (i.e. this is the moment of pressing,
  // not still being held), AND enough time has passed since the last
  // accepted click (200ms) — that last condition is debouncing, filtering
  // out the rapid electrical "bounce" a physical switch produces.
  if (state == LOW && lastSwState == HIGH && (now - lastClickTime) > 200) {
    clicked = true;
    lastClickTime = now;
  }
  lastSwState = state; // remember this reading for next call's comparison
  return clicked;
}