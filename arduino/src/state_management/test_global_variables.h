#ifndef TEST_GLOBAL_VARIABLES_H
#define TEST_GLOBAL_VARIABLES_H
#include <Arduino.h>
#include <WiFi.h>
#include "../wifi_management/test_wifi.h"
#include "../flash_management/flash-conf.h"
#include "../csv_management/csv_parser.h"
#include "../hardware_management/LED.h"
#include "../hardware_management/button.h"
#include "../../arduino_secrets.h"
#include "../types/state_types.h"
#include "../logger/logger.h"

// Forward declaration
#define MAX_PENDING_TESTS 50
#define MAX_UNSENT_TESTS 500

extern WiFiClient syncClient; // Declare syncClient as extern

TestState getCurrentState();
void setCurrentState(TestState newState);

int getPendingTestCount();
bool getHasPendingData();
void setHasPendingData(bool value);

unsigned long getLastButtonPressTime();
void setLastButtonPressTime(unsigned long time);

unsigned long getTestStartTime();
void setTestStartTime(unsigned long time);
int getTestsCompleted();
void setTestsCompleted(int count);
int getTestSessionCounter();
void setTestSessionCounter(int counter);

// Pending tests accessors
bool getPendingTest(int index, char test_id[], bool* productA_pressed, bool* productB_pressed, unsigned long* timestamp);
bool addPendingTest(const char test_id[], bool productA_pressed, bool productB_pressed, unsigned long timestamp);
void clearPendingTests();

// Simultaneous state accessors
SimultaneousState getSimultaneousState();
void setSimultaneousState(SimultaneousState state);
unsigned long getSimultaneousCooldownStart();
void setSimultaneousCooldownStart(unsigned long time);

// Free text accessors
void setFreeText(const char text[]);
void getFreeText(char result[]);
void clearFreeText();


// Sync state accessors
bool getIsSyncing();
void setIsSyncing(bool syncing);
int getCurrentSyncTest();
void setCurrentSyncTest(int test);
unsigned long getLastSyncAttempt();
void setLastSyncAttempt(unsigned long time);
bool getWifiEnabled();
void setWifiEnabled(bool enabled);
SyncState getSyncState();
void setSyncState(SyncState state);
unsigned long getSyncStateStartTime();
void setSyncStateStartTime(unsigned long time);

// CSV-based sync accessors
int getUnsentLineNumber(int index);
int getUnsentLineCount();
void setUnsentLineCount(int count);
bool getCsvSyncEnabled();
void setCsvSyncEnabled(bool enabled);

// NEW: Function to set individual unsent line number in the array
void setUnsentLineNumber(int index, int lineNumber);

// NEW: Button lock mechanism to prevent race conditions
bool getButtonProcessingLock();
void setButtonProcessingLock(bool locked);
unsigned long getButtonLockTime();
void setButtonLockTime(unsigned long time);

// Atomic button lock system
bool acquireButtonProcessingLock(); // CHANGED: Remove parameter to match implementation
void releaseButtonProcessingLock(); // ADD THIS LINE - missing declaration
bool getButtonProcessingLock();
void setButtonProcessingLock(bool state);
unsigned long getButtonLockTime();
void setButtonLockTime(unsigned long time);
#endif