
#include "csvToJson.h"

String escapeJsonString(const char input[]) {
    String result = "";
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '\"') {
            result += "\\\"";
        } else if (input[i] == '\\') {
            result += "\\\\";
        } else {
            result += input[i];
        }
    }
    return result;
}

String convertCsvLineToJson(const char csvLine[], int lineNumber) {
    String json = "{\"lineNumber\":" + String(lineNumber) + ",\"fields\":[";
    
    // UPDATED: Use new CSV parser instead of global arrays
    CSVParserResult result = parseCSVLine(csvLine);
    int actualFieldCount = 0;
   
    // Count only non-padding fields
    for (int i = 0; i < getCSVFieldCount(&result); i++) {
        const char* field = getCSVField(&result, i);
        // Skip padding fields (those containing only "_")
        if (strcmp(field, "_") != 0) {
            actualFieldCount++;
        }
    }
   
    // Add non-padding fields to JSON
    int addedFields = 0;
    for (int i = 0; i < getCSVFieldCount(&result); i++) {
        const char* field = getCSVField(&result, i);
        // Skip padding fields
        if (strcmp(field, "_") != 0) {
            json += "\"" + escapeJsonString(field) + "\"";
            if (addedFields < actualFieldCount - 1) {
                json += ",";
            }
            addedFields++;
        }
    }
   
    json += "],\"fieldCount\":" + String(actualFieldCount) + "}";
    return json;
}

String convertCsvBatchToJson(const char csvData[]) {
    String json = "{\"lines\":[";
    char line[MAX_ROW_SIZE];
    int pos = 0;
    int lineNumber = 0;
    bool firstLine = true;
   
    while (csvData[pos] != '\0') {
        int linePos = 0;
        while (csvData[pos] != '\n' && csvData[pos] != '\0' && linePos < MAX_ROW_SIZE - 1) {
            line[linePos++] = csvData[pos++];
        }
        if (csvData[pos] == '\n') pos++;
        line[linePos] = '\0';
       
        if (strlen(line) > 0) {
            if (!firstLine) {
                json += ",";
            }
            json += convertCsvLineToJson(line, lineNumber);
            firstLine = false;
            lineNumber++;
        }
    }
   
    json += "]}";
    return json;
}