#include "test_data_processor.h"

// Read test data from CSV line for synchronization 
bool readTestDataFromCSV(int lineNumber, char test_id[], bool productA_pressed[], bool productB_pressed[], unsigned long timestamp[]) {
    // Declare all variables at the start to avoid goto issues
    char msgid[MAX_FIELD_SIZE];
    char prodname[MAX_FIELD_SIZE];
    char data[MAX_FIELD_SIZE];
    char aValue[2] = "0";
    char bValue[2] = "0";
    int aIndex = -1;
    int bIndex = -1;
    bool operationSuccess = false;

    // Acquire file lock before reading
    if (!acquireFileLock()) {
        LOG_ERROR(">>> CSV SYNC: File locked, cannot read test data");
        return false;
    }

    // Read required fields from CSV using new parser
    if (!readFlashField(lineNumber, 2, msgid)) {
        LOG_ERROR(">>> CSV SYNC: Failed to read msgid field (index 2)");
        releaseFileLock();
        return false;
    }
    if (!readFlashField(lineNumber, 4, prodname)) {
        LOG_ERROR(">>> CSV SYNC: Failed to read prodname field (index 4)");
        releaseFileLock();
        return false;
    }
    if (!readFlashField(lineNumber, 7, data)) {
        LOG_ERROR(">>> CSV SYNC: Failed to read data field (index 7)");
        releaseFileLock();
        return false;
    }

    // Copy test identifier
    strncpy(test_id, msgid, 64);
    test_id[63] = '\0';

    // Parse product presses from data field - look for A1_B0 format
    for (int i = 0; data[i] != '\0'; i++) {
        if (data[i] == 'A' && aIndex == -1) aIndex = i;
        if (data[i] == 'B' && bIndex == -1) bIndex = i;
    }
   
    if (aIndex != -1 && bIndex != -1) {
        // Extract values from A1_B0 format (new format)
        aValue[0] = data[aIndex + 1];
        bValue[0] = data[bIndex + 1];
       
        productA_pressed[0] = (aValue[0] == '1');
        productB_pressed[0] = (bValue[0] == '1');

        
       
        LOG_INFO(">>> CSV SYNC: Parsed data field '" + String(data) + "' -> A:" + (productA_pressed[0] ? "1" : "0") + " B:" + (productB_pressed[0] ? "1" : "0"));

    } else {
        // Fallback to old parsing method if new format not found
        LOG_INFO(">>> CSV SYNC: Using fallback parsing for data: " + String(data));
       
        if (strcmp(prodname, "PRODUCT_A") == 0) {
            productA_pressed[0] = true;
            productB_pressed[0] = false;
        } else if (strcmp(prodname, "PRODUCT_B") == 0) {
            productA_pressed[0] = false;
            productB_pressed[0] = true;
        } else if (strcmp(prodname, "BOTH") == 0) {
            productA_pressed[0] = true;
            productB_pressed[0] = true;
        } else {
            LOG_ERROR(">>> CSV SYNC: Invalid product name: " + String(prodname));
            releaseFileLock();
            return false;
        }
    }

    timestamp[0] = 0; // Timestamp not currently stored in CSV format
    operationSuccess = true;

    // Always release the file lock before returning
    releaseFileLock();
    return operationSuccess;
}

