#include "LED.h"
#include "../logger/logger.h"

// Blocking flash - KEEP FOR COMPATIBILITY BUT MARK AS DEPRECATED
void setLED(bool state) {
  digitalWrite(LED_PIN, state ? HIGH : LOW);
}

void blinkErrorCode(int errorCode) {
  // Convert error code blinking to non-blocking
  LOG_DEBUG(">>> Starting non-blocking error blink");
  startBlinkLEDNonBlocking(errorCode * 2, 500); // Double for on+off cycles
}

// Non-blocking flash with millis - USE THESE INSTEAD
static int blinkCount = 0;
static unsigned long blinkStartTime = 0;
static bool blinkState = false;
static int blinkTimesRemaining = 0;  // ADDED: Missing variable declaration
static int blinkDelayMs = 0;         // ADDED: Missing variable declaration  
static bool isBlinking = false;

void startBlinkLEDNonBlocking(int times, int delayMs) {
    blinkTimesRemaining = times;  // Remove the *2 multiplication
    blinkDelayMs = delayMs;
    blinkStartTime = millis();
    blinkState = true;
    isBlinking = true;
    setLED(true);
    
    LOG_DEBUG(">>> LED: Non-blocking blink started (" + String(times) + " times)");
}

void updateBlinkLEDNonBlocking() {
    if (!isBlinking || blinkTimesRemaining <= 0) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    if ((currentTime - blinkStartTime) >= blinkDelayMs) {
        blinkStartTime = currentTime;
        
        if (blinkState) {
            // Turn off LED after on period
            blinkState = false;
            setLED(false);
        } else {
            // Turn on LED and decrement counter
            blinkState = true;
            setLED(true);
            blinkTimesRemaining--;
        }
        
        // Check if blinking is complete
        if (blinkTimesRemaining <= 0) {
            setLED(false);
            isBlinking = false;
            LOG_DEBUG(">>> LED: Non-blocking blink completed");
        }
    }
}

// New function to check if blinking is active
bool isLEDBlinking() {
    return isBlinking;
}

// New function to stop blinking immediately
void stopLEDBlinking() {
    blinkTimesRemaining = 0;
    isBlinking = false;
    setLED(false);
    LOG_DEBUG(">>> LED: Blinking stopped");
}