#include "../csv_management/csv_parser.h"
#include <stdio.h>


struct TestCase {
    const char* line;          // CSV input line
    int fieldIndex;            // Which field to check
    int expectedCount;         // Expected number of fields
    const char* expectedField; // Expected field value at that index
};

TestCase tests[] = {
    {"value1,value2,asd,\"v,alue3\"", 1, 4, "value2"},
    {"1,2,3,4", 2, 4, "3"},
    {"a,,c", 1, 3, ""}, 
    {"\"quoted,field\",x", 0, 2, "quoted,field"},
    {"singlefield", 0, 1, "singlefield"}
};

bool runTest(const TestCase& t);
bool displayResults();

int main(){
    printf("Test results:\n");
    bool allTestPassed = displayResults();

    if(allTestPassed){
        printf("All tests passed\n");
        return 0;
    } else {
        printf("One or more Tests failed\n");
        return 1;
    }
};

bool runTest(const TestCase& t){
    printf("==============================\n");
    printf("Input csvLine: %s\n", t.line);
    printf("Input FieldIndex: %d\n", t.fieldIndex);
    printf("Input expectedNumber: %d\n", t.expectedCount);
  
    // Initialize struct
    CSVParserResult result = parseCSVLine(t.line);
    const char* currentField = getCSVField(&result, t.fieldIndex);
    int fieldCount = getCSVFieldCount(&result);

    bool isFieldCorrect = currentField && strcmp(currentField, t.expectedField) == 0;
    bool isCountCorrect = fieldCount == t.expectedCount;
    
    printf("Check field value: %s (got '%s', expected '%s')\n",
        isFieldCorrect ? "PASS" : "FAIL",
        currentField ? currentField : "(null)", t.expectedField
    );
    printf("Check field count: %s (got %d, expected %d)\n",
        isCountCorrect ? "PASS" : "FAIL",
        fieldCount, t.expectedCount
    );

    // Final result
    bool finalResult = isFieldCorrect && isCountCorrect;
    printf("Overall result: %s\n", finalResult ? "PASS" : "FAIL");
    printf("==============================\n");
    return finalResult;
};
bool displayResults(){
    const int numTests = sizeof(tests) / sizeof(TestCase);
    printf("Running %d tests...\n", numTests);

    bool allPassed = true;

    for (int i = 0; i < numTests; ++i) {
        bool result = runTest(tests[i]);
        printf("Test %d: %s\n", i + 1, result ? "PASS" : "FAIL");
        if(!result) {
            allPassed = false;
        }
    }
    return allPassed;
}