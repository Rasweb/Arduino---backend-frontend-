/*
- This Express router file manages the rendering and handling of product test results on the web application.
- It sets up the main routes, processes query parameters for filtering, and constructs the dynamic HTML content
  to display both test statistics and individual test results.

- Functions:
    - 1. limitResults
        Determines the limit of test results to display based on the provided query parameter. Ensures the limit is a positive integer,
        defaulting to 10 if the input is invalid. 
    
    - 2. getIdFilter
        Generates HTML markup for filtering tests by Device ID, including input fields and filter button.

    - 3. getAdvancedFilters
        Centralizes advanced filtering options.

    - 4. getLimitForm
        Produces HTML for a dropdown selection, enabling users to set limits for the number of test results retrieved.

    - 5. getHead
        Constructs the HTML head section, including title, metadata, and linked styles.

    - 6. getBody
        Builds the main body of the HTML, displaying total test statistics, controls for filtering and data management, 
        and a container for test results.

    - 7. getFooter
        Complements the body with a footer section that displays system status, last update time, and includes
        dynamically generated scripts for client-side functionality.

- Route Configuration:
    - The root route ("/") processes GET requests to display the product test results by:
        a. Retrieving all data directly from the database for statistics
        b. Handling query parameters to determine the limit of displayed tests
        c. Render HTML for both the statistics and the individual tests
        d. Returning the fully assembled HTML page to the client

- Error Handling:
    - Catches errors during the request handling process, including formatting errors using Zod.
        If an error is detected, it sends a corresponding HTTP response with detailed error messages.
    
*/
import express from "express";
import { Request, Response } from "express";
const router = express.Router();
import { TestEntry, TestStats } from "../data/types.js"
import { ZodError } from "zod";
import { zodFormat, isTestPassed, isTestFailed, getDisplayStatus } from "../data/zodUtils.js";
import { getStyle } from "./styles.js";
import { getScripts, getTestHeadClient } from "./scripts.js";
import { db } from "../db/db.js";

const limitResults = (currLimit: string | undefined) => {
    let limit = parseInt(currLimit as string) || 10;
    if(isNaN(limit) || limit <= 0){
        limit = 10;
    }
    return limit;
}

const getIdFilter = () => {
    return `
        <div>
            <h4>Filter Tests by Device ID</h4>
            <input type="text" id="deviceIdInput" placeholder="Enter Device ID">
        </div>
    `;
}

const getAdvancedFilters = () => {
    return `
    ${getIdFilter()}
    <div>
        <label for="start">Start:</label>
        <input type="datetime-local" id="start" name="start">
        <span id="startError" class="error-message"></span>

        <label for="end">End:</label>
        <input type="datetime-local" id="end" name="end">
        <span id="endError" class="error-message"></span>
    </div>
    <div>
        <button id="combinedButton">Apply filters</button>
    </div>
    `;
}

const getLimitForm = (currentLimit: number = 10) => {
    const options = [10, 20, 50, 80, 100, 200];
    let optionsHTML = '';
    
    options.forEach(option => {
        const selected = option === currentLimit ? 'selected' : '';
        optionsHTML += `<option value="${option}" ${selected}>${option}</option>`;
    });

    return `
        <form id="limitForm">
            <label for="limit">Choose a limit:</label>
            <select name="limit" id="limit">
                ${optionsHTML}
            </select>
            <button id="applyButton">Apply Limit</button>
        </form>
    `;
}

const getHead = () => {
    return `
        <head>
            <title>Product Test Results</title>
            <meta charset="utf-8">
            <meta name="viewport" content="width=device-width, initial-scale=1">
            ${getStyle()}
        </head>
    `;
}

