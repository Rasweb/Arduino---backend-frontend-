import express from "express";
import { ZodError } from "zod";
import { TestStats } from "../data/types.js";
import { NewTestEntrySchema, zodFormat } from "../data/zodUtils.js";
import { db } from "../db/db.js";
import { tests } from "../db/schema.js";
const router = express.Router();

router.post("/addTestResult", async (req, res) => {
  console.log(">>> Incoming request from Arduino");
  console.log("Headers:", req.headers);
  console.log("Raw body:", req.body);
  try {

    // Parse and validate incoming Arduino JSON
    const parsed = NewTestEntrySchema.parse(req.body);

    // Use server time for consistency across all test entries
    const serverTimestamp = new Date().toISOString();

    // Generate a simple unique test ID based on device ID and current timestamp
    const simpleTestId = parsed.device_id + "_" + Date.now();

    // Create a new test object including all Arduino fields + timestamp
    const newTest = {
      lineNumber: parsed.lineNumber,
      test_id: simpleTestId,
      device_id: parsed.device_id,
      prodname: parsed.prodname,
      serialnr: parsed.serialnr,
      teststatus: parsed.teststatus,
      data: parsed.data, // dynamic object from DataSchema
      freeText: parsed.freeText,
      timestamp: serverTimestamp,
    };

    // Insert into database
    await db.insert(tests).values(newTest);


    console.log(`Test added from ${parsed.device_id}: ${JSON.stringify(parsed.data)}`);

    return res.status(200).json({
      success: true,
      message: "Test result saved successfully",
    });
  } catch (error) {
    if (error instanceof ZodError) {
      const formattedErrors = zodFormat(error);
      console.log("/addTestResult zod error", formattedErrors);
      return res.status(400).json({ message: "Bad Request", error: formattedErrors });
    } else {
      console.log("/addTestResult unkown error", error);
      return res.status(500).json({ message: "Internal server error" });
    }
  }
});

// Route: Clear all data
router.post("/clearAllData", async (req, res) => {
  try {
    let testStatistics: TestStats = res.locals.testStats;
    await db.delete(tests); // clear all records
    testStatistics.totalTests = 0;

    console.log("All test data cleared");

    return res.status(200).json({
      success: true,
      message: "All data cleared successfully",
    });
  } catch (error) {
    return res.status(500).json({ message: "Internal server error" });
  }
});

export default router;



