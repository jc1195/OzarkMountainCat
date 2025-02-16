// This function will set all the label values in the webpage
window.handleDataReceived = handleDataReceived;

// function handleDataReceived(event) {
//     let value = event.target.value; // Value is a DataView
//     let decoder = new TextDecoder('utf-8');
//     let dataString = decoder.decode(value);

//     // Parse the space-delimited string
//     let dataParts = dataString.trim().split(' ');
//     let latitude = dataParts[1];
//     let longitude = dataParts[2];

//     console.log(`Latitude: ${latitude}, Longitude: ${longitude}`);
//     document.getElementById('cordValue').textContent =  `Coordinates: ${latitude}, ${longitude}`;

//     updateTrackerLocation(latitude, longitude);

//     let utcHour = parseInt(dataParts[9]);
//     let utcMin = parseInt(dataParts[10]);
//     let utcSec = parseInt(dataParts[11]);
//     let localTime = convertUtcToLocalTime(utcHour, utcMin, utcSec);

//     document.getElementById('timeValue').textContent = `${localTime}`;
//     console.log(`Battery Level: ${dataParts[3]}`)
//     // Harness Batt (1)
//     updateBatteryLevel(`${dataParts[3]}`, 1);
//     // Receiver Batt (2)
//     updateBatteryLevel(`${dataParts[12]}`, 2);
//     updateSatIcon(dataParts[4]);
//     document.getElementById('hdopValue').textContent = `${dataParts[5]} HDOP`;
//     document.getElementById('powerModeValue').textContent = `${parsePowerMode(dataParts[6])}`;
//     updateLightIcon(dataParts[8]);
//     updateSignalBars(dataParts[13]);

// }

function handleDataReceived(event) {
    // Convert BLE DataView to string
    let decoder = new TextDecoder('utf-8');
    let dataString = decoder.decode(event.target.value);

    
        // 1) Remove null bytes or other non-printable control chars except whitespace
    //    [\x00-\x08\x0B\x0C\x0E-\x1F] covers ASCII control chars besides \t,\n,\r
    dataString = dataString.replace(/[\x00-\x08\x0B\x0C\x0E-\x1F]+/g, "");

    // 2) Trim leading/trailing whitespace
    dataString = dataString.trim();
    console.log("Raw JSON from BLE:", dataString);

    // Try to parse JSON
    let dataObj;
    try {
        dataObj = JSON.parse(dataString);
    } catch (error) {
        console.error("Failed to parse JSON:", error);
        return;
    }

    // Extract fields (with defaults if missing)
    let msgType = dataObj.msgType || 0; //MSG_ALL_DATA = 0, | MSG_ACKNOWLEDGEMENT = 1, | MSG_BUZZER = 2, | MSG_LED = 3, | MSG_RB_LED = 4 | MSG_PWR_MODE = 5,
    
    if (msgType == 5) {
        console.log("Received MSG_PWR_MODE");
        let mode = dataObj.mode || 10; // MODE_LIVE_TRACKING = 0 | MODE_POWER_SAVING = 1 | MODE_EXTREME_POWER_SAVING = 2 | MODE_NO_TRACKING = 10
        return; // No need to update UI for mode
    }
    if (msgType == 4) {
        console.log("Received MSG_RB_LED");
        let rbLed = dataObj.rbLed || false;
        return; // No need to update UI for rbLed
    }
    if (msgType == 3) {
        console.log("Received MSG_LED");
        let r = dataObj.r || 0;      
        let g = dataObj.g || 0;
        let b = dataObj.b || 0;
        return; // No need to update UI for led
    }
    if (msgType == 2) {
        console.log("Received MSG_BUZZER");
        let buzzer = dataObj.buzzer || false;
        return; // No need to update UI for buzzer
    }
    if (msgType == 1) {
        console.log("Received MSG_ACKNOWLEDGEMENT");
        let ack = dataObj.ack || false; // acknowledgement flag for APP
        return; // No need to update UI for ack
    }
    if (msgType == 0) {
        console.log("Received MSG_ALL_DATA");
        let lat = dataObj.lat || 0.0;
        let lon = dataObj.lon || 0.0;
        let hour = dataObj.hour || 0;
        let minute = dataObj.min || 0;
        let second = dataObj.sec || 0;
        let siv = dataObj.siv || 0;    // satellites in view
        let hdop = dataObj.hdop || 0.0;
        let rBatt = dataObj.rBatt || 0.0; // Receiver battery
        let hBatt = dataObj.hBatt || 0.0; // Harness battery
            // Display lat/lon
        console.log(`Latitude: ${lat}, Longitude: ${lon}`);
        document.getElementById('cordValue').textContent = `Coordinates: ${lat}, ${lon}`;
        // Update the map marker
        updateTrackerLocation(lat, lon);
        // Convert UTC time to local
        let localTime = convertUtcToLocalTime(hour, minute, second);
        document.getElementById('timeValue').textContent = localTime;
        // For now, placeholders for harness + receiver battery (not in JSON yet)
        updateBatteryLevel(hBatt, 1); // 1 = harness battery
        updateBatteryLevel(rBatt, 2); // 2 = receiver battery
         // SIV / HDOP
        updateSatIcon(siv);
        document.getElementById('hdopValue').textContent = `${hdop} HDOP`;
    }
    
    // The Receiver is always going to send the below data values. 
    let rbLed = dataObj.rbLed
    let mode = dataObj.mode || 10;
    let snr = dataObj.snr || 0;
    let rssi = dataObj.rssi || -999;
    let r = dataObj.r || 0;      
    let g = dataObj.g || 0;
    let b = dataObj.b || 0;
    
    // If you're using single char codes for color, pass that to updateLightIcon:
    updateLightIcon(r,g,b,rbLed); 
    // MODE_LIVE_TRACKING = 0 | MODE_POWER_SAVING = 1 | MODE_EXTREME_POWER_SAVING = 2 | MODE_NO_TRACKING = 10
    document.getElementById('powerModeValue').textContent = parsePowerMode(mode);
    // RSSI
    updateSignalBars(rssi);
    // We haven't displayed alt, snr, ack, or buzzer on your UI yet. You can console.log them:
    console.log(`Altitude: ${alt} m, SNR: ${snr}, Buzzer: ${buzzer}, ack: ${ack}`);
}

