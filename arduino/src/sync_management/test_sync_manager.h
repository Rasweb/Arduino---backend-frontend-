#ifndef TEST_SYNC_MANAGER_H
#define TEST_SYNC_MANAGER_H

#include <WiFi.h>
#include "../../arduino_secrets.h"
#include "../state_management/test_states.h"
#include "../wifi_management/test_wifi.h"
#include "../flash_management/flash-conf.h"
#include "../csv_management/csv_parser.h"
#include "../hardware_management/LED.h"
#include "../csv_management/test_csv_scanner.h"
#include "../csv_management/test_csv_updater.h"
#include "../types/state_types.h"
#include "../state_management/test_global_variables.h"
#include "../logger/logger.h"

void startNonBlockingSync();
void updateNonBlockingSyncStateMachine(unsigned long currentTime);
void stopNonBlockingSync();


#endif