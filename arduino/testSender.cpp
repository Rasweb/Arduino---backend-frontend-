#include "testSender.h"

void testSenderInit(int pin1, int pin2, int pin3, int ledPin){
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime) < 5000) {
    // Wait for serial connection
        delay(10);
    }
  
    // Initialize system // put in led pin here
    testSystem_init(pin1, pin2, pin3, ledPin);
}