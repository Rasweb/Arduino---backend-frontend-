/*
- This file sets up an Express web server that facilitates the management and presentation of test data.
  It encompasses routing for HTML rendering, API requests, and health checks, while also managing data validation 
  and error handling.

- Components:
  - 1. Initialization
      Instantiates the Express application, specifying the server's port and IP address.
      Enables JSON parsing for incoming requests.

  - 2. Data Management
      Defines a constant for the data source and initializes statistics for test results.
      Loads existing test data from the database and computes statistics based on test status.

  - 3. Middleware
      Attaches test statistics and data to the response locals for HTML routes.
      Configures routing for:
        a. HTML rendering via the "htmlRoutes" module.
        b. Test data management through the "testManagerRoutes" module.
        c. API interactions using the "apiRoutes" module.
        d. Health checks managed by the "healthRoutes" module.
    
  - 4. Error Handling
      Implements custom error handling middleware to address runtime exceptions and 404 errors.

  - 5. Server Configuration
      Launches the server and logs essential information about available routes and loaded test records.

- Overall, this file establishes a foundational framework for a web application dedicated to managing and displaying test results.
*/
import express from "express";
import { TestEntry, TestStats } from "./data/types.js";
import { isTestFailed, isTestPassed } from "./data/zodUtils.js";
import { db } from "./db/db.js";
import { errorHandler, notFoundHandler } from "./error/errorHandlers.js";
import apiRoutes from "./routes/api.js";
import healthRoutes from "./routes/health.js";
import testManagerRoutes from "./routes/testManager.js";
import htmlRoutes from "./views/html.js";

const app = express();
const port = 3000;
const ip = "0.0.0.0";

app.use(express.json());

// Initialize statistics based on test status
let testStatistics: TestStats = {
  totalTests: 0,
  passedTests: 0,
  failedTests: 0,
};

let dataObject: TestEntry[] = [];

// Function to load data from database
const loadDataFromDatabase = async (): Promise<void> => {
  try {
    console.log("Loading test data from database...");

    const rawData = await db.query.tests.findMany();

    // Convert null values to undefined for Zod validation and handle null data
    const dataObject = rawData.map((test) => ({
      ...test,
      lineNumber: test.lineNumber ?? undefined,
      prodname: test.prodname ?? undefined,
      serialnr: test.serialnr ?? undefined,
      teststatus: test.teststatus ?? undefined,
      freeText: test.freeText ?? undefined,
      data: test.data || {}, // Convert null data to empty object
    }));

    // Calculate statistics based on test status
    let passedTests = 0;
    let failedTests = 0;

    dataObject.forEach((test) => {
      if (isTestPassed(test.teststatus)) {
        passedTests++;
      } else if (isTestFailed(test.teststatus)) {
        failedTests++;
      }
    });

    // Update statistics directly without validation
    testStatistics = {
      totalTests: dataObject.length,
      passedTests: passedTests,
      failedTests: failedTests,
    };

    console.log(
      `Successfully loaded ${dataObject.length} test records from database`
    );
  } catch (error) {
    console.error("Failed to load data from database:", error);
    // Keep default empty statistics if loading fails
  }
};

// Initialize server with data loading
const initializeServer = async (): Promise<void> => {
  try {
    // Load data before starting the server
    await loadDataFromDatabase();

    // Setup middleware
    app.use(
      "/",
      (req, res, next) => {
        res.locals.testStats = testStatistics;
        res.locals.dataObj = dataObject;
        next();
      },
      htmlRoutes
    );

    // Middleware for routes
    app.use("/", htmlRoutes);

    app.use("/manageTests", testManagerRoutes);

    app.use("/api", apiRoutes);
    app.use("/health", healthRoutes);

    app.use("/api", apiRoutes);
    app.use("/health", healthRoutes);

    // Error handling
    app.use(errorHandler);
    app.use(notFoundHandler);

    // Start server
    app.listen(port, ip, () => {
      console.log(`GET / - Web interface`);
      console.log(`GET /api/tests - All tests`);
      console.log(`GET /api/tests/:device_id - Tests by device`);
      console.log(`GET /getLastTestId/:device_id - Latest test ID for device`);
      console.log(`POST /addTestResult - Add single test`);
      console.log(`POST /syncTests - Sync multiple tests`);
      console.log(`Server running at http://localhost:${port}`);
      console.log(`Loaded ${dataObject.length} existing test records`);
    });
  } catch (error) {
    console.error("Failed to initialize server:", error);
    process.exit(1);
  }
};

// Start the server
initializeServer();

// Optional: Function to reload data (can be used for manual refresh)
export const reloadTestData = async (): Promise<void> => {
  await loadDataFromDatabase();
};
