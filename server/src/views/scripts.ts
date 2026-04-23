/* 
- This file is responsible for rendering test results on both the server and client sides, as well as managing
  data associated with test outcomes.

- Functions:
    - 1. getTestHeadClient
        Updates and generates HTML for individual test blocks with dynamic status detection.

    - 2. getScripts
        Compiles JavaScript functions that enhance the user interface, including functions for reloading,
        exporting data, and clearing all test data from the server.

    - 3. getFilterScripts
        Contains filtering functions that enable users to customize the visibility of filter options and
        retrieve filtered test results from the server.

    - 4. getDOMScripts
        Sets up DOM interactions, including initializing dropdown values and attaching click events for various actions.

- The distinction between "rendertesthead" and "getTestHeadClient" ensures accurate filtering and rendering on both the 
  server and client sides, facilitating seamless integration.

- Overall, this file orchestrates the rendering of test results, handles user interactions, and manages 
  associated data effectively.
*/
import { TestEntry } from "../data/types.js";

// Dynamic test status detection for consistent server/client rendering
function getDynamicTestStatus(teststatus: string | undefined): { displayText: string; cssClass: string } {
    if (!teststatus) {
        return {
            displayText: "Not Tested",
            cssClass: "test-status-not-tested"
        };
    }

    const status = teststatus.toLowerCase().trim();
    
    // Failed indicators take precedence (consistent with server logic)
    const failedIndicators = ['✗', 'fail', 'error', 'failed', 'missed'];
    if (failedIndicators.some(indicator => status.includes(indicator))) {
        return {
            displayText: "Failed",
            cssClass: "test-status-failed"
        };
    }
    
    // Passed indicators
    const passedIndicators = ['✓', 'pass', 'success', 'ok', 'passed', 'complete'];
    if (passedIndicators.some(indicator => status.includes(indicator))) {
        return {
            displayText: "Passed", 
            cssClass: "test-status-passed"
        };
    }

    // Default case - use original status
    const safeClass = status.toLowerCase().replace(/[^a-z0-9]/g, '-');
    return {
        displayText: teststatus,
        cssClass: "test-status-" + safeClass
    };
}

export function getTestHeadClient(test: TestEntry, displayTime: string) {
    // Dynamic status detection using consistent logic
    const statusResult = getDynamicTestStatus(test.teststatus);
    const testResult = statusResult.displayText;
    const statusClass = statusResult.cssClass;

    return `
        <div class="test-card">
            <div class="test-card-header">
                <div class="test-card-title">
                    <span class="test-id">Test ID: ${test.test_id}</span>
                    <span class="device-id-badge">${test.device_id}</span>
                </div>
                <div class="test-card-time">${displayTime}</div>
            </div>
            
            <div class="test-result-badge ${statusClass}">${testResult}</div>
            
            <div class="test-card-body">
                <div class="test-field">
                    <span class="field-label">Line Number:</span>
                    <span class="field-value">${test.lineNumber || 'N/A'}</span>
                </div>
                <div class="test-field">
                    <span class="field-label">Product Name:</span>
                    <span class="field-value">${test.prodname || 'N/A'}</span>
                </div>
                <div class="test-field">
                    <span class="field-label">Serial Number:</span>
                    <span class="field-value">${test.serialnr || 'N/A'}</span>
                </div>
                <div class="test-field">
                    <span class="field-label">Test Status:</span>
                    <span class="field-value status-${statusClass}">${test.teststatus || 'Not Tested'}</span>
                </div>
                <div class="test-field">
                    <span class="field-label">Data:</span>
                    <div class="data-field">
                        ${test.data ? Object.entries(test.data).map(([key, value]) => `
                            <div class="data-item">
                                <span class="data-key">${key}:</span>
                                <span class="data-value">${value}</span>
                            </div>
                        `).join('') : `<span class="field-value">No Data</span>`}
                    </div>
                </div>
                <div class="test-field">
                    <span class="field-label">Free Text:</span>
                    <span class="field-value">${test.freeText || 'No comments'}</span>
                </div>
            </div>
        </div>
    `;
}

