
#ifndef TEST_STATES_H
#define TEST_STATES_H
#include <WiFi.h>
#include <Arduino.h>
#include "../types/state_types.h"
#include "../wifi_management/test_wifi.h"
#include "../flash_management/flash-conf.h"
#include "../hardware_management/LED.h"
#include "../sync_management/test_sync_manager.h"
#include "../../arduino_secrets.h"
#include "../state_management/test_global_variables.h"
#include "../logger/logger.h"

extern unsigned long testTime;

// State enter/exit function declarations
void onEnterIdle();
void onEnterWaitingForWifi();
void onEnterTestRunning();
void onEnterTestCompleted();
void onExitIdle();
void onExitWaitingForWifi();
void onExitTestRunning();
void onExitTestCompleted();

// State handler function declarations
void handleIdleState();
void handleWaitingForWifiState();
void handleTestRunningState();
void handleTestCompletedState();

// NEW: WiFi state machine functions - UPDATED RETURN TYPE
void updateWiFiStateMachine(unsigned long currentTime);
void startWiFiConnection();
void stopWiFiConnection();
bool shouldAttemptWiFiConnection(unsigned long currentTime);
bool shouldDisconnectWiFi(unsigned long currentTime);
WiFiState getCurrentWiFiState(); // CHANGED: Return enum instead of int

// Main state machine function
void updateStateMachine(unsigned long currentTime);

#endif