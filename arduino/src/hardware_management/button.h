#ifndef BUTTON_H
#define BUTTON_H
#include <Arduino.h>
#include "../types/state_types.h"

#define MAX_BUTTONS 3
#define SIMULTANEOUS_PRESS_WINDOW 100  // 100ms window for simultaneous pressure

void initButtons(int pin1, int pin2, int pin3);
void checkAllButtons();
void checkButton(int buttonIndex);
bool isButtonPressed(int buttonIndex);
void resetButton(int buttonIndex);
bool areButtonsPressedSimultaneously(int btn1, int btn2, unsigned long windowMs);
unsigned long getButtonPressStartTime(int buttonIndex);
int getButtonPin(int buttonIndex);  // New accessor function

#endif