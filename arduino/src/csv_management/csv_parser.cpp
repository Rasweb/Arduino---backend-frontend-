
#include "csv_parser.h"

// Legacy global variables for compatibility - WILL BE REMOVED
char currentCSVRow[MAX_ROW_SIZE] = "";
int fieldCount = 0;

// Parse CSV line into result structure - NO POINTERS
CSVParserResult parseCSVLine(const char csvLine[]) {
    CSVParserResult result;
    result.fieldCount = 0;
   
    // Initialize all fields to empty strings
    for (int i = 0; i < MAX_FIELDS; i++) {
        result.fields[i][0] = '\0';
    }
   
    int fieldStart = 0;
    int lineLength = strlen(csvLine);
    bool insideQuotes = false;
   
    for (int i = 0; i <= lineLength && result.fieldCount < MAX_FIELDS; i++) {
        // Handle quotes
        if (csvLine[i] == '\"') {
            insideQuotes = !insideQuotes;
            continue;
        }
       
        // Field separator or end of line
        if ((csvLine[i] == ',' || csvLine[i] == '\0' || csvLine[i] == '\n' || csvLine[i] == '\r') && !insideQuotes) {
            int fieldLength = i - fieldStart;
           
            // FIX: Include empty fields (fieldLength can be 0)
            if (fieldLength < MAX_FIELD_SIZE) {
                // Copy field content directly to array
                strncpy(result.fields[result.fieldCount], csvLine + fieldStart, fieldLength);
                result.fields[result.fieldCount][fieldLength] = '\0';
                result.fieldCount++;
            } else if (fieldLength >= MAX_FIELD_SIZE) {
                // Handle field truncation
                strncpy(result.fields[result.fieldCount], csvLine + fieldStart, MAX_FIELD_SIZE - 1);
                result.fields[result.fieldCount][MAX_FIELD_SIZE - 1] = '\0';
                result.fieldCount++;
            }
            fieldStart = i + 1;
        }
    }
   
    return result;
}

// Get field from parsed result - NO POINTERS
const char* getCSVField(const CSVParserResult* result, int fieldIndex) {
    if (fieldIndex < 0 || fieldIndex >= result->fieldCount || fieldIndex >= MAX_FIELDS) {
        return ""; // Return empty string for invalid index
    }
    return result->fields[fieldIndex];
}

// Get field count from parsed result
int getCSVFieldCount(const CSVParserResult* result) {
    return result->fieldCount;
}
