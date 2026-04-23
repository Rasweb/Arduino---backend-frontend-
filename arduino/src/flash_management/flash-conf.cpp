#include "flash-conf.h"

using namespace mbed;
FATFileSystem fs("fs");
BlockDevice &bd = *BlockDevice::get_default_instance();
MBRBlockDevice partition(&bd, 4);

bool flashInitialized = false;
static int currentLineNumber = 0;
static bool fileIsLocked = false;

// Safe string handling
void helperStringFunc(char dest[], const char src[], size_t length) {
    if (src == NULL) {
        dest[0] = '\0';
        return;
    }

    size_t src_len = strlen(src);
    if (src_len >= length) {
        LOG_WARN("WARNING: Text too long for field (max ");
        LOG_DEBUG("WARNING: Text too long for field (max " +  String(length - 1) + " chars). Truncating from " + String(src_len) + " chars.");
    }

    size_t copy_len = (src_len < (length - 1)) ? src_len : (length - 1);
    strncpy(dest, src, copy_len);
    dest[copy_len] = '\0';
}

// File locking
bool acquireFileLock() {
    unsigned long startTime = millis();
    const unsigned long TIMEOUT_MS = 100;
    while (fileIsLocked && (millis() - startTime < TIMEOUT_MS)) {
        delay(5);
    }
    if (fileIsLocked) {
        LOG_WARN(">>> FILE LOCK: Timeout - forcing release");
        fileIsLocked = false;
    }
    fileIsLocked = true;
    return true;
}

void releaseFileLock() {
    fileIsLocked = false;
}

void forceReleaseFileLock() {
    fileIsLocked = false;
    LOG_WARN(">>> FILE LOCK: Forced release");
}

// Padding
String removeCharacterPadding(const char csvLine[]) {
    String result = csvLine;
    int lastNonUnderscore = result.length() - 1;
    while (lastNonUnderscore >= 0 && result[lastNonUnderscore] == '_') {
        lastNonUnderscore--;
    }
    return (lastNonUnderscore >= 0) ? result.substring(0, lastNonUnderscore + 1) : "";
}

// Write fixed-length row
bool writeFixedLengthCSVRow(FILE *file, const testData &data) {
    if (!file) return false;

    char fullLine[256];
    int written = snprintf(fullLine, 256, "%d,%d,%s,%s,%s,%s,%s,%s,%s",
        data.sent, data.lineNumber, data.msgid, data.testarid,
        data.prodname, data.serialnr, data.teststatus, data.data, data.freeText);

    for (int i = written; i < 255; i++) fullLine[i] = '_';
    fullLine[255] = '\n';

    return fwrite(fullLine, 1, 256, file) == 256;
}

// Write new testData
bool writeFlash(const testData &data) {
    if (!acquireFileLock()) return false;

    bool success = false;
    FILE *file = fopen("/fs/test.csv", "r+");
    if (!file) file = fopen("/fs/test.csv", "w+");

    if (file) {
        fseek(file, 0, SEEK_END);
        success = writeFixedLengthCSVRow(file, data);
        fclose(file);
    }

    releaseFileLock();
    return success;
}

// Read entire file
void readFlash() {
    if (!acquireFileLock()) return;

    FILE *file = fopen("/fs/test.csv", "r");
    if (file) {
        char buf[256];
        int lineNum = 0;
        while (fgets(buf, sizeof(buf), file)) {
            buf[strcspn(buf, "\n")] = '\0';
            if (strlen(buf) > 0) {
                Serial.print("Line ");
                Serial.print(++lineNum);
                Serial.print(": ");
                Serial.println(buf);
            }
        }
        fclose(file);
    }
    releaseFileLock();
}

void readRaw() {
    if (!acquireFileLock()) return;

    FILE *file = fopen("/fs/test.csv", "r");
    if (file) {
        char buf[10];
        Serial.println(">>> RAW FILE CONTENT:");
        while (fgets(buf, sizeof(buf), file)) {
            Serial.print(buf);
        }
        fclose(file);
        Serial.println(">>> END OF RAW FILE CONTENT");
    }
    releaseFileLock();
}

