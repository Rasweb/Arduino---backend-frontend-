import sanitizeHtml from "sanitize-html";
import { z, ZodError } from "zod";

export const envSchema = z.string().min(1);

export function sanitizeText(val: string) {
  return sanitizeHtml(val, {
    allowedTags: [],
    allowedAttributes: {},
  });
}

const safeString = z.string().transform(sanitizeText);

// Unified data schema for both Arduino strings and database objects
const DataSchema = z.union([
  // Handle Arduino data strings like "A1B0", "A0B1", "A1B1", "A0B0"
  z.string().transform((data) => {
    const result: { [key: string]: boolean } = {};
    const regex = /([A-Z])(\d)/g;
    let match;
    while ((match = regex.exec(data)) !== null) {
      const key = match[1];            // letter (A, B, C ...)
      const value = match[2] === "1";  // "1" becomes true, "0" becomes false
      result[`product${key}`] = value;
    }
    return result;
  }),
  // Handle database data objects
  z.record(z.string(), z.union([z.boolean(), z.number()])).transform((data) => {
    const result: { [key: string]: boolean } = {};
    for (const [key, value] of Object.entries(data)) {
      result[key] = Boolean(value); // Konvertera 1/0 till true/false
    }
    return result;
  })
]);

// Schema for new test entries (Arduino input)
export const NewTestEntrySchema = z.object({
  lineNumber: z.number().optional(),
  test_id: safeString,
  device_id: safeString,
  prodname: safeString.optional(),
  serialnr: safeString.optional(),
  teststatus: safeString.optional(),
  data: DataSchema,
  freeText: safeString.optional(),
});

export const zodFormat = (error: ZodError) => {
  const formattedErrors = error.issues.map((issue) => ({
    field: issue.path.join("."),
    message: issue.message,
    input: issue.input,
    code: issue.code,
  }));
  return formattedErrors;
};

// Test status analysis functions with improved logic
export const isTestPassed = (teststatus: string | undefined) => {
  if (!teststatus) return false;
  
  const status = teststatus.toLowerCase().trim();
  
  // Failed indicators take precedence
  const failedIndicators = ['✗', 'fail', 'error', 'failed', 'missed'];
  if (failedIndicators.some(indicator => status.includes(indicator))) {
    return false;
  }
  
  // Passed indicators
  const passedIndicators = ['✓', 'pass', 'success', 'ok', 'passed', 'complete'];
  return passedIndicators.some(indicator => status.includes(indicator));
};

export const isTestFailed = (teststatus: string | undefined) => {
  if (!teststatus) return false;
  
  const status = teststatus.toLowerCase().trim();
  
  // Passed indicators take precedence
  const passedIndicators = ['✓', 'pass', 'success', 'ok', 'passed', 'complete'];
  if (passedIndicators.some(indicator => status.includes(indicator))) {
    return false;
  }
  
  // Failed indicators
  const failedIndicators = ['✗', 'fail', 'error', 'failed', 'missed'];
  return failedIndicators.some(indicator => status.includes(indicator));
};

export const getTestStatusResult = (teststatus: string | undefined) => {
  return {
    isPassed: isTestPassed(teststatus),
    isFailed: isTestFailed(teststatus)
  };
};

// Helper function to get display status text
export const getDisplayStatus = (teststatus: string | undefined) => {
  if (!teststatus) return "Not Tested";
  
  if (isTestPassed(teststatus)) return "Passed";
  if (isTestFailed(teststatus)) return "Failed";
  
  return teststatus; // Return original status if not clearly passed/failed
};
