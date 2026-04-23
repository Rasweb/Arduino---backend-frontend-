#ifndef TEST_DATA_PROCESSOR_H
#define TEST_DATA_PROCESSOR_H

#include <WiFi.h>
#include <Arduino.h>
#include "../wifi_management/test_wifi.h"
#include "../csv_management/csv_parser.h"
#include "../hardware_management/LED.h"
#include "../flash_management/flash-conf.h"
#include "../../arduino_secrets.h"
#include "../state_management/test_states.h"
#include "../state_management/test_global_variables.h"
#include "../types/state_types.h"
#include "../logger/logger.h"

// Add extern declaration for mutex functions
extern bool acquireFileLock();
extern void releaseFileLock();

bool readTestDataFromCSV(int lineNumber, char test_id[], bool productA_pressed[], bool productB_pressed[], unsigned long timestamp[]);
void saveTestToCSV(const char test_id[], bool productA_pressed, bool productB_pressed, unsigned long timestamp);
void saveOfflineTest(bool productA_pressed, bool productB_pressed, unsigned long currentTime);

#endif