#ifndef CSV_PARSER_H
#define CSV_PARSER_H
#ifdef ARDUINO
    #include <Arduino.h>
#else 
    #include <stdint.h>
    #include <string.h>
#endif
#include "../types/state_types.h"

// Function declarations - NO POINTERS in interface
CSVParserResult parseCSVLine(const char csvLine[]);
const char* getCSVField(const CSVParserResult* result, int fieldIndex);
int getCSVFieldCount(const CSVParserResult* result);

#endif