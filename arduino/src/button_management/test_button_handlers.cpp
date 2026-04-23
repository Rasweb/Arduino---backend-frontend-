#include "test_button_handlers.h"

// Button press handler - processes button events based on current system state
void handleButtonPress(int buttonIndex, unsigned long currentTime) {
    // FIXED: Use atomic lock acquisition instead of separate check/set
    if (!acquireButtonProcessingLock()) {
        LOG_INFO(">>> BUTTON IGNORED: Processing locked (atomic)");
        return;
    }
    
    LOG_INFO(">>> BUTTON PRESS: Processing button " + String(buttonIndex) + 
             " in state " + String(getCurrentState()));


    // Green button (index 0) - Start/Stop test session
    if (buttonIndex == 0) {
        if (getCurrentState() == STATE_IDLE || getCurrentState() == STATE_TEST_COMPLETED) {
            LOG_INFO(">>> STARTING TEST SESSION - Green button pressed!");
            
            // Stop any ongoing sync and prepare for new session
            stopNonBlockingSync();
            // setCurrentState(STATE_WAITING_FOR_WIFI);
            setCurrentState(STATE_TEST_RUNNING);
            setTestSessionCounter(getTestSessionCounter() + 1);
            setTestsCompleted(0);
            setSimultaneousState(SIMULTANEOUS_IDLE);
            testTime = millis(); // for arduino.ino
            
            LOG_INFO(">>> New session ready - " + String(getUnsentLineCount()) + " unsent tests in CSV");
            
            // FIXED: Use release function instead of direct set
            releaseButtonProcessingLock();
            return;
        }
        else if (getCurrentState() == STATE_TEST_RUNNING) {
            // Force end session if green button pressed during test
            LOG_INFO(">>> FORCE ENDING SESSION - Green button pressed during test!");
            setCurrentState(STATE_TEST_COMPLETED);
            
            // FIXED: Use release function instead of direct set
            releaseButtonProcessingLock();
            return;
        }
        else {
            LOG_INFO(">>> Green button ignored - system busy");
            
            // FIXED: Use release function instead of direct set
            releaseButtonProcessingLock();
            return;
        }
    }


    // Product buttons (RED=1, BLUE=2) - only process during active test session
    if (getCurrentState() == STATE_TEST_RUNNING) {
        if (buttonIndex == 1 || buttonIndex == 2) {
            // Button lock is already acquired atomically at function start
            
            // Increment test counter and update timing
            setTestsCompleted(getTestsCompleted() + 1);
            setLastButtonPressTime(currentTime);
            bool productA = (buttonIndex == 1);
            bool productB = (buttonIndex == 2);
            
            // Log which product was tested
            LOG_INFO(">>> PRODUCT TEST: ");
            if (productA) {
                LOG_INFO("PRODUCT A (RED)");
            } else {
                LOG_INFO("PRODUCT B (BLUE)");
            }
            LOG_INFO(">>> Test " + String(getTestsCompleted()) + "/10");

            // Generate unique test ID
            char test_id[64];
            int charsWritten = snprintf(test_id, sizeof(test_id), "%s_%lu", DEVICE_ID, currentTime);
            if (charsWritten >= sizeof(test_id)) {
                LOG_WARN(">>> WARNING: test_id truncated due to buffer size");
                test_id[sizeof(test_id) - 1] = '\0';
            }
            
            // Save test data to CSV and provide visual feedback
            saveTestToCSV(test_id, productA, productB, currentTime);
            startBlinkLEDNonBlocking(3, 100);
            
            // FIXED: Use atomic lock time setting (NON-BLOCKING)
            unsigned long lockReleaseTime = currentTime + 300; // 300ms lock duration
            setButtonLockTime(lockReleaseTime);
            
            LOG_INFO(">>> BUTTON LOCK: Will auto-release after 300ms");
            
            // FIXED: DO NOT release lock here - let timeout system handle it
            // This prevents race conditions between button presses
            return;
        }


        // Check if test session is complete (10 tests or timeout)
        if (getTestsCompleted() >= MAX_TESTS_PER_SESSION) {
            LOG_INFO(">>> MAX TESTS REACHED - Ending session");
            setCurrentState(STATE_TEST_COMPLETED);
            setSimultaneousState(SIMULTANEOUS_IDLE);
            
            // FIXED: Release lock when session ends
            releaseButtonProcessingLock();
        }
    } else {
        // Ignore product buttons when not in test running state
        LOG_INFO(">>> Button " + String(buttonIndex) + " ignored - not in TEST_RUNNING state");
        
        // FIXED: Use release function instead of direct set
        releaseButtonProcessingLock();
    }
}


