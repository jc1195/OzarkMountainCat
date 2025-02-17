/**
 * @file DataHandler.js
 * @brief Handles incoming BLE data for the pet tracker web application.
 *
 * This file provides functions to parse received BLE data in JSON format,
 * update the UI with tracker information (such as coordinates, time, battery levels,
 * and signal strength), and process various events related to the pet tracker.
 */

// Expose handleDataReceived as a global function
window.handleDataReceived = handleDataReceived;

/**
 * @function handleDataReceived
 * @description Parses the JSON data received from the BLE device and updates UI elements.
 * 
 * The function:
 * 1. Converts the received DataView to a string.
 * 2. Removes non-printable control characters.
 * 3. Trims the string.
 * 4. Attempts to parse the cleaned string as JSON.
 * 5. Depending on the msgType, extracts relevant fields and updates corresponding UI elements.
 *
 * @param {Event} event - The BLE characteristic value changed event.
 */
function handleDataReceived(event) {
    // Convert BLE DataView to a UTF-8 string.
    let decoder = new TextDecoder('utf-8');
    let dataString = decoder.decode(event.target.value);

    // 1) Remove null bytes or other non-printable control characters except whitespace.
    //    This regex removes ASCII control characters (except \t, \n, \r).
    dataString = dataString.replace(/[\x00-\x08\x0B\x0C\x0E-\x1F]+/g, "");

    // 2) Trim leading and trailing whitespace.
    dataString = dataString.trim();
    console.log("Raw JSON from BLE:", dataString);

    // Attempt to parse the string as JSON.
    let dataObj;
    try {
        dataObj = JSON.parse(dataString);
    } catch (error) {
        console.error("Failed to parse JSON:", error);
        return;
    }

    // Extract the message type (msgType) from the parsed object. Default to 0.
    let msgType = dataObj.msgType || 0; // MSG_ALL_DATA = 0, MSG_ACKNOWLEDGEMENT = 1, MSG_BUZZER = 2, MSG_LED = 3, MSG_RB_LED = 4, MSG_PWR_MODE = 5

    // Process based on the msgType:
    if (msgType == 5) {
        console.log("Received MSG_PWR_MODE");
        let mode = dataObj.mode || 10; // MODE_LIVE_TRACKING = 0, MODE_POWER_SAVING = 1, MODE_EXTREME_POWER_SAVING = 2, MODE_NO_TRACKING = 10
        return; // No UI update needed for power mode.
    }
    if (msgType == 4) {
        console.log("Received MSG_RB_LED");
        let rbLed = dataObj.rbLed || false;
        return; // No UI update needed for rainbow LED.
    }
    if (msgType == 3) {
        console.log("Received MSG_LED");
        let r = dataObj.r || 0;      
        let g = dataObj.g || 0;
        let b = dataObj.b || 0;
        return; // No UI update needed for LED.
    }
    if (msgType == 2) {
        console.log("Received MSG_BUZZER");
        let buzzer = dataObj.buzzer || false;
        return; // No UI update needed for buzzer.
    }
    if (msgType == 1) {
        console.log("Received MSG_ACKNOWLEDGEMENT");
        let ack = dataObj.ack || false; // Acknowledgement flag.
        return; // No UI update needed for acknowledgement.
    }
    if (msgType == 0) {
        console.log("Received MSG_ALL_DATA");
        let lat = dataObj.lat || 0.0;
        let lon = dataObj.lon || 0.0;
        let hour = dataObj.hour || 0;
        let minute = dataObj.min || 0;
        let second = dataObj.sec || 0;
        let siv = dataObj.siv || 0;    // Satellites in view.
        let hdop = dataObj.hdop || 0.0;
        let rBatt = dataObj.rBatt || 0.0; // Receiver battery.
        let hBatt = dataObj.hBatt || 0.0; // Harness battery.
        // Log coordinates and update UI.
        console.log(`Latitude: ${lat}, Longitude: ${lon}`);
        document.getElementById('cordValue').textContent = `Coordinates: ${lat}, ${lon}`;
        // Update the tracker marker on the map.
        updateTrackerLocation(lat, lon);
        // Convert UTC time to local time and update UI.
        let localTime = convertUtcToLocalTime(hour, minute, second);
        document.getElementById('timeValue').textContent = localTime;
        // Update battery levels.
        updateBatteryLevel(hBatt, 1); // 1 indicates harness battery.
        updateBatteryLevel(rBatt, 2); // 2 indicates receiver battery.
        // Update satellite icon and HDOP value.
        updateSatIcon(siv);
        document.getElementById('hdopValue').textContent = `${hdop} HDOP`;
    }
    
    // These values are always sent by the Receiver.
    let rbLed = dataObj.rbLed;
    let mode = dataObj.mode || 10;
    let snr = dataObj.snr || 0;
    let rssi = dataObj.rssi || -999;
    let r = dataObj.r || 0;      
    let g = dataObj.g || 0;
    let b = dataObj.b || 0;
    
    // Update the light icon on the UI with color information.
    updateLightIcon(r, g, b, rbLed);
    // Update the power mode display.
    document.getElementById('powerModeValue').textContent = parsePowerMode(mode);
    // Update signal bars based on the RSSI value.
    updateSignalBars(rssi);
    // Log additional information (e.g., altitude, SNR, buzzer status, acknowledgement) if available.
    console.log(`Altitude: ${alt} m, SNR: ${snr}, Buzzer: ${buzzer}, ack: ${ack}`);
}