function parsePowerMode(mode) {
    switch (mode) {
        case 10: return 'Error: No Tracking Mode'
        case 2: return 'Extreme Power Saving Mode';
        case 1: return 'Power Saving Mode';
        case 0: return 'Live Tracking';
        default: return 'Error: Unknown';
    }
}

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
        // Update color based on battery level
        if (level < 10) {
            hBatteryLevel.style.backgroundColor = "red";
        } else if (level < 30) {
            hBatteryLevel.style.backgroundColor = "orange";
        } else {
            hBatteryLevel.style.backgroundColor = "#32dd37"; // Default color for charged level
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
        // Update color based on battery level
        if (level < 10) {
            rBatteryLevel.style.backgroundColor = "red";
        } else if (level < 30) {
            rBatteryLevel.style.backgroundColor = "orange";
        } else {
            rBatteryLevel.style.backgroundColor = "#32dd37"; // Default color for charged level
        }
        rBatteryPercentage.textContent = level + "%";
    }
}

function updateLightIcon(r,g,b,rbLed) {
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
        'A': 'Off', // Assuming 'A' means off for the color as well
        // Add any other color codes if necessary
    };

    if (r != 0) {
        lightIcon.src = "LBRed.png";
    } else if (b != 0) {
        lightIcon.src = "LBRed.png";
    } else if (g != 0) {
        lightIcon.src = "LBRed.png";
    // } else if (color == 'I') {
    //     lightIcon.src = "LBRed.png";
    } else if (rbLed == true) {  // rainbow led
        lightIcon.src = "LBGreen.png"
    // } else if (color == 'D') {
    //     lightIcon.src = "LBBlue.png"
    // } else if (color == 'C') {
    //     lightIcon.src = "LBCyan.png"
    } else {
        lightIcon.src = "LBOff.png"
    }

    //document.getElementById("lightValue").textContent = colorMap[color];
    // return colorMap[color] || color;//'Unknown';
}

function updateSatIcon(satInView) {
    const signalIcon = document.getElementById("satIcon");

    // Choose the appropriate image based on the RSSI value
    if (satInView < 1) {
        signalIcon.src = "SatOff.png";
    } else {
        signalIcon.src = "SatOn.png";
    }
    // Update RSSI value text
    document.getElementById("satValue").textContent = satInView + " SIV";
}

function updateSignalBars(rssi) {
        const signalIcon = document.getElementById("signalIcon");

        // Choose the appropriate image based on the RSSI value
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

        // Update RSSI value text
        document.getElementById("rssiValue").textContent = rssi + " dBm";
}

// This will convert UTC to local time on webpage
function convertUtcToLocalTime(utcHour, utcMin, utcSec) {
    // Create a Date object with UTC time
    const now = new Date();
    const utcDate = new Date(Date.UTC(now.getUTCFullYear(), now.getUTCMonth(), now.getUTCDate(), utcHour, utcMin, utcSec));

    // Options for the DateTimeFormat
    const options = {
        hour: 'numeric',
        minute: 'numeric',
        second: 'numeric',
        timeZoneName: 'short'
    };

    // Format the date and time in the local timezone
    const formatter = new Intl.DateTimeFormat('en-US', options);
    return formatter.format(utcDate);
}

function updateTrackerLocation(lat, lng) {
    // If the marker doesn't exist yet, create and add it to the map
    if (!trackerMarker) {
        trackerMarker = new mapboxgl.Marker({ "color": "#FF8C00" }) // Orange color for visibility
            .setLngLat([lng, lat])
            .addTo(map);
    } else {
        // If the marker already exists, just update its position
        trackerMarker.setLngLat([lng, lat]);
    }

    // Optionally, center the map on the new position
    // map.setCenter([lng, lat]);
    // map.setZoom(15);
}