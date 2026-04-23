#include "test_system_helpers.h"

bool shouldAttemptSync(unsigned long currentTime) {
    // Don't sync during active testing - NEVER block tests!
   
    if (getSyncState() != SYNC_STATE_IDLE){
        return false;
    }

    if (getCurrentState() == STATE_TEST_RUNNING) {
        return false;
    }
   
    // Only sync if we have WiFi connection
    if (getCurrentWiFiState() != WIFI_STATE_CONNECTED) {
        return false;
    }
   
    // Check if we have unsent data based on sync mode
    bool hasUnsentData = false;
    if (getCsvSyncEnabled()) {
        hasUnsentData = (getUnsentLineCount() > 0);
    } else {
        hasUnsentData = (getPendingTestCount() > 0);
    }
   
    if (!hasUnsentData) {
        return false;
    }
   
    // Don't sync too often - wait at least 10 seconds between attempts
    if (currentTime - getLastSyncAttempt() < 10000) {
        return false;
    }
   
    LOG_INFO(">>> SYNC: Conditions met - ");
    if (getCsvSyncEnabled()) {
        LOG_INFO(String(getUnsentLineCount()) + " unsent tests in CSV");
    } else {
        LOG_INFO(String(getPendingTestCount()) + " tests in queue");
    }
    return true;
}

void manageWiFiConnection(unsigned long currentTime) {
    static unsigned long lastLog = 0;
    if (currentTime - lastLog > 30000) { // Log every 30 seconds
        logWiFiStatus();
        lastLog = currentTime;
    }
}

void printDebugInfo(unsigned long currentTime) {
    RETURN_IF_NOT_DEBUG;

    static unsigned long lastDebugPrint = 0;
   
    if (currentTime - lastDebugPrint >= 5000) {
        Serial.println("=== DEBUG INFO ===");
        Serial.print("test State: ");
        switch(getCurrentState()) {
            case STATE_IDLE: Serial.println("IDLE"); break;
            case STATE_WAITING_FOR_WIFI: Serial.println("WAITING_FOR_WIFI"); break;
            case STATE_TEST_RUNNING: Serial.println("TEST_RUNNING"); break;
            case STATE_TEST_COMPLETED: Serial.println("TEST_COMPLETED"); break;
        }
       
        Serial.print("Tests completed: "); Serial.print(getTestsCompleted()); Serial.println("/10");
        Serial.print("Session counter: "); Serial.println(getTestSessionCounter());
        Serial.print("Pending tests (queue): "); Serial.println(getPendingTestCount());
        Serial.print("Unsent tests (CSV): "); Serial.println(getUnsentLineCount());
        Serial.print("CSV sync enabled: "); Serial.println(getCsvSyncEnabled() ? "YES" : "NO");
        Serial.print("WiFi enabled: "); Serial.println(getWifiEnabled() ? "YES" : "NO");
        
        // NEW: WiFi state information
        Serial.print("WiFi state: ");
        switch(getCurrentWiFiState()) {
            case WIFI_STATE_DISCONNECTED: Serial.println("DISCONNECTED"); break;
            case WIFI_STATE_CONNECTING: Serial.println("CONNECTING"); break;
            case WIFI_STATE_CONNECTED: Serial.println("CONNECTED"); break;
            case WIFI_STATE_ERROR: Serial.println("ERROR"); break;
        }
        Serial.print("WiFi connected: "); Serial.println(WiFi.status() == WL_CONNECTED ? "YES" : "NO");
        
        Serial.print("Sync active: "); Serial.println(getIsSyncing() ? "YES" : "NO");
       
        // Add sync status info
        if (getIsSyncing()) {
            Serial.print("Sync progress: ");
            Serial.print(getCurrentSyncTest());
            Serial.print("/");
            if (getCsvSyncEnabled()) {
                Serial.println(getUnsentLineCount());
            } else {
                Serial.println(getPendingTestCount());
            }
        }
        Serial.print("Sync state: ");
        switch (getSyncState()) {
        case SYNC_STATE_IDLE:
            Serial.println("IDLE");
            break;
        case SYNC_STATE_CONNECTING:
            Serial.println("CONNECTING");
            break;
        case SYNC_STATE_SENDING:
            Serial.println("SENDING");
            break;
        case SYNC_STATE_WAITING_RESPONSE:
            Serial.println("WAITING_RESPONSE");
            break;
        case SYNC_STATE_COMPLETE:
            Serial.println("COMPLETE");
            break;
        case SYNC_STATE_ERROR:
            Serial.println("ERROR");
            break;
        }
       
        Serial.println("=================");
       
        lastDebugPrint = currentTime;
    }
}