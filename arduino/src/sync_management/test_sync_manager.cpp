#include "test_sync_manager.h"

// Add extern declaration for mutex functions  
extern bool acquireFileLock();
extern void releaseFileLock();

// Start non-blocking synchronization process
void startNonBlockingSync() {
    LOG_DEBUG("Attempting to start non-blocking sync");
    if (!getCsvSyncEnabled()) {
        // Use queue-based sync if CSV sync is disabled
        if (getPendingTestCount() == 0 || !getWifiEnabled()) {
            LOG_INFO(">>> CSV SYNC: No tests to sync or WiFi disabled");
            return;
        }
    } else {
        // Use CSV-based sync - scan for unsent tests
        findUnsentTestsFast(); // This function now has mutex internally
        if (getUnsentLineCount() == 0 || !getWifiEnabled()) {
            LOG_INFO(">>> CSV SYNC: No unsent tests found or WiFi disabled");
            return;
        }
    }
   
    // // FIXED: Accurate log message based on sync type
    LOG_INFO(">>> Starting NON-BLOCKING SYNC");
   
    setIsSyncing(true);
    setSyncState(SYNC_STATE_CONNECTING);

    setSyncStateStartTime(millis());
    setCurrentSyncTest(0);
    setLastSyncAttempt(millis());
}

// Stop ongoing synchronization process
void stopNonBlockingSync() {
    if (getSyncState() != SYNC_STATE_IDLE) {
        LOG_INFO(">>> Stopping ongoing sync");
        if (syncClient.connected()) {
            syncClient.stop();
        }
    }
    setSyncState(SYNC_STATE_IDLE);
    setIsSyncing(false);
    setSyncStateStartTime(0);
    LOG_INFO(">>> CSV SYNC: Sync stopped");
}

