#include "test_csv_scanner.h"

// Add extern declaration for mutex functions
extern bool acquireFileLock();
extern void releaseFileLock();

// NEW: Optimized scanning function for fast unsent test detection - UPDATED: Use new CSV parser
void findUnsentTestsFast() {
    LOG_INFO(">>> FAST SCAN: Scanning for unsent tests (optimized method)...");
    setUnsentLineCount(0); // Use accessor function instead of direct variable access
    
    // Acquire file lock before scanning
    if (!acquireFileLock()) {
        LOG_INFO(">>> FAST SCAN: File locked, skipping scan");
        return;
    }
   
    FILE *file = fopen("/fs/test.csv", "r");
    if (!file) {
        LOG_INFO(">>> FAST SCAN: No CSV file found");
        releaseFileLock();
        return;
    }
   
    char lineBuffer[MAX_ROW_SIZE];
    int currentLine = 0;
    unsigned long scanStartTime = millis();
    int totalLinesScanned = 0;
    int emptyLines = 0;
   
    // Read file line by line looking for unsent tests (field 0 = "0")
    while (fgets(lineBuffer, sizeof(lineBuffer), file) && getUnsentLineCount() < MAX_UNSENT_TESTS) {
        currentLine++;
        totalLinesScanned++;
       
        // Skip empty lines to improve performance
        if (strlen(lineBuffer) == 0 || (strlen(lineBuffer) == 1 && lineBuffer[0] == '\n')) {
            emptyLines++;
            continue;
        }
       
        // Safety check to prevent buffer overflow
        if (getUnsentLineCount() >= MAX_UNSENT_TESTS) {
            LOG_WARN(">>> WARNING: Reached maximum unsent tests limit");
            break;
        }
       
        // Parse CSV line using new parser (no global variables)
        CSVParserResult result = parseCSVLine(lineBuffer);
       
        // Check if first field is "0" (unsent) and we have at least 1 field
        if (getCSVFieldCount(&result) >= 1 && strcmp(getCSVField(&result, 0), "0") == 0) {
            // Use the new accessor function to set individual array element
            setUnsentLineNumber(getUnsentLineCount(), currentLine);
            setUnsentLineCount(getUnsentLineCount() + 1); // Increment count
           
            // Log first 10 findings for debugging
            if (getUnsentLineCount() < 10) {
                LOG_INFO(">>> FAST SCAN: Found unsent test at line ", currentLine);
            }
        }
       
        // Skip to next line if we haven't reached the end of this line
        int len = strlen(lineBuffer);
        if (len > 0 && lineBuffer[len-1] != '\n') {
            int c;
            while ((c = fgetc(file)) != EOF && c != '\n') {
                // Consume characters until newline or EOF
            }
        }
    }
   
    fclose(file);
    
    // Always release the file lock
    releaseFileLock();
   
    // Report scanning statistics
    unsigned long scanTime = millis() - scanStartTime;
    LOG_INFO(">>> FAST SCAN: Found " + String(getUnsentLineCount()) + " unsent tests in " + String(scanTime) + " ms (scanned "  + String(totalLinesScanned) + " lines, " + String(emptyLines) + " empty lines skipped)");
   
    // Summary for large numbers of unsent tests
    if (getUnsentLineCount() >= 10) {
        LOG_INFO(">>> FAST SCAN: ... and " + String(getUnsentLineCount() - 10) + " more unsent tests");
    }
   
    if (getUnsentLineCount() == 0 && totalLinesScanned > 0) {
        LOG_INFO(">>> FAST SCAN: No unsent tests found - all tests may already be sent");
    }
}

// Function to read complete testData from CSV line
testData readCompleteTestDataFromCSV(int lineNumber) {
    testData result;
    memset(&result, 0, sizeof(testData));  // Nollställ struct

    if (lineNumber <= 0) return result;
    
    if (!acquireFileLock()) return result;
    
    FILE *file = fopen("/fs/test.csv", "r");
    if (!file) {
        releaseFileLock();
        return result;
    }
    
    char buf[MAX_ROW_SIZE];
    int currentLine = 1;
    bool found = false;

    // Should probably be replaced with fread and fseek.
    while (fgets(buf, sizeof(buf), file)) {
        LOG_DEBUG(">>> READ CSV: Read line " + String(currentLine) + ": " + String(buf));
        LOG_DEBUG(">>> READ CSV: Scanning for line " + String(lineNumber));
        if(strlen(buf) == 1) {
            LOG_DEBUG(">>> READ CSV: Skipping empty line");
            continue;
        }

        if (currentLine == lineNumber) {
            String cleanLine = removeCharacterPadding(buf);
            LOG_DEBUG(">>> READ CSV: Found target line: " + cleanLine);
            CSVParserResult parsed = parseCSVLine(cleanLine.c_str());
            LOG_DEBUG(">>> READ CSV: Parsed field count: " + String(getCSVFieldCount(&parsed)));
            
            if (getCSVFieldCount(&parsed) >= 9) {
                // Fyll hela testData struct
                result.sent = atoi(getCSVField(&parsed, 0));
                result.lineNumber = atoi(getCSVField(&parsed, 1));
                helperStringFunc(result.msgid, getCSVField(&parsed, 2), 24);
                helperStringFunc(result.testarid, getCSVField(&parsed, 3), 16);
                helperStringFunc(result.prodname, getCSVField(&parsed, 4), 16);
                helperStringFunc(result.serialnr, getCSVField(&parsed, 5), 16);
                helperStringFunc(result.teststatus, getCSVField(&parsed, 6), 8);
                helperStringFunc(result.data, getCSVField(&parsed, 7), 32);
                helperStringFunc(result.freeText, getCSVField(&parsed, 8), 64);
                found = true;
            }
            break;
        }
        currentLine++;
    }

    fclose(file);
    releaseFileLock();
    return result;
}

// Legacy function name for compatibility - calls the optimized version
void findUnsentTestsFromCSV() {
    findUnsentTestsFast();
}

// Return count of unsent tests (compatibility function)
int getUnsentTestCount() {
    return getUnsentLineCount(); // Use accessor function
}

// Initialize CSV-based synchronization system
void initializeCSVSync() {
    setUnsentLineCount(0); // Use accessor function
    setCsvSyncEnabled(true); // Enable CSV-based sync by default
    LOG_INFO(">>> CSV SYNC: Initialized CSV-based sync system with fast scanning");
    findUnsentTestsFast(); // Perform initial scan
}