/**
 * @function parsePowerMode
 * @description Parses the numeric power mode into a human-readable string.
 * 
 * @param {number} mode - The numeric power mode.
 * @returns {string} The corresponding power mode description.
 */
function parsePowerMode(mode) {
    switch (mode) {
        case 10: return 'Error: No Tracking Mode';
        case 2: return 'Extreme Power Saving Mode';
        case 1: return 'Power Saving Mode';
        case 0: return 'Live Tracking';
        default: return 'Error: Unknown';
    }
}

/**
 * @function updateBatteryLevel
 * @description Updates the battery level UI element based on the battery reading.
 * 
 * Adjusts the height and background color of battery level indicators, and updates percentage text.
 *
 * @param {number|string} level - The battery level as a percentage.
 * @param {number} battFlag - Indicator of which battery (1 for harness, 2 for receiver).
 */
function updateBatteryLevel(level, battFlag) {
    const hBatteryLevel = document.querySelector(".hBattery-level");
    const rBatteryLevel = document.querySelector(".rBattery-level");
    const hBatteryPercentage = document.getElementById("hBatteryPercentage");
    const rBatteryPercentage = document.getElementById("rBatteryPercentage");
    console.log(`Update Battery Level: ${level}`);

    if (battFlag == 1) {
        if (level == '0') {
            hBatteryLevel.style.height = 1 + "%";
            console.log(level);
        } else {
            console.log(level);
            hBatteryLevel.style.height = level + "%";
        }
        // Update color based on battery level.
        if (level < 10) {
            hBatteryLevel.style.backgroundColor = "red";
        } else if (level < 30) {
            hBatteryLevel.style.backgroundColor = "orange";
        } else {
            hBatteryLevel.style.backgroundColor = "#32dd37"; // Default color for charged level.
        }
        hBatteryPercentage.textContent = level + "%";
    } else if (battFlag == 2) {
        if (level == '0') {
            rBatteryLevel.style.height = 1 + "%";
            console.log(level);
        } else {
            console.log(level);
            rBatteryLevel.style.height = level + "%";
        }
        // Update color based on battery level.
        if (level < 10) {
            rBatteryLevel.style.backgroundColor = "red";
        } else if (level < 30) {
            rBatteryLevel.style.backgroundColor = "orange";
        } else {
            rBatteryLevel.style.backgroundColor = "#32dd37"; // Default color for charged level.
        }
        rBatteryPercentage.textContent = level + "%";
    }
}