export const getScripts = (dataObject: TestEntry[]) => {
    return `
            // Client-side version of dynamic test status detection
            function getDynamicTestStatusClient(teststatus) {
                if (!teststatus) {
                    return {
                        displayText: "Not Tested",
                        cssClass: "test-status-not-tested"
                    };
                }

                const status = teststatus.toLowerCase().trim();
                
                // Failed indicators take precedence (consistent with server logic)
                const failedIndicators = ['✗', 'fail', 'error', 'failed', 'missed'];
                if (failedIndicators.some(indicator => status.includes(indicator))) {
                    return {
                        displayText: "Failed",
                        cssClass: "test-status-failed"
                    };
                }
                
                // Passed indicators
                const passedIndicators = ['✓', 'pass', 'success', 'ok', 'passed', 'complete'];
                if (passedIndicators.some(indicator => status.includes(indicator))) {
                    return {
                        displayText: "Passed", 
                        cssClass: "test-status-passed"
                    };
                }

                // Default case - use original status
                const safeClass = status.toLowerCase().replace(/[^a-z0-9]/g, '-');
                return {
                    displayText: teststatus,
                    cssClass: "test-status-" + safeClass
                };
            }

            function loadTests() {
                location.reload();
            }

            function exportData() {
                const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(${JSON.stringify(dataObject)}, null, 2));
                const downloadAnchorNode = document.createElement('a');
                downloadAnchorNode.setAttribute("href", dataStr);
                downloadAnchorNode.setAttribute("download", "test_results_" + new Date().toISOString().split('T')[0] + ".json");
                document.body.appendChild(downloadAnchorNode);
                downloadAnchorNode.click();
                downloadAnchorNode.remove();
            }
            
            function clearAllData() {
                if (confirm('Are you sure you want to delete ALL test data? This cannot be undone!')) {
                    fetch('/manageTests/clearAllData', { method: 'POST' })
                        .then(response => response.json())
                        .then(data => {
                            if (data.success) {
                                alert('All data cleared successfully');
                                location.reload();
                            } else {
                                alert('Error clearing data');
                            }
                        })
                        .catch(error => {
                            alert('Error clearing data: ' + error);
                    });
                }
            }

            ${getFilterScripts()}
            ${getDOMScripts()}

            // Auto-refresh every 30 seconds
            setInterval(() => {
                document.getElementById('last-update').textContent = new Date().toLocaleString('sv-SE');
            }, 30000);

    `;
}

const getFilterScripts = () => {
    return `
        function filterState(){
            const filterDiv = document.getElementById("filters-container");
            const stateBtn = document.getElementById("filters-btn"); 
            const styles = window.getComputedStyle(filterDiv);
            if(styles.display === "none"){
                filterDiv.style.display = "inline";
                stateBtn.textContent = "Hide filters";
            } else {
                filterDiv.style.display = "none";
                stateBtn.textContent = "Show filters";
            }
        }

        // Helper to render test entries array
        function renderTests(tests, limit, emptyMessage = 'No tests found') {
            const container = document.getElementById("tests-container");
            container.innerHTML = "";
            const slicedTests = (tests || []).slice(-limit).reverse();

            if (slicedTests.length > 0) {
                slicedTests.forEach(test => {
                    const displayTime = new Date(test.timestamp).toLocaleString('sv-SE');
                    container.innerHTML += getTestHeadClient(test, displayTime);
                });
            } else {
                container.innerHTML = '<p style="text-align:center; color:#666;">' + emptyMessage + '</p>';
            }
        }

        function getById(limit, startIso, endIso) {
            const deviceIdInput = document.getElementById("deviceIdInput");
            const device_id = deviceIdInput ? deviceIdInput.value.trim() : '';

            if(!device_id){
                return getDate(startIso, endIso, limit);
            }

            fetch("/api/tests/" + encodeURIComponent(device_id), { method: "GET" })
                .then(response => response.json())
                .then(data => {
                    let tests = data.data || [];

                    if(startIso && endIso) {
                        const start = new Date(startIso).getTime();
                        const end = new Date(endIso).getTime();
                        tests = tests.filter(t => {
                            const ts = new Date(t.timestamp).getTime();
                            return ts >= start && ts <= end;
                        });
                    }

                    renderTests(tests, limit, 'No tests found for Device ID: ' + device_id);
                })
                .catch(error => {
                    console.error("Error fetching filtered tests:", error);
                    const container = document.getElementById("tests-container");
                    container.innerHTML = '<p style="text-align:center; color:#666;">Error fetching tests</p>';
                });
        }   
        
        function getDate(start, end, limit){
            const url = "/api/tests/filter-by-date?start_time=" + encodeURIComponent(start) + "&end_time=" + encodeURIComponent(end);
            fetch(url, { method: "GET" })
            .then(response => response.json())
            .then(data => {
                 if (!data.success) {
                        renderTests([], limit, data.message);
                        return;
                    }

                    let tests = data.data || [];
                    const deviceIdInput = document.getElementById("deviceIdInput");
                    const device_id = deviceIdInput ? deviceIdInput.value.trim() : '';
                    if (device_id) {
                        tests = tests.filter(t => t.device_id === device_id);
                    }

                    renderTests(tests, limit, 'No tests found for the given date range');
            })
            .catch(error => {
                console.error("Error fetching data:", error);
                  const container = document.getElementById("tests-container");
                  container.innerHTML = '<p style="text-align:center; color:#666;">Error fetching tests</p>';
                    console.log("Error :", error);
            })
        }
    `;
}

