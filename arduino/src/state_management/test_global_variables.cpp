#include "test_global_variables.h"

// GLOBAL VARIABLES DEFINITIONS - NOW STATIC AND PRIVATE (ONLY DEFINED HERE)
// These variables maintain the system state and are accessed through accessor functions

// Test system state variables
static TestState currentState = STATE_IDLE;                          // Current system state (idle, running, etc.)
static unsigned long lastButtonPressTime = 0;                        // Timestamp of last button press
static unsigned long testStartTime = 0;                              // When the current test session started
static int testsCompleted = 0;                                       // Number of tests completed in current session
static int testSessionCounter = 0;                                   // Total test sessions run
static bool hasPendingData = false;                                  // Flag indicating unsynced data exists

// Pending tests queue for offline storage and synchronization
static struct PendingTest pendingTests[MAX_PENDING_TESTS];           // Buffer for unsent test data
static int pendingTestCount = 0;                                     // Number of tests in pending queue

// Simultaneous button press state management
static SimultaneousState simultaneousState = SIMULTANEOUS_IDLE;      // State for detecting both buttons pressed
static unsigned long simultaneousCooldownStart = 0;                  // When simultaneous cooldown started

// Free text storage for user notes
static char currentFreeText[FREE_TEXT_SIZE] = "";                    // Buffer for free text annotations

// WiFi synchronization state management
static bool isSyncing = false;                                       // Whether sync is currently in progress
static int currentSyncTest = 0;                                      // Index of test being synced
static unsigned long lastSyncAttempt = 0;                            // Last sync attempt timestamp
static bool wifiEnabled = true;                                      // Global WiFi enable/disable flag
static SyncState syncState = SYNC_STATE_IDLE;                        // Current state of sync state machine
static unsigned long syncStateStartTime = 0;                         // When current sync state started
WiFiClient syncClient;                                        // WiFi client for HTTP communication

// CSV-based synchronization variables
static int unsentLineNumbers[MAX_UNSENT_TESTS];                      // Array of line numbers with unsent data
static int unsentLineCount = 0;                                      // Number of unsent lines found
static bool csvSyncEnabled = true;                                   // Use CSV scanning (true) or queue (false)

// FIX 3: Atomic button processing lock with mutex protection
static bool buttonProcessingLock = false;
static unsigned long buttonLockTime = 0;
static bool buttonLockMutex = false;  // Mutex for atomic button lock operations

// Thread-safe button lock acquisition
static bool acquireButtonLockMutex() {
    unsigned long startTime = millis();
    const unsigned long timeout = 1000; // 1 second timeout
   
    // Wait for mutex to be released with timeout
    while (buttonLockMutex && (millis() - startTime < timeout)) {
        delay(1); // Minimal delay to prevent busy-waiting
    }
   
    if (buttonLockMutex) {
        LOG_ERROR("ERROR: Button lock mutex timeout");
        return false;
    }
   
    // Acquire mutex
    buttonLockMutex = true;
    return true;
}

// Thread-safe button lock release
static void releaseButtonLockMutex() {
    buttonLockMutex = false;
}

// GLOBAL VARIABLE ACCESSOR FUNCTIONS - IMPLEMENTATION (ONLY DEFINED HERE)
// These functions provide controlled access to the global variables

// System state accessors
TestState getCurrentState() { return currentState; }
void setCurrentState(TestState newState) { currentState = newState; }

// Test session management accessors
int getPendingTestCount() { return pendingTestCount; }
bool getHasPendingData() { return hasPendingData; }
void setHasPendingData(bool value) { hasPendingData = value; }
unsigned long getLastButtonPressTime() { return lastButtonPressTime; }
void setLastButtonPressTime(unsigned long time) { lastButtonPressTime = time; }
unsigned long getTestStartTime() { return testStartTime; }
void setTestStartTime(unsigned long time) { testStartTime = time; }
int getTestsCompleted() { return testsCompleted; }
void setTestsCompleted(int count) { testsCompleted = count; }
int getTestSessionCounter() { return testSessionCounter; }
void setTestSessionCounter(int counter) { testSessionCounter = counter; }

// Pending tests queue accessors
bool getPendingTest(int index, char test_id[], bool* productA_pressed, bool* productB_pressed, unsigned long* timestamp) {
    if (index < 0 || index >= pendingTestCount) return false;
   
    // Copy test data to output parameters
    strncpy(test_id, pendingTests[index].test_id, 63);
    test_id[63] = '\0';
    *productA_pressed = pendingTests[index].productA_pressed;
    *productB_pressed = pendingTests[index].productB_pressed;
    *timestamp = pendingTests[index].timestamp;
    return true;
}

bool addPendingTest(const char test_id[], bool productA_pressed, bool productB_pressed, unsigned long timestamp) {
    if (pendingTestCount >= MAX_PENDING_TESTS) return false;
   
    // Add new test to pending queue
    strncpy(pendingTests[pendingTestCount].test_id, test_id, 63);
    pendingTests[pendingTestCount].test_id[63] = '\0';
    pendingTests[pendingTestCount].productA_pressed = productA_pressed;
    pendingTests[pendingTestCount].productB_pressed = productB_pressed;
    pendingTests[pendingTestCount].timestamp = timestamp;
    pendingTestCount++;
    hasPendingData = true; // Set flag indicating unsynced data exists
    return true;
}