const getBody = (dataObject: TestEntry[], currentLimit: number = 10) => {
    // Use utility functions for consistent status counting
    const passedTests = dataObject.filter(test => 
        isTestPassed(test.teststatus)
    ).length;
    
    const failedTests = dataObject.filter(test => 
        isTestFailed(test.teststatus)
    ).length;

    return `
    <div class="container">
        <h1>Product Test Results</h1>   
        <div class="stats">
            <div class="stat-item">
                <span class="stat-number">${dataObject.length}</span>
                <span class="stat-label">Total Tests</span>
            </div>
            <div class="stat-item">
                <span class="stat-number">${passedTests}</span>
                <span class="stat-label">Passed Tests</span>
            </div>
            <div class="stat-item">
                <span class="stat-number">${failedTests}</span>
                <span class="stat-label">Failed Tests</span>
            </div>
            <div class="stat-item">
                <span class="stat-number">${[...new Set(dataObject.map(test => test.device_id))].length}</span>
                <span class="stat-label">Active Devices</span>
            </div>
        </div>
        <div class="controls">
            <button id="refreshButton">Refresh Data</button>
            <button onclick="exportData()">Export Data</button>
            <button onclick="clearAllData()">Clear All Data</button>
            <div>${getLimitForm(currentLimit)}</div>   
            <button onclick="filterState()" id="filters-btn">Show filters</button>
            <div id="filters-container" style="display:none">
                <div>${getAdvancedFilters()}</div>
            </div>
        </div>
        <h2>Test Results</h2>
        <div id="tests-container">
            <!-- Initial test entries go here -->
        </div>
    `;
}

const getFooter = (dataObject: TestEntry[]) => {
    return `
        </div>
            <div style="text-align: center; margin-top: 20px; color: #666666;">
                <p>System Status: Operational</p>
                <p>Last update: <span id="last-update">${new Date().toLocaleString('sv-SE')}</span></p>
            </div>
        </div>
        <script>
            ${getScripts(dataObject)}
        </script>
    </body>
    </html>
    `;
}

router.get('/', async (req: Request, res: Response) => { 
    try {
        const { limit } = req.query;
        
        let limitValue = 10;
        if (typeof limit === 'string') {
            limitValue = parseInt(limit); 
        };
        
        // Get ALL data directly from database
        const rawData = await db.query.tests.findMany();
        
        // Convert database data to TestEntry type with proper data handling
        const allTests: TestEntry[] = rawData.map(test => ({
            test_id: test.test_id,
            device_id: test.device_id,
            lineNumber: test.lineNumber ?? undefined,
            data: test.data as { [key: string]: boolean } || {}, // Handle null data
            prodname: test.prodname ?? undefined,
            serialnr: test.serialnr ?? undefined,
            teststatus: test.teststatus ?? undefined,
            freeText: test.freeText ?? undefined,
            timestamp: test.timestamp
        })) as TestEntry[];
        
        // Use utility functions for consistent status analysis
        const passedTests = allTests.filter(test => isTestPassed(test.teststatus)).length;
        const failedTests = allTests.filter(test => isTestFailed(test.teststatus)).length;
        
        console.log("=== Test Status Summary ===");
        console.log("Total tests:", allTests.length);
        console.log("Passed tests:", passedTests);
        console.log("Failed tests:", failedTests);
        console.log("Other status tests:", allTests.length - passedTests - failedTests);
        console.log("===========================");

        // Get only the recent tests for display
        const recentTestsRaw = await db.query.tests.findMany({
            orderBy: (table, {desc}) => [desc(table.timestamp)],
            limit: limitValue,
        });

        // Convert recent tests with proper data handling
        const recentTests: TestEntry[] = recentTestsRaw.map(test => ({
            test_id: test.test_id,
            device_id: test.device_id,
            lineNumber: test.lineNumber ?? undefined,
            data: test.data as { [key: string]: boolean } || {}, // Handle null data
            prodname: test.prodname ?? undefined,
            serialnr: test.serialnr ?? undefined,
            teststatus: test.teststatus ?? undefined,
            freeText: test.freeText ?? undefined,
            timestamp: test.timestamp
        })) as TestEntry[];

        // Build HTML for test cards
        let entriesHTML = "";
        if (recentTests.length === 0) {
            entriesHTML += `<p style="text-align: center; color: #666666;">No tests recorded yet</p>`;
        } else {
            recentTests.forEach((test) => {
                const displayTime = new Date(test.timestamp + "UTC").toLocaleString()
                entriesHTML += getTestHeadClient(test, displayTime);
            });
        }

        // Use allTests for stats and export functionality
        const bodyWithEntries = getBody(allTests, limitValue).replace('<!-- Initial test entries go here -->', entriesHTML);

        const html = `
            <!DOCTYPE html>
            <html>
                ${getHead()}
            <body>
                ${bodyWithEntries}
                ${getFooter(allTests)}
        `;

        res.send(html);
    } catch (error) {
        if (error instanceof ZodError) {
            const formattedErrors = zodFormat(error);
            return res.status(400).json({message: "Bad Request", error: formattedErrors}); 
        } else {
            console.error('Error in router:', error);
            return res.status(500).json({message: "Internal server error"});
        }
    }
});

export default router;