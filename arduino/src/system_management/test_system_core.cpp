#include "test_system_core.h"

// Timing constants
const unsigned long BUTTON_LOCK_TIMEOUT = 1000; // 1 second max lock time

// Static variables for this file
static unsigned long lastWiFiCheck = 0;

extern unsigned long SYNC_RETRY_DELAY;
extern unsigned long WIFI_CHECK_INTERVAL;
extern unsigned long WIFI_CHECK_INTERVAL_TESTING;
extern unsigned long DEBUG_INTERVAL;
extern unsigned long AUTO_SYNC_CHECK_INTERVAL;
extern unsigned long lastButtonCheck;
extern unsigned long DEBOUNCE_TIME;

void testSystem_init(int btnPin1, int btnPin2, int btnPin3, int ledPin) {
    delay(1000);
   
    LOG_INFO("==================================");
    LOG_INFO(">>> SYSTEM: Initializing components...");
    LOG_INFO("==================================");
   
    LOG_INFO(">>> SYSTEM: Initializing Flash Storage...");
    if (!initFlash()) {
        LOG_ERROR(">>> SYSTEM CRITICAL ERROR: Flash storage failed!");
        while(1) {
            blinkErrorCode(1);
            delay(2000);
        }
    }
    LOG_INFO(">>> Flash storage ready");
   
    LOG_INFO(">>> SYSTEM: Initializing buttons...");
    pinMode(ledPin, OUTPUT); //    pinMode(12, OUTPUT);  
    initButtons(btnPin1, btnPin2, btnPin3);
    LOG_INFO(">>> Buttons ready");
   
    // Use non-blocking LED initialization
    startBlinkLEDNonBlocking(1, 500);
    LOG_INFO(">>> LED ready (non-blocking)");
   
    // NEW: Initialize CSV-based sync system
    LOG_INFO(">>> SYSTEM: Initializing CSV-based sync...");
    initializeCSVSync();
    setCsvSyncEnabled(true); // Enable CSV-based sync
    LOG_INFO(">>> CSV sync system ready");
   
    LOG_INFO(">>> SYSTEM: Initializing WiFi (non-blocking)...");
    setWifiEnabled(true); // Enable WiFi but don't connect immediately
   
    // FIXED: Initialize button lock system
    setButtonProcessingLock(false); // Ensure lock starts in released state
    setButtonLockTime(0);
   
    LOG_INFO(">>> SYSTEM: All components initialized");
    LOG_INFO("==================================");
    LOG_INFO("Commands: read, json, upload, sync, wifi, state, help, reset, status");
    LOG_INFO("NEW: Using non-blocking WiFi and CSV-based sync system");
}

static unsigned long lastDebug = 0;
// static unsigned long lastButtonCheck = 0;
static unsigned long lastAutoSyncCheck = 0;

void testSystem_update() {
    unsigned long currentTime = millis();

    // Non-blocking LED updates - ALWAYS RUN
    updateBlinkLEDNonBlocking();

    // FIXED: Atomic button lock timeout check - PREVENT DEADLOCKS
    if (getButtonProcessingLock() && (currentTime - getButtonLockTime() > BUTTON_LOCK_TIMEOUT)) {
        LOG_WARN(">>> BUTTON LOCK: Timeout - forcing atomic release");
        setButtonProcessingLock(false);
    }

    // Button handling - ALWAYS RUN (never blocked by WiFi/sync)
    if (currentTime - lastButtonCheck > DEBOUNCE_TIME) {
        checkAllButtons();
        lastButtonCheck = currentTime;
       
        // STEP 1: Check for simultaneous presses FIRST (if not locked)
        if (!getButtonProcessingLock()) {
            updateSimultaneousStateMachine(currentTime);
        }
       
        // STEP 2: Then check individual buttons (only if not locked and no simultaneous cooldown)
        if (!getButtonProcessingLock() && getSimultaneousState() != SIMULTANEOUS_COOLDOWN) {
            for (int i = 0; i < MAX_BUTTONS; i++) {
                // if(i == 0) continue;
                if (isButtonPressed(i)) {
                    LOG_DEBUG(">>> BUTTON PRESSED IN MAIN LOOP: " + String(i));
                    handleButtonPress(i, currentTime);
                    resetButton(i);
                    break; // IMPORTANT: Only process one button per cycle to prevent race conditions
                }
            }
        }
    }
   
    // State machine - ALWAYS RUN (includes non-blocking WiFi)
    updateStateMachine(currentTime);


    // if (getIsSyncing() && getSyncState() != SYNC_STATE_IDLE) {
        updateNonBlockingSyncStateMachine(currentTime);
    // }
   
    // Auto-sync check - start sync automatically when idle and WiFi connected
    if (currentTime - lastAutoSyncCheck > AUTO_SYNC_CHECK_INTERVAL) {
        lastAutoSyncCheck = currentTime;
        if (shouldAttemptSync(currentTime)) {
            LOG_INFO(">>> AUTO-SYNC: Starting automatic sync...");
            startNonBlockingSync();
            setSyncStateStartTime(currentTime);
        } 
    }
   
    // WiFi management (non-blocking)
    if (currentTime - lastWiFiCheck > 1000) {
        manageWiFiConnection(currentTime);
        lastWiFiCheck = currentTime;
    }
   
    // Debug info
    if (currentTime - lastDebug > DEBUG_INTERVAL) {
        printDebugInfo(currentTime);
        lastDebug = currentTime;
    }
   
    handleIncomingData();
}