#include "button.h"
#include "../logger/logger.h"

static btn_comp buttons[MAX_BUTTONS] = {
  {10, false, false, 0, 50, 0},  // btnStart - GREEN
  {13, false, false, 0, 50, 0},  // btnProductA - RED  
  {11, false, false, 0, 50, 0}   // btnProductB - BLUE
};

void initButtons(int pin1, int pin2, int pin3) {
  buttons[0].pin = pin1;
  buttons[1].pin = pin2;
  buttons[2].pin = pin3;
  for (int i = 0; i < MAX_BUTTONS; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    LOG_DEBUG(">>> BUTTON: Initialized pin " + String(buttons[i].pin));
  }
}

void checkAllButtons() {
  for (int i = 0; i < MAX_BUTTONS; i++) {
    checkButton(i);
  }
}

void checkButton(int buttonIndex) {
  if (buttonIndex < 0 || buttonIndex >= MAX_BUTTONS) return;
 
  int reading = digitalRead(buttons[buttonIndex].pin);
  unsigned long currentTime = millis();

  // DEBOUNCE LOGIC: Only process if enough time has passed since last press
  if ((currentTime - buttons[buttonIndex].lastPressTime) < buttons[buttonIndex].debounceDelay) {
    return;
  }

  // BUTTON PRESSED (LOW because of INPUT_PULLUP)
  if (reading == LOW) {
    if (!buttons[buttonIndex].pressed) {
      // New press detected
      buttons[buttonIndex].pressed = true;
      buttons[buttonIndex].lastPressTime = currentTime;
      buttons[buttonIndex].pressStartTime = currentTime;
      buttons[buttonIndex].waitingForRelease = false; // Reset waiting flag
      LOG_DEBUG(">>> BUTTON PRESSED: " + String(buttonIndex));
    }
  } 
  // BUTTON RELEASED (HIGH)
  else if (reading == HIGH) {
    if (buttons[buttonIndex].pressed) {
      // Button was pressed and is now released
      buttons[buttonIndex].pressed = false;
      buttons[buttonIndex].waitingForRelease = true; // Set waiting flag on release
      buttons[buttonIndex].pressStartTime = 0;
      LOG_DEBUG(">>> BUTTON RELEASED: " + String(buttonIndex));
    } else if (buttons[buttonIndex].waitingForRelease) {
      // Clear waiting flag when button is fully released
      buttons[buttonIndex].waitingForRelease = false;
      LOG_DEBUG(">>> BUTTON WAITING CLEARED: " + String(buttonIndex));
    }
  }
}

bool isButtonPressed(int buttonIndex) {
  if (buttonIndex < 0 || buttonIndex >= MAX_BUTTONS) return false;
  
  // IMPORTANT CHANGE: Return true only if button is physically pressed AND not waiting for release
  // This allows simultaneous detection while preventing repeated triggers
  return buttons[buttonIndex].pressed && !buttons[buttonIndex].waitingForRelease;
}

void resetButton(int buttonIndex) {
  if (buttonIndex < 0 || buttonIndex >= MAX_BUTTONS) return;
  
  // Only reset if button is not physically pressed
  if (digitalRead(buttons[buttonIndex].pin) == HIGH) {
    buttons[buttonIndex].pressed = false;
    buttons[buttonIndex].waitingForRelease = false;
    buttons[buttonIndex].pressStartTime = 0;
    LOG_DEBUG(">>> BUTTON FULLY RESET: " + String(buttonIndex));
  } else {
    // If button is still pressed, just set waitingForRelease to prevent repeated triggers
    buttons[buttonIndex].waitingForRelease = true;
    LOG_DEBUG(">>> BUTTON SET TO WAITING (still pressed): " + String(buttonIndex));
  }
}

// Check if two buttons were pressed at the same time
bool areButtonsPressedSimultaneously(int btn1, int btn2, unsigned long windowMs) {
  // IMPORTANT CHANGE: Use the new isButtonPressed that considers waitingForRelease
  if (!isButtonPressed(btn1) || !isButtonPressed(btn2)) {
    return false;
  }
 
  unsigned long time1 = buttons[btn1].pressStartTime;
  unsigned long time2 = buttons[btn2].pressStartTime;
 
  if (time1 == 0 || time2 == 0) {
    return false;
  }
 
  unsigned long timeDiff = (time1 > time2) ? (time1 - time2) : (time2 - time1);
 
  return (timeDiff <= windowMs);
}

// Retrieve when the button was first pressed
unsigned long getButtonPressStartTime(int buttonIndex) {
  if (buttonIndex < 0 || buttonIndex >= MAX_BUTTONS) return 0;
  return buttons[buttonIndex].pressStartTime;
}

// New accessor function to get button pin
int getButtonPin(int buttonIndex) {
  if (buttonIndex < 0 || buttonIndex >= MAX_BUTTONS) return -1;
  return buttons[buttonIndex].pin;
}