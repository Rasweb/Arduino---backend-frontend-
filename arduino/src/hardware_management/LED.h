#ifndef LED_H
#define LED_H
#include <Arduino.h>
extern const int LED_PIN;

void setLED(bool state);
void blinkErrorCode(int errorCode);

// Non-blocking flash - USE THESE
void startBlinkLEDNonBlocking(int times, int delayMs);
void updateBlinkLEDNonBlocking();
bool isLEDBlinking(); // Check if blinking is active
void stopLEDBlinking(); // Stop blinking immediately

#endif