/**
 * @function updateLightIcon
 * @description Updates the light icon on the UI based on the LED color values and rainbow LED flag.
 * 
 * Chooses an image source for the light icon depending on the provided color values and whether
 * the rainbow LED is activated.
 *
 * @param {number} r - Red channel value.
 * @param {number} g - Green channel value.
 * @param {number} b - Blue channel value.
 * @param {boolean} rbLed - Flag indicating if the rainbow LED is on.
 */
function updateLightIcon(r, g, b, rbLed) {
    const lightIcon = document.getElementById("lightIcon");

    const colorMap = {
        'M': 'Rainbow',
        'L': 'Red',
        'K': 'Pink',
        'J': 'Purple',
        'I': 'Orange',
        'H': 'BrightOrange',
        'G': 'Yellow',
        'F': 'Bright Green',
        'E': 'Green',
        'D': 'Blue',
        'C': 'Cyan',
        'B': 'White',
        'A': 'Off', // Assuming 'A' means off.
    };

    if (r != 0) {
        lightIcon.src = "LBRed.png";
    } else if (b != 0) {
        lightIcon.src = "LBRed.png";
    } else if (g != 0) {
        lightIcon.src = "LBRed.png";
    } else if (rbLed == true) {  // Rainbow LED active.
        lightIcon.src = "LBGreen.png";
    } else {
        lightIcon.src = "LBOff.png";
    }
}

/**
 * @function updateSatIcon
 * @description Updates the satellite icon on the UI based on the number of satellites in view.
 *
 * @param {number} satInView - The number of satellites in view (SIV).
 */
function updateSatIcon(satInView) {
    const signalIcon = document.getElementById("satIcon");

    // Set icon based on satellite count.
    if (satInView < 1) {
        signalIcon.src = "SatOff.png";
    } else {
        signalIcon.src = "SatOn.png";
    }
    document.getElementById("satValue").textContent = satInView + " SIV";
}

/**
 * @function updateSignalBars
 * @description Updates the signal bars icon and text on the UI based on the RSSI value.
 *
 * @param {number} rssi - The Received Signal Strength Indicator in dBm.
 */
function updateSignalBars(rssi) {
    const signalIcon = document.getElementById("signalIcon");

    // Determine which signal icon to display based on the RSSI value.
    if (rssi >= -90) {
        signalIcon.src = "4Bars.png";
    } else if (rssi >= -110) {
        signalIcon.src = "3Bars.png";
    } else if (rssi >= -120) {
        signalIcon.src = "2Bars.png";
    } else if (rssi >= -130) {
        signalIcon.src = "1Bars.png";
    } else {
        signalIcon.src = "0Bars.png";
    }

    document.getElementById("rssiValue").textContent = rssi + " dBm";
}

/**
 * @function convertUtcToLocalTime
 * @description Converts a given UTC time to the local time and formats it.
 *
 * @param {number} utcHour - The hour component in UTC.
 * @param {number} utcMin - The minute component in UTC.
 * @param {number} utcSec - The second component in UTC.
 * @returns {string} The formatted local time string.
 */
function convertUtcToLocalTime(utcHour, utcMin, utcSec) {
    // Create a Date object using current UTC date with provided UTC time.
    const now = new Date();
    const utcDate = new Date(Date.UTC(now.getUTCFullYear(), now.getUTCMonth(), now.getUTCDate(), utcHour, utcMin, utcSec));

    const options = {
        hour: 'numeric',
        minute: 'numeric',
        second: 'numeric',
        timeZoneName: 'short'
    };

    const formatter = new Intl.DateTimeFormat('en-US', options);
    return formatter.format(utcDate);
}

/**
 * @function updateTrackerLocation
 * @description Updates the map marker for the tracker location.
 *
 * If the tracker marker does not exist, it creates one and adds it to the map.
 * Otherwise, it updates the existing marker's position.
 *
 * @param {number} lat - The latitude value.
 * @param {number} lng - The longitude value.
 */
function updateTrackerLocation(lat, lng) {
    if (!trackerMarker) {
        trackerMarker = new mapboxgl.Marker({ "color": "#FF8C00" }) // Orange for visibility.
            .setLngLat([lng, lat])
            .addTo(map);
    } else {
        trackerMarker.setLngLat([lng, lat]);
    }
}