// FIXED: Simultaneous button press state machine with improved detection
void updateSimultaneousStateMachine(unsigned long currentTime) {
    // Only process simultaneous presses during active test session
    if (getCurrentState() != STATE_TEST_RUNNING) {
        if (getSimultaneousState() != SIMULTANEOUS_IDLE) {
            setSimultaneousState(SIMULTANEOUS_IDLE);
        }
        return;
    }


    // FIXED: Use atomic lock check - if locked, skip simultaneous processing
    if (getButtonProcessingLock()) {
        // Debug log for simultaneous press skipped due to lock
        static unsigned long lastLockLog = 0;
        if (currentTime - lastLockLog > 1000) {
            LOG_INFO(">>> SIMULTANEOUS: Skipped due to button lock");
            lastLockLog = currentTime;
        }
        return;
    }


    // Check if both buttons are currently pressed
    bool button1Pressed = isButtonPressed(1);
    bool button2Pressed = isButtonPressed(2);
    bool bothPressed = button1Pressed && button2Pressed;
    
    // DEBUG: Log button states for troubleshooting
    static unsigned long lastDebugLog = 0;
    if (currentTime - lastDebugLog > 5000) { // Log every 5 seconds
        LOG_DEBUG(">>> BUTTON DEBUG: B1=" + String(button1Pressed) + 
                  ", B2=" + String(button2Pressed) + 
                  ", Both=" + String(bothPressed) + 
                  ", State=" + String(getSimultaneousState()));
        lastDebugLog = currentTime;
    }


    // State machine for simultaneous button handling
    switch(getSimultaneousState()) {
        case SIMULTANEOUS_IDLE:
            // FIXED: Check if both buttons were pressed within the simultaneous window (increased to 200ms)
            if (bothPressed && areButtonsPressedSimultaneously(1, 2, 200)) {
                // FIXED: Use atomic lock acquisition
                if (!acquireButtonProcessingLock()) {
                    LOG_INFO(">>> SIMULTANEOUS: Could not acquire lock, skipping");
                    return;
                }
                
                // Register simultaneous press as a test completion
                setTestsCompleted(getTestsCompleted() + 1);
                setLastButtonPressTime(currentTime);
                
                LOG_INFO(">>> PRODUCT TEST: BOTH PRODUCTS");
                LOG_INFO(">>> Test " + String(getTestsCompleted()) + "/10");

                // Generate unique test ID for simultaneous press
                char test_id[64];
                int charsWritten = snprintf(test_id, sizeof(test_id), "%s_%lu_BOTH", DEVICE_ID, currentTime);
                if (charsWritten >= sizeof(test_id)) {
                    LOG_WARN(">>> WARNING: test_id truncated due to buffer size");
                    test_id[sizeof(test_id) - 1] = '\0';
                }
                
                // Save both products pressed to CSV
                saveTestToCSV(test_id, true, true, currentTime);
                startBlinkLEDNonBlocking(3, 100);


                // Enter cooldown state to prevent multiple detections
                setSimultaneousState(SIMULTANEOUS_COOLDOWN);
                setSimultaneousCooldownStart(currentTime);
                
                LOG_INFO(">>> Simultaneous press handled - entering cooldown");
                
                // FIXED: Don't reset buttons immediately - let the normal button handling do it
                // This prevents missing button releases
                
                // FIXED: Use atomic lock time setting with longer duration for simultaneous
                unsigned long lockReleaseTime = currentTime + 500; // 500ms lock for simultaneous
                setButtonLockTime(lockReleaseTime);
                LOG_INFO(">>> BUTTON LOCK: Simultaneous - will auto-release after 500ms");
                
                // FIXED: DO NOT release lock here - let timeout system handle it
            }
            break;
            
        case SIMULTANEOUS_COOLDOWN:
            // Wait for cooldown period before allowing another simultaneous press
            if (currentTime - getSimultaneousCooldownStart() > SIMULTANEOUS_COOLDOWN_MS) {
                setSimultaneousState(SIMULTANEOUS_IDLE);
                LOG_INFO(">>> Simultaneous cooldown finished");
            }
            break;
    }
}