// Clear file
void clearEntireFileFlash() {
    if (!acquireFileLock()) return;

    FILE *file = fopen("/fs/test.csv", "w");
    if (file) fclose(file);
    resetLineNumberCounter();
    releaseFileLock();
}

// Read full line by logical line number
bool readCompleteLine(int lineNumber, char result[256]) {
    if (lineNumber <= 0 || !acquireFileLock()) return false;

    FILE *file = fopen("/fs/test.csv", "r");
    if (!file) {
        releaseFileLock();
        return false;
    }

    char buf[256];
    while (fgets(buf, sizeof(buf), file)) {
        buf[strcspn(buf, "\n")] = '\0';
        CSVParserResult parsed = parseCSVLine(buf);
        if (getCSVFieldCount(&parsed) >= 2) {
            int currentLine = atoi(getCSVField(&parsed, 1));
            if (currentLine == lineNumber) {
                strncpy(result, buf, 255);
                result[255] = '\0';
                fclose(file);
                releaseFileLock();
                return true;
            }
        }
    }

    fclose(file);
    releaseFileLock();
    return false;
}

// Read specific field
bool readFlashField(int lineNumber, int fieldIndex, char result[MAX_FIELD_SIZE]) {
    if (lineNumber <= 0 || !acquireFileLock()) return false;

    FILE *file = fopen("/fs/test.csv", "r");
    if (!file) {
        releaseFileLock();
        return false;
    }

    char buf[256];
    while (fgets(buf, sizeof(buf), file)) {
        buf[strcspn(buf, "\n")] = '\0';
        CSVParserResult parsed = parseCSVLine(buf);
        int currentLine = atoi(getCSVField(&parsed, 1));
        if (currentLine == lineNumber) {
            String unpadded = removeCharacterPadding(buf);
            CSVParserResult clean = parseCSVLine(unpadded.c_str());
            if (fieldIndex < getCSVFieldCount(&clean)) {
                strncpy(result, getCSVField(&clean, fieldIndex), MAX_FIELD_SIZE - 1);
                result[MAX_FIELD_SIZE - 1] = '\0';
                fclose(file);
                releaseFileLock();
                return true;
            }
        }
    }

    fclose(file);
    releaseFileLock();
    return false;
}

// Show line
bool showLine(int lineNumber) {
    char line[256];
    if (readCompleteLine(lineNumber, line)) {
        String clean = removeCharacterPadding(line);
        LOG_DEBUG("Line " + String(lineNumber) + ": " + clean);
        return true;
    }
    return false;
}

// Add text to line
bool addTextToLine(int lineNumber, const char text[]) {
    char existing[64];
    if (!readFlashField(lineNumber, 8, existing)) existing[0] = '\0';

    char combined[128];
    snprintf(combined, sizeof(combined), "%s %s", existing, text);
    return rebuildLineWithNewText(lineNumber, combined);
}

// Replace freeText
bool updateLineWithText(int lineNumber, const char text[]) {
    return rebuildLineWithNewText(lineNumber, text);
}

// Edit line
bool editLine(int lineNumber, const char newFreeText[]) {
    return rebuildLineWithNewText(lineNumber, newFreeText);
}

