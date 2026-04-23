export interface TestEntry {
  lineNumber?: number;                // optional, sent by Arduino
  test_id: string;                    // unique test identifier
  device_id: string;                  // device identifier
  prodname?: string;                  // optional product name
  serialnr?: string;                  // optional serial number
  teststatus?: string;                // optional test status
  data: {} | null;                    // dynamic object or null
  freeText?: string;                  // optional free text field
  timestamp: string;                  // added by server
}

// Represents aggregated statistics for test results - based on test status
export interface TestStats {
  totalTests: number;
  passedTests: number;
  failedTests: number;
}
