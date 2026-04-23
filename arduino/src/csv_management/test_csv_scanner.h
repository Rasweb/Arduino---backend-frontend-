#ifndef TEST_CSV_SCANNER_H
#define TEST_CSV_SCANNER_H

#include <WiFi.h>
#include <Arduino.h>
#include "../wifi_management/test_wifi.h"
#include "../flash_management/flash-conf.h"
#include "../csv_management/csv_parser.h"
#include "../hardware_management/LED.h"
#include "../../arduino_secrets.h"
#include "../state_management/test_global_variables.h"
#include "../state_management/test_states.h"
#include "../types/state_types.h"
#include "../logger/logger.h"


// Optimized scanning function for fast unsent test detection
testData readCompleteTestDataFromCSV(int lineNumber);
void findUnsentTestsFast();
void findUnsentTestsFromCSV();
int getUnsentTestCount();
void initializeCSVSync();


#endif