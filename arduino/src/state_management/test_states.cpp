#include "test_states.h"

// FIXED: Initialize previousState to sentinel value to trigger initial entry actions
static TestState previousState = (TestState)-1;
static unsigned long waitingForWifiStartTime = 0;

// WiFi state machine variables - CHANGED TO WiFiState ENUM
static WiFiState currentWiFiState = WIFI_STATE_DISCONNECTED;
static unsigned long lastWiFiAttempt = 0;
static unsigned long wifiConnectStartTime = 0;
static int wifiRetryCount = 0;
static const unsigned long WIFI_CONNECT_TIMEOUT = 10000; // 10 seconds
static const unsigned long WIFI_RETRY_DELAY = 30000;     // 30 seconds between retries
static const unsigned long WIFI_IDLE_DISCONNECT_TIME = 60000; // 1 minute idle disconnect

// FIXED: Share one WiFi inactivity timer between both functions
static unsigned long lastWiFiActiveTime = 0;

// Function to check if WiFi should disconnect due to inactivity - FIXED LOGIC
bool shouldDisconnectWiFi(unsigned long currentTime) {
    // Don't disconnect during active testing or syncing
    if (getCurrentState() == STATE_TEST_RUNNING || getIsSyncing()) {
        lastWiFiActiveTime = currentTime; // Reset timer when active
        return false;
    }
   
    // Don't disconnect if we're waiting for WiFi or in test completed state
    if (getCurrentState() == STATE_WAITING_FOR_WIFI || getCurrentState() == STATE_TEST_COMPLETED) {
        lastWiFiActiveTime = currentTime; // Reset timer
        return false;
    }
   
    // Don't disconnect if we have unsent data that needs syncing
    bool hasUnsentData = getCsvSyncEnabled() ? (getUnsentLineCount() > 0) : (getPendingTestCount() > 0);
    if (hasUnsentData) {
        // We have data to sync, don't disconnect but update activity time
        if (lastWiFiActiveTime == 0) {
            lastWiFiActiveTime = currentTime;
        }
        return false;
    }
   
    // If we have no last active time, set it now
    if (lastWiFiActiveTime == 0) {
        lastWiFiActiveTime = currentTime;
        return false;
    }
   
    // Check if idle time exceeded
    unsigned long idleTime = currentTime - lastWiFiActiveTime;
    bool shouldDisconnect = (idleTime > WIFI_IDLE_DISCONNECT_TIME);
   
    if (shouldDisconnect) {
      LOG_INFO(">>> WiFi: Auto-disconnecting due to " + String(idleTime / 1000) + " seconds of inactivity");
        // FIX: Reset the timer when we decide to disconnect to prevent immediate re-disconnect
      lastWiFiActiveTime = 0; // Reset to prevent immediate disconnect on next connection
  }
   
    return shouldDisconnect;
}

// FIXED: Add function to reset WiFi activity timer when connection is established
void resetWiFiActivityTimer(unsigned long currentTime) {
    lastWiFiActiveTime = currentTime;
    LOG_DEBUG(">>> WiFi: Activity timer reset");
}

// State enter/exit functions
void onEnterIdle() {
  LOG_DEBUG(">>> STATE: Entering IDLE");
  setLED(false);
}

void onEnterWaitingForWifi() {
  LOG_DEBUG(">>> STATE: Entering WAITING_FOR_WIFI");
  waitingForWifiStartTime = millis();
}

void onEnterTestRunning() {
  LOG_DEBUG(">>> STATE: Entering TEST_RUNNING");
  setLED(true);
  setTestStartTime(millis());
  setLastButtonPressTime(getTestStartTime());
  LOG_DEBUG(">>> TEST SESSION STARTED - Press RED/BLUE buttons!");
}

void onEnterTestCompleted() {
  LOG_DEBUG(">>> STATE: Entering TEST_COMPLETED");
  setLED(false);
  LOG_DEBUG(">>> SESSION COMPLETE: " + String(getTestsCompleted()) + " tests completed");
}

