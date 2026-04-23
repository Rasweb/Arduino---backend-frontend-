/**
 * @file sample_arduino_secrets.h
 * @brief Configuration template for WiFi and server settings
 * @warning RENAME this file to 'arduino_secrets.h' and fill in your credentials
 * @warning This file contains sensitive information - ensure it's added to .gitignore
 * 
 * Template for storing WiFi credentials, device identification and server configuration
 * Used for local development with HTTP server
 */

#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H

// WiFi credentials for network connection
#define SECRET_SSID ""              // Your WiFi network name (SSID)
#define SECRET_PASS ""              // Your WiFi password

// Network configuration  
#define SECRET_IP ""                // Static IP address (leave empty for DHCP)

// Device identification
#define DEVICE_ID ""                // possible to change the device-id here
//#define DEVICE_ID "" //possible to extend test-units; to add more device-id

// Server configuration
#define SECRET_SERVER_ADDRESS ""    // Full server address with port (e.g., "http://10.0.21.216:3000")

#endif

/**
 * @example Usage in test system:
 * 
 * // Connect to WiFi (in test_states.cpp)
 * WiFi.begin(SECRET_SSID, SECRET_PASS);
 * 
 * // Use device ID in test data (in test_data_processor.cpp)
 * char test_id[64];
 * snprintf(test_id, sizeof(test_id), "%s_%lu", DEVICE_ID, currentTime);
 * 
 * // Server communication (in test_wifi.cpp and test_csv.cpp)
 * WiFiClient client;
 * client.connect(SECRET_IP, 3000); // For individual test sending
 * // OR use full server address for CSV uploads
 * 
 * // Set static IP if configured (in test_system_core.cpp)
 * if (strlen(SECRET_IP) > 0) {
 *   IPAddress localIP;
 *   localIP.fromString(SECRET_IP);
 *   WiFi.config(localIP);
 * }
 */
