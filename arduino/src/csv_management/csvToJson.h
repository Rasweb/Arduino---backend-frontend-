
#ifndef CSV_TO_JSON_H
#define CSV_TO_JSON_H
#include <Arduino.h>
#include "csv_parser.h"

String convertCsvLineToJson(const char csvLine[], int lineNumber = -1);
String convertCsvBatchToJson(const char csvData[]);
String escapeJsonString(const char input[]);

#endif