const getDOMScripts = () => {
    return `        
        const manageLimitonStart = () => {
            const urlParams = new URLSearchParams(window.location.search);
            if(!urlParams.has("limit")){
                urlParams.set("limit", "10");
                const newUrl = window.location.pathname + '?' + urlParams.toString();
                window.location.replace(newUrl); // full redirect with new URL
            }
            const limitSelect = document.getElementById("limit");
            if (limitSelect) {
                limitSelect.value = urlParams.get("limit") || "10";
            };
        }

        ${getTestHeadClient.toString()}

        const applyFiltersAndDates = () => {
            const urlParams = new URLSearchParams(window.location.search);
            const filterInp = document.getElementById("deviceIdInput").value;
            const startVal = document.getElementById("start").value;
            const endVal = document.getElementById("end").value;
            const limit = document.getElementById("limit").value || 10;

            // for updating the limit
            urlParams.set("limit", limit);
            if (filterInp) {
                urlParams.set("id", filterInp);
            } else {
                urlParams.delete("id");
            }

            let startIso = null;
            let endIso = null;
            if (startVal && endVal) {
                startIso = new Date(startVal).toISOString();
                endIso = new Date(endVal).toISOString();
                urlParams.set("start", startVal);
                urlParams.set("end", endVal);
                const newUrl = window.location.pathname + "?" + urlParams.toString();
                window.history.pushState({}, "", newUrl);
                console.log("Updated URL: ", newUrl);
                ${clearError("start")};
                ${clearError("end")};
            } else {
                console.log("Start", startIso);
                console.log("End", endIso);
                urlParams.delete("start");
                urlParams.delete("end");
            }

            if (filterInp && startIso && endIso) {
                getById(limit, startIso, endIso);
            } else if (filterInp) {
                getById(limit);
            } else if (startIso && endIso) {
                getDate(startIso, endIso, limit);
            } else if(startVal || endVal){
                console.log("Only one of the dates are provided!");
                if(startVal == ""){
                    ${showError("start", "Please enter a start date!")};
                } else {
                    ${showError("end", "Please enter an end date!")};
                }
            } else {
                loadTests();
            }
        }

        function addClickEvents(){
            const limitBtn = document.getElementById("applyButton");
            const refreshBtn = document.getElementById("refreshButton");
            const combinedBtn = document.getElementById("combinedButton");

            if (combinedBtn) {
                combinedBtn.addEventListener("click", (event) => {
                    event.preventDefault();
                    applyFiltersAndDates();
                });
            }

            if (limitBtn) {
                limitBtn.addEventListener("click", (event) => {
                    event.preventDefault();
                    const selectedLimit = document.getElementById("limit").value;
                    const urlParams = new URLSearchParams(window.location.search);
                    urlParams.set("limit", selectedLimit);
                    const newUrl = window.location.pathname + "?" + urlParams.toString();
                    window.history.pushState({}, "", newUrl);
                    console.log("Updated limit URL: ", newUrl);
                    loadTests();
                });
            }

            if(refreshBtn){
                refreshBtn.addEventListener("click", (event) => {
                    event.preventDefault();
                    const urlParams = new URLSearchParams(window.location.search);
                    urlParams.delete("id");
                    urlParams.delete("start");
                    urlParams.delete("end");
                    const newUrl = window.location.pathname + "?" + urlParams.toString();
                    window.history.pushState({}, "", newUrl);
                    loadTests();
                });
            }
        }

        // when its loaded
        document.addEventListener("DOMContentLoaded", () => {
            manageLimitonStart();
            addClickEvents();
        });
    `;
}

function showError(id: string, msg:string){
    return `
        console.log("${msg}");
        const inp${id} = document.getElementById("${id}");
        if(inp${id}){
            inp${id}.style.background = "#ffe6e6";
        }
        const error${id}  = document.getElementById("${id}Error");
        if(error${id} ){
            error${id}.textContent  = "${msg}";
        };
    `;
}

function clearError(id: string){
    return `
        const inp${id} = document.getElementById("${id}");
        if(inp${id}){
            inp${id}.style.background = "#ffffff";
        }
        const error${id}  = document.getElementById("${id}Error");
        if(error${id} ){
            error${id}.textContent = "";
        };
    `;
}