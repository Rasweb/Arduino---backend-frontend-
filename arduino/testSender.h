#ifndef TESTSENDER_H
#define TESTSENDER_H
#include <WiFi.h>
#include <Arduino.h> 
#include "src/state_management/test_states.h"
#include "arduino_secrets.h"
#include "src/hardware_management/LED.h"
#include "src/hardware_management/button.h"
#include "src/flash_management/flash-conf.h"
#include "src/wifi_management/test_wifi.h"
#include "src/system_management/test_system_core.h"
#include "src/system_management/test_system_helpers.h"
#include "src/state_management/test_global_variables.h"
#include "src/sync_management/test_sync_manager.h"
#include "src/types/state_types.h"
#include "src/logger/logger.h"

void testSenderInit(int pin1, int pin2, int pin3, int ledPin);
void syncToServer();
// Maybe for future functionality
// void addTest();

#endif