// FIXED: Thread-safe file operations in rebuildLineWithNewText with proper padding math
bool rebuildLineWithNewText(int lineNumber, const char newFreeText[]) {
    if (lineNumber <= 0) {
        LOG_ERROR("ERROR: Invalid line number " + String(lineNumber));
        return false;
    }

    // Acquire file lock for the entire update operation
    if (!acquireFileLock()) {
        LOG_ERROR("ERROR: Could not acquire file lock for editing");
        return false;
    }

    bool success = false;
   
    LOG_DEBUG("DEBUG: Searching for line " + String(lineNumber));

    // Read the existing line
    char existingLine[256];
    bool lineFound = false;
    int physicalLineNumber = 0;
   
    FILE *file = fopen("/fs/test.csv", "r");
    if (!file) {
        LOG_ERROR("ERROR: Cannot open CSV file for reading");
        releaseFileLock();
        return false;
    }
   
    char buf[256];
    while (fgets(buf, sizeof(buf), file)) {
        physicalLineNumber++;
        buf[strcspn(buf, "\n")] = '\0';
       
        if (strlen(buf) == 0) {
            continue;
        }
       
        // UPDATED: Use new CSV parser instead of global arrays
        CSVParserResult result = parseCSVLine(buf);
       
        if (getCSVFieldCount(&result) >= 2) {
            int currentLogicalLine = atoi(getCSVField(&result, 1));
           
            if (currentLogicalLine == lineNumber) {
                strncpy(existingLine, buf, sizeof(existingLine) - 1);
                existingLine[sizeof(existingLine) - 1] = '\0';
                lineFound = true;
                LOG_DEBUG("Found line at physical position " + String(physicalLineNumber));
                break;
            }
        }
    }
    fclose(file);

    if (!lineFound) {
        LOG_ERROR("ERROR: Line " + String(lineNumber) + " not found in CSV file");
        releaseFileLock();
        return false;
    }

    // Parse the existing line to extract data
    String cleanLine = removeCharacterPadding(existingLine);
    CSVParserResult parsed = parseCSVLine(cleanLine.c_str());
    if (getCSVFieldCount(&parsed) < 8) {
        LOG_ERROR("ERROR: Invalid CSV format in existing line");
        releaseFileLock();
        return false;
    }

    // Create updated data structure
    testData updatedData;
    memset(&updatedData, 0, sizeof(testData));
    updatedData.sent = atoi(getCSVField(&parsed, 0));
    updatedData.lineNumber = atoi(getCSVField(&parsed, 1));
    helperStringFunc(updatedData.msgid, getCSVField(&parsed, 2), 24);
    helperStringFunc(updatedData.testarid, getCSVField(&parsed, 3), 16);
    helperStringFunc(updatedData.prodname, getCSVField(&parsed, 4), 16);
    helperStringFunc(updatedData.serialnr, getCSVField(&parsed, 5), 16);
    helperStringFunc(updatedData.teststatus, getCSVField(&parsed, 6), 8);
    helperStringFunc(updatedData.data, getCSVField(&parsed, 7), 32);
    helperStringFunc(updatedData.freeText, newFreeText, 64);

    // Create temporary file for updating
    FILE *tempFile = fopen("/fs/test_temp.csv", "w");
    FILE *originalFile = fopen("/fs/test.csv", "r");
   
    if (!tempFile || !originalFile) {
        LOG_ERROR("ERROR: Cannot open files for editing");
        if (tempFile) fclose(tempFile);
        if (originalFile) fclose(originalFile);
        releaseFileLock();
        return false;
    }

    char buf2[256];
    int currentPhysicalLine = 0;
    success = true;

    while (fgets(buf2, sizeof(buf2), originalFile)) {
        currentPhysicalLine++;
        buf2[strcspn(buf2, "\n")] = '\0';
       
        if (strlen(buf2) == 0) {
            continue;
        }
       
        // Parse each line to check if it's our target
        char tempLine[256];
        strcpy(tempLine, buf2);
        CSVParserResult lineResult = parseCSVLine(tempLine);
       
        if (getCSVFieldCount(&lineResult) >= 2) {
            int currentLogicalLine = atoi(getCSVField(&lineResult, 1));
           
            if (currentLogicalLine == lineNumber) {
                // This is the line we want to update
                LOG_DEBUG("Writing updated line...");
                if (!writeFixedLengthCSVRow(tempFile, updatedData)) {
                    LOG_ERROR("Failed to write updated line");
                    success = false;
                    break;
                }
                LOG_DEBUG("SUCCESS: Line " + String(lineNumber) + " updated with free text: '" + String(newFreeText) + "'");
            } else {
                // FIXED: Use proper fixed-length writing for non-target lines
                // Write the line with proper padding using writeFixedLengthCSVRow
                String unpaddedLine = removeCharacterPadding(buf2);
                CSVParserResult result = parseCSVLine(unpaddedLine.c_str());
               
                if (getCSVFieldCount(&result) >= 9) {
                    testData existingData;
                    memset(&existingData, 0, sizeof(testData));
                   
                    // Populate existingData from parsed result
                    existingData.sent = atoi(getCSVField(&result, 0));
                    existingData.lineNumber = atoi(getCSVField(&result, 1));
                    helperStringFunc(existingData.msgid, getCSVField(&result, 2), 24);
                    helperStringFunc(existingData.testarid, getCSVField(&result, 3), 16);
                    helperStringFunc(existingData.prodname, getCSVField(&result, 4), 16);
                    helperStringFunc(existingData.serialnr, getCSVField(&result, 5), 16);
                    helperStringFunc(existingData.teststatus, getCSVField(&result, 6), 8);
                    helperStringFunc(existingData.data, getCSVField(&result, 7), 32);
                    helperStringFunc(existingData.freeText, getCSVField(&result, 8), 64);
                   
                    // Write with proper fixed-length formatting
                    if (!writeFixedLengthCSVRow(tempFile, existingData)) {
                        LOG_ERROR("Failed to write existing line");
                        success = false;
                        break;
                    }
                } else {
                    // Fallback: Copy line as-is if parsing fails
                    if (fprintf(tempFile, "%s\n", buf2) < 0) {
                        LOG_ERROR("Failed to copy line");
                        success = false;
                        break;
                    }
                }
            }
        } else {
            // Copy lines that don't match format as-is (with padding)
            if (fprintf(tempFile, "%s\n", buf2) < 0) {
                LOG_ERROR("Failed to copy line");
                success = false;
                break;
            }
        }
    }

    fclose(tempFile);
    fclose(originalFile);

    if (!success) {
        LOG_ERROR("File operation failed");
        remove("/fs/test_temp.csv");
        releaseFileLock();
        return false;
    }

    // Replace original file with updated version
    if (remove("/fs/test.csv") != 0) {
        LOG_ERROR("Could not remove original file");
        remove("/fs/test_temp.csv");
        releaseFileLock();
        return false;
    }

    if (rename("/fs/test_temp.csv", "/fs/test.csv") != 0) {
        LOG_ERROR("Could not rename temp file");
        releaseFileLock();
        return false;
    }

    LOG_DEBUG("File updated successfully with preserved data");
    releaseFileLock();
    return true;
}