// Non-blocking sync state machine - processes one test per call to avoid blocking
void updateNonBlockingSyncStateMachine(unsigned long currentTime) {   
    if (!getIsSyncing()) {
        return;
    }

    // Check if all tests have been processed
    if (getCsvSyncEnabled()) {
        if (getCurrentSyncTest() >= getUnsentLineCount()) {
            setIsSyncing(false);
            LOG_INFO(">>> CSV SYNC: All tests processed");
            return;
        }
    } else {
        if (getCurrentSyncTest() >= getPendingTestCount()) {
            setIsSyncing(false);
            LOG_INFO(">>> CSV SYNC: All tests processed");
            return;
        }
    }
   
    // Limit processing time to avoid blocking
    unsigned long workStartTime = millis();
    const unsigned long MAX_WORK_TIME = 100;
   
    // FIXED: Move ALL variable declarations to the top of the function
    // Declare all WiFiClient variables at function scope
    String jsonData;
    String request;
    String host;
    char device_id[64];
    char test_id[64];
    bool productA_pressed[1];
    bool productB_pressed[1];
    unsigned long timestamp[1];
    bool productA_pressed_single;
    bool productB_pressed_single;
    unsigned long timestamp_single;
    int lineNumber;

    // Sync state machine
    switch(getSyncState()) {
        case SYNC_STATE_IDLE:
            // Start processing next test if available
            LOG_DEBUG("In case: SYNC_STATE_IDLE");
            if ((getCsvSyncEnabled() && getCurrentSyncTest() < getUnsentLineCount()) ||
                (!getCsvSyncEnabled() && getCurrentSyncTest() < getPendingTestCount())) {
                LOG_INFO(">>> CSV SYNC: Starting test " + String(getCurrentSyncTest() + 1) + "/" + 
                         String(getCsvSyncEnabled() ? getUnsentLineCount() : getPendingTestCount()));
               
                setSyncState(SYNC_STATE_CONNECTING);
                setSyncStateStartTime(currentTime);
            }
            break;
           
        case SYNC_STATE_CONNECTING:
            // Connection timeout handling
            if (millis() - getSyncStateStartTime() > 5000) {
                LOG_WARN(">>> CSV SYNC: Server connection timeout");
                setSyncState(SYNC_STATE_ERROR);
                break;
            }
           
            if (!syncClient.connected()) {
                host = SECRET_IP;
                if (syncClient.connect(host.c_str(), 3000)) { // This is probably blocking
                    LOG_INFO(">>> CSV SYNC: Connected to server");
                    setSyncState(SYNC_STATE_SENDING);
                    setSyncStateStartTime(currentTime);
                } else {
                    LOG_WARN(">>> CSV SYNC: Failed connecting to server");
                }
            } else {
                setSyncState(SYNC_STATE_SENDING);
                setSyncStateStartTime(currentTime);
            }
            break;
           
        case SYNC_STATE_SENDING:
            // Send timeout handling
            if (millis() - getSyncStateStartTime() > 3000) {
                LOG_INFO(">>> CSV SYNC: Send timeout");
                setSyncState(SYNC_STATE_ERROR);
                break;
            }
           
            if (syncClient.connected()) {
                jsonData = "";
               
                if (getCsvSyncEnabled()) {
                    // Read test data from CSV for synchronization
                    lineNumber = getUnsentLineNumber(getCurrentSyncTest());
                   
                    // FIXED: Validate line number before reading CSV data
                    if (lineNumber <= 0) {
                        LOG_ERROR(">>> CSV SYNC: ERROR - Invalid line number from unsent list");
                        setSyncState(SYNC_STATE_ERROR);
                        break;
                    }
                   
                    testData completeData = readCompleteTestDataFromCSV(lineNumber);
                    if (completeData.lineNumber > 0) {  // Check if data is valid
                        jsonData = "{";
                        jsonData += "\"sent\":" + String(completeData.sent) + ",";
                        jsonData += "\"lineNumber\":" + String(completeData.lineNumber) + ",";
                        jsonData += "\"test_id\":\"" + String(completeData.msgid) + "\",";
                        jsonData += "\"device_id\":\"" + String(completeData.testarid) + "\",";
                        jsonData += "\"prodname\":\"" + String(completeData.prodname) + "\",";
                        jsonData += "\"serialnr\":\"" + String(completeData.serialnr) + "\",";
                        jsonData += "\"teststatus\":\"" + String(completeData.teststatus) + "\",";
                        jsonData += "\"data\":\"" + String(completeData.data) + "\",";
                        jsonData += "\"freeText\":\"" + String(completeData.freeText) + "\"";
                        jsonData += "}";
                        
                        if (jsonData.length() > 4096) {
                            LOG_ERROR(">>> CSV SYNC: ERROR - JSON data too large for server");
                            setSyncState(SYNC_STATE_ERROR);
                            break;
                        }
                    } else {
                        LOG_ERROR(">>> CSV SYNC: ERROR - Could not read complete test data from CSV");
                        setSyncState(SYNC_STATE_ERROR);
                        break;
                    }
                } else {
                    // Use pending test queue data
                    if (getPendingTest(getCurrentSyncTest(), test_id, &productA_pressed_single, &productB_pressed_single, &timestamp_single)) {
                        jsonData = "{\"test_id\":\"" + String(test_id) + "\",";
                        jsonData += "\"data\":{\"productA\":" + String(productA_pressed_single ? "true" : "false") + ",";
                        jsonData += "\"productB\":" + String(productB_pressed_single ? "true" : "false") + "},";
                        jsonData += "\"device_id\":\"" + String(device_id) + "\"}";
                    } else {
                        LOG_ERROR(">>> CSV SYNC: ERROR - Could not read pending test data");
                        setSyncState(SYNC_STATE_ERROR);
                        break;
                    }
                }

                // Build HTTP request
                request = "POST /manageTests/addTestResult HTTP/1.1\r\n";
                request += "Host: " + String(SECRET_IP) + ":3000\r\n";
                request += "Content-Type: application/json\r\n";
                request += "Content-Length: " + String(jsonData.length()) + "\r\n";
                request += "Connection: close\r\n\r\n";
                request += jsonData;

                syncClient.print(request);
                setSyncState(SYNC_STATE_WAITING_RESPONSE);
                setSyncStateStartTime(currentTime);
                LOG_INFO(">>> CSV SYNC: Request sent, waiting for response...");
            } else {
                LOG_ERROR(">>> CSV SYNC: Client not connected, moving to error state");
                setSyncState(SYNC_STATE_ERROR);
            }
            break;
           
        case SYNC_STATE_WAITING_RESPONSE:
            // Response timeout handling
            if (currentTime - getSyncStateStartTime() > 5000) {
                LOG_INFO(">>> CSV SYNC: Response timeout");
                setSyncState(SYNC_STATE_ERROR);
                break;
            }
           
            if (syncClient.available()) {
                String response = syncClient.readStringUntil('\n');
               
                if (response.indexOf("HTTP/1.1 200") != -1 || response.indexOf("HTTP/1.1 201") != -1) {
                    LOG_INFO(">>> CSV SYNC: Test synced successfully");
                   
                    if (getCsvSyncEnabled()) {
                        // Mark test as sent in CSV
                        lineNumber = getUnsentLineNumber(getCurrentSyncTest());
                       
                        // FIXED: Eliminate duplicate marking - only call updateSentStatus
                        updateSentStatus(lineNumber);
                       
                        // FIXED: Check if marking was successful
                        if (getUnsentLineCount() == 0) {
                            LOG_ERROR(">>> CSV SYNC: Failed to mark as sent in CSV");
                        }
                    }
                   
                    setSyncState(SYNC_STATE_COMPLETE);
                } else {
                    LOG_ERROR(">>> CSV SYNC: Server error response: " + response);
                    setSyncState(SYNC_STATE_ERROR);
                }
            }
            break;
           
        case SYNC_STATE_COMPLETE:
            // Clean up and move to next test
            syncClient.stop();
            setCurrentSyncTest(getCurrentSyncTest() + 1);
            setSyncState(SYNC_STATE_IDLE);
            setSyncStateStartTime(0);
           
            // Check if all tests are complete
            if ((getCsvSyncEnabled() && getCurrentSyncTest() >= getUnsentLineCount()) ||
                (!getCsvSyncEnabled() && getCurrentSyncTest() >= getPendingTestCount())) {
                LOG_INFO(">>> CSV SYNC: All tests synced successfully!");
               
                // Visual feedback for successful sync
                startBlinkLEDNonBlocking(2, 200);
               
                // Clear sync data
                if (getCsvSyncEnabled()) {
                    setUnsentLineCount(0);
                } else {
                    clearPendingTests();
                }
               
                setHasPendingData(false);
                setIsSyncing(false);
            }
            break;
           
        case SYNC_STATE_ERROR:
            // Handle sync error and move to next test
            LOG_ERROR(">>> CSV SYNC: Sync failed, moving to next test");
            syncClient.stop();
            setCurrentSyncTest(getCurrentSyncTest() + 1);
            setSyncState(SYNC_STATE_IDLE);
            setSyncStateStartTime(0);
           
            // Check if all tests have been attempted
            if ((getCsvSyncEnabled() && getCurrentSyncTest() >= getUnsentLineCount()) ||
                (!getCsvSyncEnabled() && getCurrentSyncTest() >= getPendingTestCount())) {
                if (getCsvSyncEnabled()) {
                    setUnsentLineCount(0);
                } else {
                    clearPendingTests();
                }
                setHasPendingData(false);
                setIsSyncing(false);
                LOG_WARN(">>> CSV SYNC: All tests processed (some with errors)");
            }
            break;
    }
   
    // Early return if processing took too long
    if (millis() - workStartTime > MAX_WORK_TIME) {
        return;
    }
}