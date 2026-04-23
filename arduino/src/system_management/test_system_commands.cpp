#include "test_system_commands.h"

// Add extern declaration for mutex functions
extern bool acquireFileLock();
extern void releaseFileLock();
extern void forceReleaseFileLock();

// NEW: Function to find logical line number from physical line number - UPDATED: Use new CSV parser
int findLogicalLineNumber(int physicalLineNumber) {
    if (physicalLineNumber <= 0) {
        return -1;
    }
    
    // Acquire file lock before reading
    if (!acquireFileLock()) {
        LOG_WARN(">>> COMMAND: File locked, cannot find logical line number");
        return -1;
    }
   
    FILE *file = fopen("/fs/test.csv", "r");
    if (!file) {
        releaseFileLock();
        return -1;
    }
   
    char buf[MAX_ROW_SIZE];
    int currentPhysicalLine = 0;
    int logicalLineNumber = -1;
   
    while (fgets(buf, sizeof(buf), file)) {
        currentPhysicalLine++;
        buf[strcspn(buf, "\n")] = '\0';
       
        if (strlen(buf) == 0) {
            continue;
        }
       
        if (currentPhysicalLine == physicalLineNumber) {
            // UPDATED: Use new CSV parser instead of global arrays
            CSVParserResult result = parseCSVLine(buf);
            if (getCSVFieldCount(&result) >= 2) {
                logicalLineNumber = atoi(getCSVField(&result, 1));
            }
            break;
        }
    }
   
    fclose(file);
    releaseFileLock();
    return logicalLineNumber;
}