int getNextLineNumber() {
    currentLineNumber++;
    LOG_DEBUG(">>> FLASH: Next line number: " + String(currentLineNumber));
    return currentLineNumber;
}

void resetLineNumberCounter() {
    currentLineNumber = 0;
    LOG_DEBUG(">>> FLASH: Line number counter reset to 0");
}

int getTotalLineCount() {
    if (!acquireFileLock()) return 0;

    FILE *file = fopen("/fs/test.csv", "r");
    if (!file) {
        releaseFileLock();
        return 0;
    }

    int lineCount = 0;
    char buf[256];
    while (fgets(buf, sizeof(buf), file)) {
        if (strlen(buf) > 0 && buf[0] != '\n' && buf[0] != '\r') {
            lineCount++;
        }
    }

    fclose(file);
    releaseFileLock();
    return lineCount;
}

int findHighestLineNumberInCSV() {
    FILE *file = fopen("/fs/test.csv", "r");
    if (!file) return 0;

    int highestLineNumber = 0;
    char buf[256];

    while (fgets(buf, sizeof(buf), file)) {
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) > 0) {
            CSVParserResult result = parseCSVLine(buf);
            if (getCSVFieldCount(&result) >= 2) {
                int currentLineNumber = atoi(getCSVField(&result, 1));
                if (currentLineNumber > highestLineNumber) {
                    highestLineNumber = currentLineNumber;
                }
            }
        }
    }

    fclose(file);
    return highestLineNumber;
}

bool initFlash() {
    if (flashInitialized) return true;

    if (bd.init()) return false;
    if (partition.init()) {
        bd.deinit();
        return false;
    }

    if (fs.mount(&partition)) {
        fs.reformat(&partition);
        if (fs.mount(&partition)) {
            partition.deinit();
            bd.deinit();
            return false;
        }
    }

    int highestLineNumber = findHighestLineNumberInCSV();
    currentLineNumber = (highestLineNumber > 0) ? highestLineNumber : 0;
    flashInitialized = true;
    return true;
}

void unmountFlash() {
    fs.unmount();
    partition.deinit();
    bd.deinit();
    flashInitialized = false;
}