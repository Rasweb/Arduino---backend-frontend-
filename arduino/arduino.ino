#include "testSender.h" // includes any required functions 

// Pin definitions
unsigned long DEBOUNCE_TIME = 50; // milliseconds
const int BUTTON1_PIN = 2;
const int BUTTON2_PIN = 3;
const int BUTTON3_PIN = 4;
const int LED_PIN = 5;
#define LOOP_INDICATOR_LED 6

// Test control
unsigned long TEST_RUN_TIME = 30000;

// External variables
unsigned long SYNC_RETRY_DELAY = 30000;
unsigned long WIFI_CHECK_INTERVAL = 60000;
unsigned long WIFI_CHECK_INTERVAL_TESTING = 30000;
unsigned long DEBUG_INTERVAL = 15000;
unsigned long AUTO_SYNC_CHECK_INTERVAL = 10000;
unsigned long lastButtonCheck = 0;
unsigned long testTime = 0;

static SyncState lastSyncState = SYNC_STATE_IDLE;
void setup() {
  pinMode(LOOP_INDICATOR_LED, OUTPUT);
  // Serial.begin(9600);
  Serial.begin(115200);
  // Give serial a moment (testSenderInit() also waits)
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime) < 2000) { delay(10); }

  // Initialize test system (pins, led, button wiring etc.)
  testSenderInit(BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN, LED_PIN);

  LOG_INFO("Setup complete.");
  LOG_INFO("Arduino IP: " + WiFi.localIP().toString());
}

void loop() {
  unsigned long currentTime = millis();

  // test will stop after 30 secs
  if(getCurrentState() == STATE_TEST_RUNNING  && millis() - testTime > TEST_RUN_TIME){
    LOG_INFO("Has ran 30 sec");
    LOG_INFO("ending test");
    setCurrentState(STATE_TEST_COMPLETED);
    // Trigger sync
    if (shouldAttemptSync(currentTime)) {
      LOG_INFO(">>> Triggering sync after test completion...");
      startNonBlockingSync();
    }
  }


  // handle test logic and other btns
if(getSyncState() != lastSyncState) {
    Serial.print("Sync state changed to: ");
    switch(getSyncState()) {
        case SYNC_STATE_IDLE: Serial.println("IDLE"); break;
        case SYNC_STATE_CONNECTING: Serial.println("CONNECTING"); break;
        case SYNC_STATE_SENDING: Serial.println("SENDING"); break;
        case SYNC_STATE_WAITING_RESPONSE: Serial.println("WAITING_RESPONSE"); break;
        case SYNC_STATE_COMPLETE: Serial.println("COMPLETE"); break;
        case SYNC_STATE_ERROR: Serial.println("ERROR"); break;
    }
    lastSyncState = getSyncState();
}

  testSystem_update();

  // Blink loop led
  digitalWrite(LOOP_INDICATOR_LED, !digitalRead(LOOP_INDICATOR_LED));
  delay(30); 
}
