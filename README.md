# Product Test Results Server

A Node.js/Express server for collecting and displaying product test results with a web interface and REST API.

## Features

- **Web Interface**: Real-time dashboard showing test statistics and results
- **REST API**: JSON endpoints for programmatic access
- **Data Persistence**: Automatic saving to JSON file
- **Device Management**: Support for multiple test devices
- **Statistics**: Real-time counters for Product A and Product B tests
- **Filtering**: Filter tests by device and product type
- **Data Export**: Download test data as JSON

## Installation

1. Install Node.js dependencies:
```bash
npm install express
Start the server:

bash
node server.js
Open your browser to:

text
http://localhost:3000
API Endpoints
Web Interface
GET / - Web dashboard

Test Management
GET /api/tests - Get all tests

GET /api/tests/:device_id - Get tests by device

POST /addTestResult - Add single test result

POST /syncTests - Sync multiple test results

GET /getLastTestId/:device_id - Get latest test ID for device

Utility
GET /api/stats - Get statistics

GET /logAllEntries - Log all entries to console

GET /health - Health check

POST /addRandomTest - Add random test (for testing)

Data Structure
Test results are stored with the following format:

json
{
  "test_id": "string",
  "data": {
    "productA": boolean,
    "productB": boolean
  },
  "timestamp": "ISO-string",
  "device_id": "string"
}
Adding Test Results
Single Test
bash
curl -X POST http://localhost:3000/addTestResult \
  -H "Content-Type: application/json" \
  -d '{
    "test_id": "TEST_123",
    "productA": true,
    "productB": false,
    "timestamp": "2023-10-15T12:00:00.000Z",
    "device_id": "DEVICE_001"
  }'
Sync Multiple Tests
bash
curl -X POST http://localhost:3000/syncTests \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "DEVICE_001",
    "tests": [
      {
        "test_id": "TEST_124",
        "productA": false,
        "productB": true,
        "timestamp": "2023-10-15T12:05:00.000Z"
      }
    ]
  }'
Configuration
Port: 3000

IP: 0.0.0.0 (accessible from any network interface)

Data File: testData.json (automatically created)

Features Details
Automatic Data Persistence: Data is automatically saved to testData.json

Duplicate Prevention: Prevents duplicate test IDs for the same device

Real-time Statistics: Live counters for total tests and product-specific tests

Device Filtering: View tests from specific devices

Product Filtering: Filter by Product A, Product B, or both

Auto-refresh: Web interface updates every 30 seconds

Usage Examples
Get Statistics
bash
curl http://localhost:3000/api/stats
Get Tests for Specific Device
bash
curl http://localhost:3000/api/tests/DEVICE_001
Health Check
bash
curl http://localhost:3000/health
Data File Format
The server uses JSON format for data storage:

json
[
  {
    "test_id": "TEST_123",
    "data": {
      "productA": true,
      "productB": false
    },
    "timestamp": "2023-10-15T12:00:00.000Z",
    "device_id": "DEVICE_001"
  }
]
The server automatically handles file creation, reading, and writing using JSON.parse() and JSON.stringify().