// NEW: WiFi State Machine Functions - FIXED: Reset lastActiveTime when connecting
void updateWiFiStateMachine(unsigned long currentTime) {
  if (!getWifiEnabled()) {
    currentWiFiState = WIFI_STATE_DISCONNECTED;
    return;
  }

  // Declare status variable outside switch to avoid compilation errors
  uint8_t wifiStatus = WiFi.status();
 
  switch (currentWiFiState) {
    case WIFI_STATE_DISCONNECTED:
      // Check if we should attempt connection - ONLY when needed
      if (shouldAttemptWiFiConnection(currentTime)) {
        startWiFiConnection();
        currentWiFiState = WIFI_STATE_CONNECTING;
        wifiConnectStartTime = currentTime;
        wifiRetryCount++;
        LOG_INFO(">>> WiFi: Starting connection attempt");
      }
      break;

    case WIFI_STATE_CONNECTING:
      // Check for timeout
      if (currentTime - wifiConnectStartTime > WIFI_CONNECT_TIMEOUT) {
        LOG_INFO(">>> WiFi: Connection timeout");
        currentWiFiState = WIFI_STATE_ERROR;
        lastWiFiAttempt = currentTime;
        break;
      }

      // Check connection status non-blocking
      if (wifiStatus == WL_CONNECTED) {
        LOG_INFO(">>> WiFi: Connected successfully");
        currentWiFiState = WIFI_STATE_CONNECTED;
        wifiRetryCount = 0;
        LOG_INFO(">>> WiFi IP: " + WiFi.localIP().toString());
       
        // FIX: Reset activity timer when WiFi connects to prevent immediate disconnect
        resetWiFiActivityTimer(currentTime);
       
      } else if (wifiStatus == WL_CONNECT_FAILED || wifiStatus == WL_NO_SSID_AVAIL) {
        LOG_INFO(">>> WiFi: Connection failed");
        currentWiFiState = WIFI_STATE_ERROR;
        lastWiFiAttempt = currentTime;
      }
      break;

    case WIFI_STATE_CONNECTED:
      // FIX: Auto-disconnect WiFi when idle for too long
      if (wifiStatus != WL_CONNECTED) {
        LOG_INFO(">>> WiFi: Connection lost");
        currentWiFiState = WIFI_STATE_DISCONNECTED;
        // FIX: Reset activity timer when connection is lost
        resetWiFiActivityTimer(0);
      } else {
        // Check if we should disconnect due to inactivity
        bool shouldDisconnect = shouldDisconnectWiFi(currentTime);
        if (shouldDisconnect) {
          LOG_INFO(">>> WiFi: Auto-disconnecting due to inactivity");
          stopWiFiConnection();
          // FIX: Reset activity timer after disconnect
          resetWiFiActivityTimer(0);
        }
      }
      break;

    case WIFI_STATE_ERROR:
      // Wait before retrying
      if (currentTime - lastWiFiAttempt > WIFI_RETRY_DELAY) {
        currentWiFiState = WIFI_STATE_DISCONNECTED;
        LOG_DEBUG(">>> WiFi: Retry delay passed, ready for new attempt");
      }
      break;
  }
}

void startWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  LOG_INFO(">>> WiFi: Attempting to connect...");
  WiFi.disconnect();
  delay(100); // Short delay
 
  WiFi.begin(SECRET_SSID, SECRET_PASS);
 
  LOG_INFO(">>> WiFi: Connecting to " + String(SECRET_SSID) + "...");
}

void stopWiFiConnection() {
  WiFi.disconnect();
  currentWiFiState = WIFI_STATE_DISCONNECTED;
  wifiRetryCount = 0;
  // FIX: Reset activity timer when manually disconnecting
  resetWiFiActivityTimer(0);
  LOG_INFO(">>> WiFi: Disconnected");
}

bool shouldAttemptWiFiConnection(unsigned long currentTime) {
  // Don't attempt if WiFi is disabled
  if (!getWifiEnabled()) {
    return false;
  }

  // Don't attempt if already connected or connecting
  if (currentWiFiState == WIFI_STATE_CONNECTED ||
      currentWiFiState == WIFI_STATE_CONNECTING) {
    return false;
  }

  // Apply retry delay for error state
  if (currentWiFiState == WIFI_STATE_ERROR) {
    if (currentTime - lastWiFiAttempt < WIFI_RETRY_DELAY) {
      return false;
    }
  }

  // FIX: Allow tests to run even without WiFi - only require WiFi for syncing
  bool hasUnsentData = getCsvSyncEnabled() ? (getUnsentLineCount() > 0) : (getPendingTestCount() > 0);
  bool shouldSyncState = (getCurrentState() == STATE_IDLE ||
                         getCurrentState() == STATE_TEST_COMPLETED);

  return hasUnsentData && shouldSyncState;
}

// CHANGED: Update function to return WiFiState instead of int
WiFiState getCurrentWiFiState() {
  return currentWiFiState;
}

