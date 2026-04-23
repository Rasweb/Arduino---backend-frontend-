
#ifndef TEST_SYSTEM_HELPERS_H
#define TEST_SYSTEM_HELPERS_H

#include <Arduino.h>
#include <WiFi.h>  
#include "../state_management/test_states.h"
#include "../state_management/test_global_variables.h"
#include "../flash_management/flash-conf.h"
#include "../wifi_management/test_wifi.h"
#include "../logger/logger.h"  

// Function declarations
bool shouldAttemptSync(unsigned long currentTime);
void manageWiFiConnection(unsigned long currentTime); 
void printDebugInfo(unsigned long currentTime);

#endif