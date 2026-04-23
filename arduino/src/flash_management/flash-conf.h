
#ifndef FLASH_CONF_H
#define FLASH_CONF_H
#include <string.h>
#include <Arduino.h>
#include "mbed.h"
#include "FATFileSystem.h"
#include "MBRBlockDevice.h"
#include "../csv_management/csv_parser.h"
#include "../types/state_types.h"
#include "../../arduino_secrets.h"
#include "../logger/logger.h"

// CSV format definitions - UPDATED for 9 fields
#define WRITE_DATA_TYPES "%d,%d,%s,%s,%s,%s,%s,%s,%s"  // 9 fields 
#define WRITE_DATA_VALUES data.sent, data.lineNumber, safe_msgid, safe_testarid, safe_prodname, safe_serialnr, safe_teststatus, safe_data, safe_freeText

bool initFlash();
bool writeFlash(const testData &data);
void readFlash();
void readRaw();
void unmountFlash();  
void clearEntireFileFlash();

// Functions from other version
int getNextLineNumber();
void resetLineNumberCounter();
int getTotalLineCount();
bool readFlashField(int lineNumber, int fieldIndex, char result[MAX_FIELD_SIZE]);

// Text editing functions - ARRAYS
bool addTextToLine(int lineNumber, const char text[]);
bool updateLineWithText(int lineNumber, const char text[]);
bool showLine(int lineNumber);
bool readCompleteLine(int lineNumber, char result[MAX_ROW_SIZE]);
bool editLine(int lineNumber, const char newFreeText[]);

// Improved functions for fixed-length rows with guaranteed 256 characters
bool writeFixedLengthCSVRow(FILE *file, const testData &data);
String removeCharacterPadding(const char csvLine[]);  
bool rebuildLineWithNewText(int lineNumber, const char newFreeText[]);  

// Function to find highest line number in CSV file
int findHighestLineNumberInCSV();

// MUTEX FUNCTION DECLARATIONS:
bool acquireFileLock();
void releaseFileLock();
void forceReleaseFileLock();

void helperStringFunc(char dest[], const char src[], size_t length);

#endif