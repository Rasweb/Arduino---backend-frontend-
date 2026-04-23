#ifndef STATE_TYPES_H
#define STATE_TYPES_H

#define MAX_FIELD_SIZE 64
#define MAX_FIELDS 10
#define MAX_ROW_SIZE 256  
#define FREE_TEXT_SIZE 64  

// Test states
enum TestState {
  STATE_IDLE,
  STATE_WAITING_FOR_WIFI,
  STATE_TEST_RUNNING,
  STATE_TEST_COMPLETED
};

// WiFi states - ADD ENUM FOR TYPE SAFETY
enum WiFiState {
  WIFI_STATE_DISCONNECTED = 0,
  WIFI_STATE_CONNECTING = 1,
  WIFI_STATE_CONNECTED = 2,
  WIFI_STATE_ERROR = 3
};

// State for simultaneous button handling
typedef enum {
  SIMULTANEOUS_IDLE,
  SIMULTANEOUS_DETECTED,
  SIMULTANEOUS_COOLDOWN
} SimultaneousState;

// SyncState enum
typedef enum {
  SYNC_STATE_IDLE,
  SYNC_STATE_CONNECTING,
  SYNC_STATE_SENDING,
  SYNC_STATE_WAITING_RESPONSE,
  SYNC_STATE_COMPLETE,
  SYNC_STATE_ERROR
} SyncState;

struct PendingTest {
  char test_id[64];
  bool productA_pressed;
  bool productB_pressed;
  unsigned long timestamp;
};

// CSV parser result structure
typedef struct {
    char fields[MAX_FIELDS][MAX_FIELD_SIZE]; // 2D array for fields
    int fieldCount;                          // Number of fields found
} CSVParserResult;

typedef struct {
    uint8_t sent;           // Field 0: 1 byte
    int lineNumber;         // Field 1: ~4 bytes  
    char msgid[24];         // Field 2
    char testarid[16];      // Field 3
    char prodname[16];      // Field 4
    char serialnr[16];      // Field 5
    char teststatus[8];     // Field 6
    char data[32];          // Field 7: A1_B0 format
    char freeText[64];      // Field 8: Free text (64 chars)
} testData;

typedef struct {
  int pin;
  bool pressed;
  bool waitingForRelease;  // Prevents repeated button-press
  unsigned long lastPressTime;
  unsigned long debounceDelay;
  unsigned long pressStartTime;  // Time when the pressure started
} btn_comp;

#endif