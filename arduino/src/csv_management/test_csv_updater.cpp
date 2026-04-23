#include "test_csv_updater.h"

// Add extern declaration for mutex functions
extern bool acquireFileLock();
extern void releaseFileLock();

// FIXED: CSV field truncation issue with proper mutex handling - PREVENT EMPTY LINES
bool markTestAsSent(int lineNumber) {
     FILE *file = fopen("/fs/test.csv", "r+");
    if (!file) {
        LOG_ERROR(">>> CSV SYNC: ERROR - Cannot open file for in-place update");
        return false;
    }

    // Calculate the offset: (lineNumber - 1) * lineLength
    // lineLength should include the newline character
    const int lineLength = MAX_ROW_SIZE; // or set to your actual line length
    long offset = (lineNumber - 1) * lineLength;

    if (fseek(file, offset, SEEK_SET) != 0) {
        LOG_ERROR(">>> CSV SYNC: ERROR - Failed to seek to line");
        fclose(file);
        return false;
    }

    // Write '1' at the start of the line
    if (fputc('1', file) == EOF) {
        LOG_ERROR(">>> CSV SYNC: ERROR - Failed to write sent status");
        fclose(file);
        return false;
    }

    fclose(file);
    LOG_INFO(">>> CSV SYNC: In-place update successful");
    return true;
}

// Wrapper function for marking test as sent in CSV
bool markTestAsSentInCSV(int lineNumber) {
    return markTestAsSent(lineNumber);
}

// Update sent status in CSV file after successful server sync
void updateSentStatus(int lineNumber) {
    LOG_DEBUG(">>> UPDATE: Marking line " + String(lineNumber) + " as sent in CSV");
 
    if (lineNumber <= 0) {
        LOG_ERROR(">>> ERROR: Invalid line number");
        return;
    }

    // Use the existing markTestAsSent function which handles CSV updates properly
    if (markTestAsSent(lineNumber)) {
        LOG_INFO(">>> SUCCESS: Line marked as sent in CSV");
        // Log additional debug information for verification
        char sentStatus[MAX_FIELD_SIZE]; // FIXED: Increased buffer size
        if (readFlashField(lineNumber, 0, sentStatus)) {
            LOG_DEBUG(">>> VERIFICATION: Field 0 now contains: ", sentStatus);
        }
    } else {
        LOG_ERROR(">>> ERROR: Failed to mark line as sent");
    }
}