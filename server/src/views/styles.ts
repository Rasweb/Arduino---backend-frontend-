export const getStyle = () => {
    return `
        <style>
            body {
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                margin: 0;
                padding: 20px;
                background: #ffffff;
                color: #000000;
            }
            .container {
                max-width: 1200px;
                margin: 0 auto;
                background: #ffffff;
                padding: 20px;
                border-radius: 10px;
                box-shadow: 0 2px 10px rgba(0,0,0,0.1);
                border: 1px solid #dddddd;
            }
            h1 {
                color: #000000;
                text-align: center;
                margin-bottom: 30px;
            }
            .stats {
                background: #f8f8f8;
                color: #000000;
                padding: 20px;
                border-radius: 8px;
                margin-bottom: 30px;
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
                gap: 15px;
                border: 1px solid #cccccc;
            }
            .stat-item {
                text-align: center;
            }
            .stat-number {
                font-size: 2em;
                font-weight: bold;
                display: block;
                color: #000000;
            }
            .stat-label {
                font-size: 0.9em;
                opacity: 0.8;
                color: #000000;
            }
            .test {
                border: 1px solid #cccccc;
                padding: 15px;
                margin: 10px 0;
                border-radius: 8px;
                background: #ffffff;
            }
            .test-header {
                display: flex;
                justify-content: space-between;
                align-items: center;
                margin-bottom: 10px;
                border-bottom: 1px solid #eeeeee;
                padding-bottom: 10px;
            }
            .device-id {
                background: #666666;
                color: white;
                padding: 2px 8px;
                border-radius: 12px;
                font-size: 0.8em;
            }
            .test-result {
                font-size: 1.1em;
                font-weight: bold;
                margin: 5px 0;
                color: #000000;
            }
            .pressed {
                color: #000000;
                font-weight: bold;
            }
            .not-pressed {
                color: #000000;
            }
            .controls {
                margin: 20px 0;
                text-align: center;
            }
            button {
                background: #444444;
                color: white;
                border: 1px solid #666666;
                padding: 10px 20px;
                border-radius: 5px;
                cursor: pointer;
                font-size: 1em;
                transition: background 0.3s;
                margin: 0 5px;
            }
            button:hover {
                background: #666666;
            }
            .error-message {
                color: red;
                font-size: 0.9em;
                margin-left: 8px;
            }

            /* Test Card Styles */
            .test-card {
                border: 1px solid #dddddd;
                border-radius: 8px;
                padding: 15px;
                margin: 15px 0;
                background: #ffffff;
                box-shadow: 0 2px 4px rgba(0,0,0,0.1);
                transition: box-shadow 0.3s ease;
            }
            
            .test-card:hover {
                box-shadow: 0 4px 8px rgba(0,0,0,0.15);
            }
            
            .test-card-header {
                display: flex;
                justify-content: space-between;
                align-items: flex-start;
                margin-bottom: 10px;
                padding-bottom: 10px;
                border-bottom: 1px solid #eeeeee;
            }
            
            .test-card-title {
                display: flex;
                align-items: center;
                gap: 10px;
            }
            
            .test-id {
                font-weight: bold;
                color: #000000;
                font-size: 1.1em;
            }
            
            .device-id-badge {
                background: #666666;
                color: white;
                padding: 4px 8px;
                border-radius: 12px;
                font-size: 0.8em;
                font-weight: bold;
            }
            
            .test-card-time {
                color: #666666;
                font-size: 0.9em;
            }
            
            .test-result-badge {
                display: inline-block;
                padding: 6px 12px;
                border-radius: 20px;
                font-weight: bold;
                font-size: 0.9em;
                margin-bottom: 15px;
            }
            
            /* Dynamic status classes for test badges */
            .test-status-passed,
            .test-status-pass,
            .test-status-success,
            .test-status-ok,
            .test-status-completed {
                background: #e8f5e8;
                color: #2e7d32;
                border: 1px solid #c8e6c9;
            }
            
            .test-status-failed,
            .test-status-fail,
            .test-status-error,
            .test-status-rejected {
                background: #ffebee;
                color: #c62828;
                border: 1px solid #ffcdd2;
            }
            
            .test-status-pending,
            .test-status-running,
            .test-status-in-progress {
                background: #fff3cd;
                color: #856404;
                border: 1px solid #ffeaa7;
            }
            
            /* Fallback for any other status values */
            .test-result-badge:not([class*="test-status-"]) {
                background: #f5f5f5;
                color: #666666;
                border: 1px solid #e0e0e0;
            }
            
            .test-card-body {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
                gap: 12px;
            }
            
            .test-field {
                display: flex;
                flex-direction: column;
                padding: 8px;
                background: #f9f9f9;
                border-radius: 4px;
            }
            
            .field-label {
                font-weight: bold;
                color: #333333;
                font-size: 0.85em;
                margin-bottom: 4px;
            }
            
            .field-value {
                color: #000000;
                word-break: break-word;
            }
            
            .data-field {
                font-family: 'Courier New', monospace;
                font-size: 0.9em;
                background: #f0f0f0;
                padding: 4px 6px;
                border-radius: 3px;
            }

            /* Fix for data items to display on separate lines */
            .data-item {
                display: block;
                margin-bottom: 4px;
            }
            
            /* Status field styling */
            .status-passed,
            .status-pass,
            .status-success { 
                color: #2e7d32; 
                font-weight: bold; 
            }
            
            .status-failed,
            .status-fail,
            .status-error { 
                color: #c62828; 
                font-weight: bold; 
            }
            
            .status-pending,
            .status-running,
            .status-in-progress { 
                color: #ff8f00; 
                font-weight: bold; 
            }
            
            /* Table Styles (keep for potential future use) */
            .test-table {
                width: 100%;
                border-collapse: collapse;
                margin: 20px 0;
                background: #ffffff;
            }
            
            .test-table th,
            .test-table td {
                border: 1px solid #cccccc;
                padding: 12px;
                text-align: left;
            }
            
            .test-table th {
                background: #f8f8f8;
                font-weight: bold;
                color: #000000;
                position: sticky;
                top: 0;
            }
            
            .test-table tr:nth-child(even) {
                background: #f9f9f9;
            }
            
            .test-table tr:hover {
                background: #f0f0f0;
            }
            
            .test-row {
                color: #000000;
            }
            
            /* Responsive design */
            @media (max-width: 768px) {
                .test-card-header {
                    flex-direction: column;
                    gap: 10px;
                }
                
                .test-card-body {
                    grid-template-columns: 1fr;
                }
                
                .stats {
                    grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
                }
            }
        </style>
    `;
};