#ifndef TEST_BUTTON_HANDLERS_H
#define TEST_BUTTON_HANDLERS_H
#include "../csv_management/csv_parser.h"
#include "../hardware_management/LED.h"
#include "../wifi_management/test_wifi.h"
#include "../../arduino_secrets.h"
#include "../state_management/test_global_variables.h"
#include <Arduino.h>
#include <WiFi.h>
#include "../flash_management/flash-conf.h"
#include "../state_management/test_states.h"
#include "../data_processor_management/test_data_processor.h"
#include "../sync_management/test_sync_manager.h"
#include "../logger/logger.h"

#define MAX_TESTS_PER_SESSION 10
#define SIMULTANEOUS_COOLDOWN_MS 500

void updateSimultaneousStateMachine(unsigned long currentTime);
void handleButtonPress(int buttonIndex, unsigned long currentTime);

#endif