void handleIncomingData() {
    if (!Serial.available()) return;
   
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0) return;
   
    Serial.print(">>> COMMAND: ");
    Serial.println(input);
   
    if (input == "read") {
        Serial.println(">>> Reading all test data from flash...");
        readFlash(); // This function already has mutex internally
    }
    else if (input.startsWith("readfield")) {
        int space1 = input.indexOf(' ');
        int space2 = input.indexOf(' ', space1 + 1);
        if (space1 != -1 && space2 != -1) {
            int row = input.substring(space1 + 1, space2).toInt();
            int field = input.substring(space2 + 1).toInt();
            char result[MAX_FIELD_SIZE];
            if (readFlashField(row, field, result)) { // This function already has mutex internally
                Serial.print(">>> Field: ");
                Serial.println(result);
            } else {
                Serial.println(">>> Field not found");
            }
        }
    }
    else if (input == "json") {
        Serial.println(">>> Converting CSV to JSON...");
        
        // Acquire file lock for JSON conversion
        if (!acquireFileLock()) {
            Serial.println(">>> JSON: File locked, skipping conversion");
            return;
        }
        
        FILE *file = fopen("/fs/test.csv", "r");
        if (file) {
            Serial.println(">>> Parsing CSV file...");
            char csvData[4096] = "";
            char buf[256];
            int lineCount = 0;
           
            while (fgets(buf, sizeof(buf), file) && lineCount < 100) {
                lineCount++;
                buf[strcspn(buf, "\n")] = 0;
                if (strlen(csvData) + strlen(buf) < sizeof(csvData) - 10) {
                    strncat(csvData, buf, sizeof(csvData) - strlen(csvData) - 1);
                    strncat(csvData, "\n", 1);
                } else {
                    Serial.println(">>> WARNING: CSV data too large for JSON conversion, truncating");
                    break;
                }
            }
            fclose(file);
           
            String json = convertCsvBatchToJson(csvData);
            Serial.println(">>> JSON Output:");
            Serial.println(json);
            Serial.print(">>> JSON size: ");
            Serial.print(json.length());
            Serial.println(" characters");
            Serial.print(">>> Lines converted: ");
            Serial.println(lineCount);
        } else {
            Serial.println(">>> No CSV file found");
        }
        
        releaseFileLock();
    }
    else if (input == "upload") {
        Serial.println(">>> Manual upload disabled - use auto-sync");
    }
    else if (input == "sync") {
        setLastSyncAttempt(0);
        startNonBlockingSync();
        Serial.println(">>> NON-BLOCKING CSV-based sync triggered");
    }
    else if (input == "wifi") {
        logWiFiStatus();
        Serial.print(">>> WiFi enabled: ");
        Serial.println(getWifiEnabled() ? "YES" : "NO");
    }
    else if (input == "state") {
        Serial.print(">>> Current state: ");
        switch(getCurrentState()) {
            case STATE_IDLE: Serial.println("IDLE"); break;
            case STATE_WAITING_FOR_WIFI: Serial.println("WAITING_FOR_WIFI"); break;
            case STATE_TEST_RUNNING: Serial.println("TEST_RUNNING"); break;
            case STATE_TEST_COMPLETED: Serial.println("TEST_COMPLETED"); break;
        }
    }
    else if (input == "clean") {
        Serial.println(">>> CLEAN: Starting cleanup procedure...");
        
        // Force release any stuck file locks first
        forceReleaseFileLock();
        
        // Small delay to ensure lock is fully released
        delay(100);
        
        // Now clear the file using the normal function
        clearEntireFileFlash();
        Serial.println(">>> Cleanup completed successfully");
    }
    else if (input == "forcelock") {
        Serial.println(">>> FORCE RELEASING FILE LOCK...");
        forceReleaseFileLock();
        Serial.println(">>> File lock force-released");
    }
    else if (input == "clear") {
        clearEntireFileFlash(); // This function already has mutex internally
        Serial.println(">>> CSV file cleared and line counter reset");
    }
    else if (input == "nowifi") {
        setWifiEnabled(false);
        WiFi.disconnect();
        stopNonBlockingSync();
        Serial.println(">>> WiFi disabled - no more connection or sync attempts");
    }
    else if (input == "enablewifi") {
        setWifiEnabled(true);
        Serial.println(">>> WiFi enabled - will attempt connection");
    }
    else if (input == "reset") {
        clearPendingTests();
        setHasPendingData(false);
        stopNonBlockingSync();
        setTestsCompleted(0);
        setCurrentState(STATE_IDLE);
        setWifiEnabled(true);
        Serial.println(">>> SYSTEM RESET: Ready for new tests");
    }
    else if (input == "status") {
        Serial.println("=== SYSTEM STATUS ===");
        Serial.print("State: ");
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
        Serial.print("Sync active: "); Serial.println(getIsSyncing() ? "YES" : "NO");
        Serial.print("WiFi enabled: "); Serial.println(getWifiEnabled() ? "YES" : "NO");
        Serial.print("WiFi connected: "); Serial.println(WiFi.status() == WL_CONNECTED ? "YES" : "NO");
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("IP address: "); Serial.println(WiFi.localIP());
        }
        Serial.println("====================");
    }
    else if (input == "csvscan") {
        Serial.println(">>> Scanning CSV for unsent tests...");
        findUnsentTestsFromCSV(); // This function now has mutex internally
        Serial.print(">>> Found ");
        Serial.print(getUnsentLineCount());
        Serial.println(" unsent tests");
    }
    else if (input == "fastscan") {
        Serial.println(">>> FAST SCAN: Scanning for unsent tests with optimized method...");
        findUnsentTestsFast(); // This function now has mutex internally
        Serial.print(">>> Found ");
        Serial.print(getUnsentLineCount());
        Serial.println(" unsent tests with optimized scanning");
    }
    else if (input == "usecsvsync") {
        setCsvSyncEnabled(true);
        Serial.println(">>> CSV-based sync ENABLED - using optimized scanning");
    }
    else if (input == "usequeuesync") {
        setCsvSyncEnabled(false);
        Serial.println(">>> Queue-based sync ENABLED");
    }
    else if (input.startsWith("text ")) {
        String noteText = input.substring(5);
        if (noteText.length() > 0) {
            setFreeText(noteText.c_str());
        } else {
            Serial.println(">>> ERROR: Text cannot be empty");
        }
    }
    else if (input == "cleartext") {
        clearFreeText();
        Serial.println(">>> Free text cleared");
    }
    else if (input == "showtext") {
        char currentText[FREE_TEXT_SIZE];
        getFreeText(currentText);
        Serial.print(">>> Current free text: '");
        Serial.print(currentText);
        Serial.println("'");
    }
    else if (input.startsWith("editline ")) {
        int spacePos = input.indexOf(' ');
        int secondSpacePos = input.indexOf(' ', spacePos + 1);
       
        if (spacePos != -1 && secondSpacePos != -1) {
            int inputLineNumber = input.substring(spacePos + 1, secondSpacePos).toInt();
            String noteText = input.substring(secondSpacePos + 1);
           
            if (inputLineNumber > 0) {
                bool success = false;
               
                Serial.print(">>> Trying logical line number ");
                Serial.println(inputLineNumber);
                success = editLine(inputLineNumber, noteText.c_str()); // This function has mutex internally
               
                if (!success) {
                    Serial.print(">>> Trying physical line number ");
                    Serial.println(inputLineNumber);
                    int logicalLineNumber = findLogicalLineNumber(inputLineNumber);
                   
                    if (logicalLineNumber > 0) {
                        Serial.print(">>> Physical line ");
                        Serial.print(inputLineNumber);
                        Serial.print(" has logical line ");
                        Serial.println(logicalLineNumber);
                        success = editLine(logicalLineNumber, noteText.c_str()); // This function has mutex internally
                    } else {
                        Serial.println(">>> ERROR: Could not find logical line number for physical line");
                    }
                }
               
                if (success) {
                    Serial.println(">>> Line edited successfully");
                    Serial.print(">>> Free text saved as: '");
                    Serial.print(noteText);
                    Serial.println("'");
                } else {
                    Serial.println(">>> ERROR: Failed to edit line");
                }
            } else {
                Serial.println(">>> ERROR: Invalid line number");
            }
        } else {
            Serial.println(">>> ERROR: Usage: editline [line] [text]");
        }
    }
    else if (input.startsWith("addtext ")) {
        int spacePos = input.indexOf(' ');
        int secondSpacePos = input.indexOf(' ', spacePos + 1);
       
        if (spacePos != -1 && secondSpacePos != -1) {
            int inputLineNumber = input.substring(spacePos + 1, secondSpacePos).toInt();
            String noteText = input.substring(secondSpacePos + 1);
           
            if (inputLineNumber > 0) {
                bool success = false;
                int logicalLineNumber = findLogicalLineNumber(inputLineNumber);
               
                if (logicalLineNumber > 0) {
                    Serial.print(">>> Physical line ");
                    Serial.print(inputLineNumber);
                    Serial.print(" has logical line ");
                    Serial.println(logicalLineNumber);
                    success = addTextToLine(logicalLineNumber, noteText.c_str()); // This function has mutex internally
                } else {
                    success = addTextToLine(inputLineNumber, noteText.c_str()); // This function has mutex internally
                }
               
                if (success) {
                    Serial.println(">>> Text added to line successfully");
                } else {
                    Serial.println(">>> ERROR: Failed to add text to line");
                }
            } else {
                Serial.println(">>> ERROR: Invalid line number");
            }
        } else {
            Serial.println(">>> ERROR: Usage: addtext [line] [text]");
        }
    }
    else if (input.startsWith("updateline ")) {
        int spacePos = input.indexOf(' ');
        int secondSpacePos = input.indexOf(' ', spacePos + 1);
       
        if (spacePos != -1 && secondSpacePos != -1) {
            int inputLineNumber = input.substring(spacePos + 1, secondSpacePos).toInt();
            String noteText = input.substring(secondSpacePos + 1);
           
            if (inputLineNumber > 0) {
                bool success = false;
                int logicalLineNumber = findLogicalLineNumber(inputLineNumber);
               
                if (logicalLineNumber > 0) {
                    Serial.print(">>> Physical line ");
                    Serial.print(inputLineNumber);
                    Serial.print(" has logical line ");
                    Serial.println(logicalLineNumber);
                    success = updateLineWithText(logicalLineNumber, noteText.c_str()); // This function has mutex internally
                } else {
                    success = updateLineWithText(inputLineNumber, noteText.c_str()); // This function has mutex internally
                }
               
                if (success) {
                    Serial.println(">>> Line text updated successfully");
                } else {
                    Serial.println(">>> ERROR: Failed to update line text");
                }
            } else {
                Serial.println(">>> ERROR: Invalid line number");
            }
        } else {
            Serial.println(">>> ERROR: Usage: updateline [line] [text]");
        }
    }
    else if (input.startsWith("showline ")) {
        int spacePos = input.indexOf(' ');
        if (spacePos != -1) {
            int inputLineNumber = input.substring(spacePos + 1).toInt();
           
            if (inputLineNumber > 0) {
                bool success = false;
                int logicalLineNumber = findLogicalLineNumber(inputLineNumber);
               
                if (logicalLineNumber > 0) {
                    Serial.print(">>> Physical line ");
                    Serial.print(inputLineNumber);
                    Serial.print(" has logical line ");
                    Serial.println(logicalLineNumber);
                    success = showLine(logicalLineNumber); // This function has mutex internally
                } else {
                    success = showLine(inputLineNumber); // This function has mutex internally
                }
               
                if (!success) {
                    Serial.println(">>> ERROR: Line not found");
                }
            } else {
                Serial.println(">>> ERROR: Invalid line number");
            }
        } else {
            Serial.println(">>> ERROR: Usage: showline [line]");
        }
    }
    else if (input == "listlines") {
        Serial.println(">>> Physical to Logical Line Number Mapping:");
        
        // Acquire file lock for listing lines
        if (!acquireFileLock()) {
            Serial.println(">>> LISTLINES: File locked, skipping");
            return;
        }
        
        FILE *file = fopen("/fs/test.csv", "r");
        if (file) {
            char buf[MAX_ROW_SIZE];
            int physicalLine = 0;
           
            while (fgets(buf, sizeof(buf), file)) {
                physicalLine++;
                buf[strcspn(buf, "\n")] = '\0';
               
                if (strlen(buf) == 0) {
                    continue;
                }
               
                // UPDATED: Use new CSV parser instead of global arrays
                CSVParserResult result = parseCSVLine(buf);
                if (getCSVFieldCount(&result) >= 2) {
                    int logicalLine = atoi(getCSVField(&result, 1));
                    Serial.print(">>> Physical ");
                    Serial.print(physicalLine);
                    Serial.print(" -> Logical ");
                    Serial.println(logicalLine);
                }
            }
            fclose(file);
        } else {
            Serial.println(">>> ERROR: Cannot open CSV file");
        }
        
        releaseFileLock();
    }
    else if (input == "readRaw") {
        Serial.println(">>> Reading raw CSV data from flash...");
        readRaw(); // This function already has mutex internally
    }
    else if (input == "help") {
        Serial.println("=== TEST SYSTEM COMMANDS ===");
        Serial.println("read        - Read all CSV data (shows PHYSICAL line numbers)");
        Serial.println("readRaw     - Read raw CSV file content");
        Serial.println("listlines   - Show mapping between PHYSICAL and LOGICAL line numbers");
        Serial.println("readfield r f - Read specific field (row, field)");
        Serial.println("json        - Convert CSV to JSON (limited to 100 lines)");
        Serial.println("sync        - Trigger NON-BLOCKING CSV-based sync");
        Serial.println("wifi        - Show WiFi status");
        Serial.println("enablewifi  - Enable WiFi and connect");
        Serial.println("nowifi      - Disable WiFi completely");
        Serial.println("state       - Show current state");
        Serial.println("status      - Detailed system status");
        Serial.println("reset       - Reset system for new tests");
        Serial.println("clean       - Force cleanup and clear CSV file");
        Serial.println("forcelock   - Force release file lock (emergency)");
        Serial.println("clear       - Clear CSV file");
        Serial.println("csvscan     - Scan CSV for unsent tests (original method)");
        Serial.println("fastscan    - FAST optimized scanning for unsent tests");
        Serial.println("usecsvsync  - Enable CSV-based sync (DEFAULT)");
        Serial.println("usequeuesync - Enable old queue-based sync");
        Serial.println("text [note] - Set free text for next test");
        Serial.println("cleartext   - Clear free text");
        Serial.println("showtext    - Show current free text");
        Serial.println("editline [line] [text] - Edit line (accepts PHYSICAL or LOGICAL line numbers)");
        Serial.println("addtext [line] [text] - Add text to line (accepts PHYSICAL or LOGICAL)");
        Serial.println("updateline [line] [text] - Replace free text (same as editline)");
        Serial.println("showline [line] - Show specific line (accepts PHYSICAL or LOGICAL)");
        Serial.println("help        - This help message");
        Serial.println("=== LINE NUMBER EXPLANATION ===");
        Serial.println("PHYSICAL: Line number shown in 'read' command (1, 2, 3...)");
        Serial.println("LOGICAL: Line number stored in CSV file (second field)");
        Serial.println("Commands automatically try both types of line numbers");
        Serial.println("Use 'listlines' to see the mapping");
        Serial.println("=== FREE TEXT FORMAT ===");
        Serial.println("Free text is saved in field 8 without T: prefix");
        Serial.println("Data field (field 7) always contains: 'A:1,B:0,T:'");
        Serial.println("==============================");
    }
    else {
        Serial.println(">>> Unknown command. Type 'help' for available commands");
    }
}