void clearPendingTests() {
    pendingTestCount = 0;
    hasPendingData = false; // Clear flag when queue is emptied
}

// Simultaneous button state accessors
SimultaneousState getSimultaneousState() { return simultaneousState; }
void setSimultaneousState(SimultaneousState state) { simultaneousState = state; }
unsigned long getSimultaneousCooldownStart() { return simultaneousCooldownStart; }
void setSimultaneousCooldownStart(unsigned long time) { simultaneousCooldownStart = time; }

// Free text management accessors
void setFreeText(const char text[]) {
    strncpy(currentFreeText, text, FREE_TEXT_SIZE - 1);
    currentFreeText[FREE_TEXT_SIZE - 1] = '\0';
    LOG_DEBUG(">>> Free text set to: '" + String(currentFreeText) + "' (will be saved in free text field)");
}

void getFreeText(char result[]) {
    strncpy(result, currentFreeText, FREE_TEXT_SIZE - 1);
    result[FREE_TEXT_SIZE - 1] = '\0';
}

void clearFreeText() {
    currentFreeText[0] = '\0';
    LOG_DEBUG(">>> Free text cleared");
}

// Synchronization state accessors
bool getIsSyncing() { return isSyncing; }
void setIsSyncing(bool syncing) { isSyncing = syncing; }
int getCurrentSyncTest() { return currentSyncTest; }
void setCurrentSyncTest(int test) { currentSyncTest = test; }
unsigned long getLastSyncAttempt() { return lastSyncAttempt; }
void setLastSyncAttempt(unsigned long time) { lastSyncAttempt = time; }
bool getWifiEnabled() { return wifiEnabled; }
void setWifiEnabled(bool enabled) { wifiEnabled = enabled; }
SyncState getSyncState() { return syncState; }
void setSyncState(SyncState state) { syncState = state; }
unsigned long getSyncStateStartTime() { return syncStateStartTime; }
void setSyncStateStartTime(unsigned long time) { syncStateStartTime = time; }

// CSV-based synchronization accessors
int getUnsentLineNumber(int index) {
    if (index < 0 || index >= unsentLineCount) return -1;
    return unsentLineNumbers[index];
}

// NEW: Function to set individual unsent line number in the array
void setUnsentLineNumber(int index, int lineNumber) {
    if (index >= 0 && index < MAX_UNSENT_TESTS) {
        unsentLineNumbers[index] = lineNumber;
    }
}

int getUnsentLineCount() { return unsentLineCount; }
void setUnsentLineCount(int count) {
    if (count >= 0 && count <= MAX_UNSENT_TESTS) {
        unsentLineCount = count;
    }
}

bool getCsvSyncEnabled() { return csvSyncEnabled; }
void setCsvSyncEnabled(bool enabled) { csvSyncEnabled = enabled; }

// FIXED: Atomic button lock mechanism accessors
bool getButtonProcessingLock() {
    // Atomic check without acquiring mutex for performance
    // This is safe for read-only checks
    return buttonProcessingLock;
}

void setButtonProcessingLock(bool locked) {
    // Atomic set operation with mutex protection
    if (!acquireButtonLockMutex()) {
        LOG_ERROR("ERROR: Could not acquire button lock mutex");
        return;
    }
   
    buttonProcessingLock = locked;
    if (locked) {
        buttonLockTime = millis();
        LOG_DEBUG(">>> BUTTON LOCK: Activated atomically");
    } else {
        LOG_DEBUG(">>> BUTTON LOCK: Released atomically");
    }
   
    releaseButtonLockMutex();
}

// FIXED: Atomic button lock check and set combined (eliminates race condition)
// CHANGED: Remove currentTime parameter to match all call sites
bool acquireButtonProcessingLock() {
    if (!acquireButtonLockMutex()) {
        return false;
    }
   
    bool acquired = false;
    if (!buttonProcessingLock) {
        buttonProcessingLock = true;
        buttonLockTime = millis(); // Use current time internally
        acquired = true;
        LOG_DEBUG(">>> BUTTON LOCK: Acquired atomically");
    } else {
        LOG_DEBUG(">>> BUTTON LOCK: Already locked, cannot acquire");
    }
   
    releaseButtonLockMutex();
    return acquired;
}

// NEW: Add missing releaseButtonProcessingLock function
void releaseButtonProcessingLock() {
    if (!acquireButtonLockMutex()) {
        LOG_ERROR("ERROR: Could not acquire button lock mutex for release");
        return;
    }
   
    buttonProcessingLock = false;
    buttonLockTime = 0;
    LOG_DEBUG(">>> BUTTON LOCK: Released atomically");
   
    releaseButtonLockMutex();
}

unsigned long getButtonLockTime() {
    return buttonLockTime;
}

void setButtonLockTime(unsigned long time) {
    if (!acquireButtonLockMutex()) {
        return;
    }
    buttonLockTime = time;
    releaseButtonLockMutex();
}