// UPDATED: State handler functions with non-blocking WiFi
void handleIdleState() {
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 10000) {
    LOG_INFO(">>> IDLE: Press GREEN button to start test session");
    LOG_INFO(">>> Unsent tests in CSV: " + String(getUnsentLineCount()));
    Serial.print(">>> WiFi: ");
    switch (getCurrentWiFiState()) {
      case WIFI_STATE_DISCONNECTED: Serial.println("DISCONNECTED"); break;
      case WIFI_STATE_CONNECTING: Serial.println("CONNECTING"); break;
      case WIFI_STATE_CONNECTED: Serial.println("CONNECTED"); break;
      case WIFI_STATE_ERROR: Serial.println("ERROR"); break;
    }
    lastLog = millis();
  }
}

void handleWaitingForWifiState() {
  // Show status every second
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 1000) {
    LOG_INFO(">>> Waiting for WiFi (" + String(3000 - (millis() - waitingForWifiStartTime)) + "ms remaining)");
    lastStatus = millis();
  }
}

void handleTestRunningState() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    Serial.print(">>> TEST_RUNNING: ");
    Serial.print(getTestsCompleted());
    Serial.print("/10 tests completed");
    Serial.print(" | WiFi: ");
    switch (getCurrentWiFiState()) {
      case WIFI_STATE_DISCONNECTED: Serial.println("DISCONNECTED"); break;
      case WIFI_STATE_CONNECTING: Serial.println("CONNECTING"); break;
      case WIFI_STATE_CONNECTED: Serial.println("CONNECTED"); break;
      case WIFI_STATE_ERROR: Serial.println("ERROR"); break;
    }
    lastUpdate = millis();
  }

  // FIX: Don't process simultaneous presses when test session is completed
  if (getTestsCompleted() >= 10 || (millis() - getLastButtonPressTime() > 60000)) {
    setCurrentState(STATE_TEST_COMPLETED);
  }
}

void handleTestCompletedState() {
  static unsigned long completedTime = 0;
  if (completedTime == 0) completedTime = millis();

  // Stay in completed state for 5 seconds then return to idle
  if (millis() - completedTime > 5000) {
    setCurrentState(STATE_IDLE);
    setTestsCompleted(0);
    completedTime = 0;
    setIsSyncing(false);
   
    LOG_INFO(">>> Ready for new test session");
    LOG_INFO(">>> Press GREEN button to start new session");
  }
}

// UPDATED: Main state machine update function with WiFi state machine
void updateStateMachine(unsigned long currentTime) {
  // Update WiFi state machine first (non-blocking)
  updateWiFiStateMachine(currentTime);

  // FIXED: Check for initial state entry (when previousState is sentinel value)
  if (previousState == (TestState)-1) {
    // This is the first call - trigger initial state entry
    previousState = getCurrentState();
    switch(getCurrentState()) {
      case STATE_IDLE:
        onEnterIdle();
        break;
      case STATE_WAITING_FOR_WIFI:
        onEnterWaitingForWifi();
        break;
      case STATE_TEST_RUNNING:
        onEnterTestRunning();
        break;
      case STATE_TEST_COMPLETED:
        onEnterTestCompleted();
        break;
    }
  }

  // Check for state changes and call exit/enter functions
  if (getCurrentState() != previousState) {

    // Call exit function for previous state
    switch(previousState) {
      case STATE_IDLE:
        LOG_DEBUG(">>> STATE: Exiting IDLE");
        break;
      case STATE_WAITING_FOR_WIFI:
        waitingForWifiStartTime = 0;
        break;
      case STATE_TEST_RUNNING:
        setSimultaneousState(SIMULTANEOUS_IDLE);
        break;
      case STATE_TEST_COMPLETED:
        setIsSyncing(false);
        break;
    }
   
    // Call enter function for new state
    switch(getCurrentState()) {
      case STATE_IDLE:
        onEnterIdle();
        break;
      case STATE_WAITING_FOR_WIFI:
        onEnterWaitingForWifi();
        break;
      case STATE_TEST_RUNNING:
        onEnterTestRunning();
        break;
      case STATE_TEST_COMPLETED:
        onEnterTestCompleted();
        break;
    }
   
    previousState = getCurrentState();
  }
 
  // Handle the current state
  switch(getCurrentState()) {
    case STATE_IDLE:
      handleIdleState();
      break;
    case STATE_WAITING_FOR_WIFI:
      handleWaitingForWifiState();
      break;
    case STATE_TEST_RUNNING:
      handleTestRunningState();
      break;
    case STATE_TEST_COMPLETED:
      handleTestCompletedState();
      break;
  }
}