// Save test to CSV - UPDATED FOR 9 FIELDS (NO RESERVED FIELD) with mutex - PREVENT EMPTY LINES
void saveTestToCSV(const char test_id[], bool productA_pressed, bool productB_pressed, unsigned long timestamp) {
    // Acquire file lock before writing
    if (!acquireFileLock()) {
        LOG_ERROR(">>> SAVE: File locked, skipping");
        return;
    }

    LOG_INFO(">>> SAVING TEST DATA TO SD CARD...");
    testData data;
 
    // Initialize all fields to avoid garbage data
    memset(&data, 0, sizeof(testData));
 
    data.sent = 0; // FIELD 0 - marks as unsent (0 = not sent to server)
    data.lineNumber = getNextLineNumber(); // FIELD 1 - unique line identifier

    // FIELD 2: msgid (24 chars) - unique test identifier with device ID and timestamp
    strncpy(data.msgid, test_id, 24 - 1);
    data.msgid[24 - 1] = '\0';
 
    // FIELD 3: testarid (16 chars) - device identifier from secrets
    strncpy(data.testarid, DEVICE_ID, 16 - 1);
    data.testarid[16 - 1] = '\0';

    // FIELD 4: prodname (16 chars) - which product was tested
    if (productA_pressed && productB_pressed) {
        strncpy(data.prodname, "BOTH", 16 - 1);
    } else if (productA_pressed) {
        strncpy(data.prodname, "PRODUCT_A", 16 - 1);
    } else {
        strncpy(data.prodname, "PRODUCT_B", 16 - 1);
    }
    data.prodname[16 - 1] = '\0';

    // FIELD 5: serialnr (16 chars) - test sequence number within session
    char serialnr[16];
    snprintf(serialnr, sizeof(serialnr), "TEST_%03d", getTestsCompleted());
    strncpy(data.serialnr, serialnr, 16 - 1);
    data.serialnr[16 - 1] = '\0';

    // FIELD 6: teststatus (8 chars) - test result (always PASS for now)
    strncpy(data.teststatus, "PASS", 8 - 1);
    data.teststatus[8 - 1] = '\0';

    // FIELD 7: data - product press states in A1_B0 format (no commas for CSV safety)
    char extraData[32];
    snprintf(extraData, sizeof(extraData), "A%d_B%d", productA_pressed ? 1 : 0, productB_pressed ? 1 : 0);
    strncpy(data.data, extraData, 32 - 1);
    data.data[32 - 1] = '\0';

    // FIELD 8: freeText (64 chars) - user notes from serial commands
    char currentFreeTextBuffer[64];
    getFreeText(currentFreeTextBuffer);
    strncpy(data.freeText, currentFreeTextBuffer, 64 - 1);
    data.freeText[64 - 1] = '\0';

    // FIXED: Use the corrected writeFlash function that prevents empty lines
    if (writeFlash(data)) {
        LOG_INFO(">>> TEST SAVED TO SD CARD SUCCESSFULLY");
        LOG_DEBUG(">>> Saved Test Data:");
        LOG_DEBUG(">>> Test ID (2): " + String(data.msgid));
        LOG_DEBUG(">>> Product Name (4): " + String(data.prodname));
        LOG_DEBUG(">>> Data field (7): " + String(data.data));
        LOG_DEBUG(">>> Free text field (8): '" + String(data.freeText) + "'");
        // Also add to pending sync queue for immediate WiFi sync
        if (addPendingTest(test_id, productA_pressed, productB_pressed, timestamp)) {
            LOG_INFO(">>> Added to pending sync queue. Total pending: " + String(getPendingTestCount()));
        }
   
        // Clear free text for next test
        clearFreeText();
    } else {
        LOG_ERROR(">>> ERROR: Failed to save test to SD card");
        blinkErrorCode(1); // Visual error indication
    }
    LOG_DEBUG(">>> Releasing file lock");
    
    // Always release the file lock before returning
    releaseFileLock();
}

// Save test for offline synchronization when WiFi is unavailable
void saveOfflineTest(bool productA_pressed, bool productB_pressed, unsigned long currentTime) {
    char test_id[64];
    snprintf(test_id, sizeof(test_id), "%s_%lu", DEVICE_ID, currentTime);
 
    if (addPendingTest(test_id, productA_pressed, productB_pressed, currentTime)) {
        LOG_INFO(">>> Test saved for offline sync");
    } else {
        LOG_WARN(">>> WARNING: Could not save test to pending queue - queue full");
    }
}