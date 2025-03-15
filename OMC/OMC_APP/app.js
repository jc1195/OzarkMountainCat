/**
 * @file app.js
 * @brief JavaScript code for the OMC_APP web application for the pet tracker.
 *
 * This script handles map initialization using Mapbox GL, BLE connectivity, geolocation,
 * offline map tile downloading using IndexedDB, and various UI interactions.
 */

// Global variables
/** @global {mapboxgl.Map} map - Global variable to hold the Mapbox map instance */
var map;
window.map = map;

/** @global {mapboxgl.Marker} trackerMarker - Global variable to hold the tracker's map marker */
var trackerMarker;
/** @global {mapboxgl.Marker} userLocationMarker - Global variable to hold the user's location marker */
var userLocationMarker;
/** @global {BluetoothDevice|null} bleDevice - Global variable to hold the connected BLE device */
var bleDevice = null;
/** @global {BluetoothRemoteGATTCharacteristic} commandCharacteristic - Global BLE characteristic used for sending commands */
let commandCharacteristic;
/** @global {MapboxDraw} draw - Global variable to hold the Mapbox GL Draw instance */
var draw;
/** @global {Array} urls - Global array to hold tile URLs */
var urls = [];

// Mapbox configuration
/** @global {string} pMapBox_Token - Your Mapbox access token */
var pMapBox_Token = 'pk.eyJ1IjoiamMxMTk1IiwiYSI6ImNsaW54c2tuZjAzaDIzcm53YjQ2MHFkNTYifQ.Gvtk1A-oWSIUPeGLi1Pflg';
/** @global {string} pMapBox_URL - URL for the Mapbox terrain data */
var pMapBox_URL = 'mapbox://mapbox.mapbox-terrain-dem-v1';

// BLE configuration
/** @global {string} pBLE_PrimaryGUID - Primary BLE service UUID */
var pBLE_PrimaryGUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
/** @global {string} pBLE_CharacteristicGUID - BLE characteristic UUID */
var pBLE_CharacteristicGUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
// Alternate definitions (commented out)
// var pBLE_PrimaryGUID = '0x1234';
// var pBLE_CharacteristicGUID = '0x4231';

/** @global {number} pDBVersionNumber - Database version number for IndexedDB */
var pDBVersionNumber = 3;

/**
 * @description Called when the DOM content is loaded. Initializes the map, geolocation, BLE, and event listeners.
 */
document.addEventListener('DOMContentLoaded', (event) => {
    console.log("PWA Pet Tracker is ready!");
    updateBatteryLevel(0, 1);
    updateBatteryLevel(0, 2);

    // Initialize the map using Mapbox GL
    mapboxgl.accessToken = pMapBox_Token; // Set the Mapbox access token
    map = new mapboxgl.Map({
        container: 'map', // ID of the container element for the map
        style: 'mapbox://styles/mapbox/outdoors-v11', // Map style URL
        center: [-74.5, 40], // Starting position [lng, lat]
        zoom: 9 // Starting zoom level
    });

    // When the map loads, initialize Mapbox GL Draw and add additional layers
    map.on('load', function () {
        draw = new MapboxDraw({
            displayControlsDefault: false,
            controls: {
                polygon: true,
                trash: true
            }
        });
        map.addControl(draw);

        // Set up event listeners for draw events to update the area
        map.on('draw.create', updateArea);
        map.on('draw.delete', updateArea);
        map.on('draw.update', updateArea);

        // Add a raster DEM source and set terrain for 3D effects
        map.addSource('mapbox-dem', {
            'type': 'raster-dem',
            'url': pMapBox_URL,
            'tileSize': 512,
            'maxzoom': 14
        });
        map.setTerrain({ 'source': 'mapbox-dem', 'exaggeration': 1.5 });

        // Add a sky layer to simulate atmosphere
        map.addLayer({
            'id': 'sky',
            'type': 'sky',
            'paint': {
                'sky-type': 'atmosphere',
                'sky-atmosphere-sun': [0.0, 90.0],
                'sky-atmosphere-sun-intensity': 15
            }
        });
    });

    // Add navigation controls to the map
    map.addControl(new mapboxgl.NavigationControl());

    // Create and style a div element to display the zoom level
    const zoomDisplay = document.createElement('div');
    zoomDisplay.setAttribute('id', 'zoom-level');
    zoomDisplay.style.background = 'white';
    zoomDisplay.style.padding = '5px';
    zoomDisplay.style.margin = '10px';
    zoomDisplay.style.borderRadius = '3px';
    zoomDisplay.textContent = `Zoom Level: ${map.getZoom().toFixed(2)}`;
    document.body.appendChild(zoomDisplay);

    // Update the zoom level display when the map zooms
    map.on('zoom', () => {
        zoomDisplay.textContent = `Zoom Level: ${map.getZoom().toFixed(2)}`;
    });

    // Initialize geolocation to track the user's location on the map
    if ('geolocation' in navigator) {
        userLocationMarker = new mapboxgl.Marker();
        var initialPositionSet = false;

        const locationWatchID = navigator.geolocation.watchPosition(position => {
            var lat = position.coords.latitude;
            var lng = position.coords.longitude;
            var alt = position.coords.altitude;
            if (alt == null) {
                alt = 0.0;
            }
            alt = (alt * 3.28084).toFixed(0);
            document.getElementById('uAltValue').textContent = `User Altitude ${alt}ft`;

            userLocationMarker.setLngLat([lng, lat]).addTo(map);

            if (!initialPositionSet) {
                map.setCenter([lng, lat]);
                map.setZoom(15);
                initialPositionSet = true;
            }
        }, error => {
            console.error("Error getting location: ", error);
        }, {
            enableHighAccuracy: true
        });

        // Clear the geolocation watch when the page is unloaded
        window.onbeforeunload = function () {
            navigator.geolocation.clearWatch(locationWatchID);
        };
    } else {
        console.log('Geolocation is not supported by this browser.');
    }

    // BLE connection setup
    let isConnected = false;
    const connectButton = document.getElementById('connectButton');
    connectButton.addEventListener('click', function () {
        console.log('Connect button clicked');

        navigator.bluetooth.requestDevice({
            filters: [{ name: 'Mount' }],
            optionalServices: [pBLE_PrimaryGUID, pBLE_CharacteristicGUID] // Use both service UUIDs
        })
            .then(device => {
                bleDevice = device;
                console.log('Connecting to device...');
                device.addEventListener('gattserverdisconnected', onDisconnected);
                return device.gatt.connect();
            })
            .then(server => {
                console.log('Getting primary GATT Service...');
                return server.getPrimaryService(pBLE_PrimaryGUID);
            })
            .then(secondService => {
                console.log('Getting second GATT Characteristic...');
                return secondService.getCharacteristic(pBLE_CharacteristicGUID);
            })
            .then(characteristic => {
                console.log('Starting notifications...');
                commandCharacteristic = characteristic;
                return characteristic.startNotifications();
            })
            .then(characteristic => {
                console.log('Notifications started.');
                characteristic.addEventListener('characteristicvaluechanged', handleDataReceived);
                isConnected = true;
                updateConnectionStatus(isConnected);
            })
            .catch(error => {
                console.error('Connection failed', error);
                isConnected = false;
                updateConnectionStatus(isConnected);
            });
    });

    // BLE disconnect button event listener
    const disconnectButton = document.getElementById('disconnectButton');
    disconnectButton.addEventListener('click', function () {
        if (bleDevice && bleDevice.gatt.connected) {
            bleDevice.gatt.disconnect();
            console.log('Device disconnected');
        }
    });

    /**
     * @description Called when the BLE device disconnects.
     */
    function onDisconnected() {
        console.log('Device disconnected');
        isConnected = false;
        updateConnectionStatus(isConnected);
    }

    /**
     * @description Updates the connection status indicator on the UI.
     * @param {boolean} connected - True if connected; false otherwise.
     */
    function updateConnectionStatus(connected) {
        const statusIndicator = document.getElementById('connectionStatus');
        if (connected) {
            statusIndicator.classList.remove('red');
            statusIndicator.classList.add('green');
            //statusIndicator.textContent = 'Connected';
        } else {
            statusIndicator.classList.remove('green');
            statusIndicator.classList.add('red');
            //statusIndicator.textContent = 'Disconnected';
        }
    }
});

// /**
//  * @function setLightColor
//  * @description Reads the selected light color from the UI and sends a command to the BLE device.
//  */
// function setLightColor() {
//     var selectedColor = document.getElementById('lightColorSelect').value;
//     console.log('Selected Light Color:', selectedColor);

//     if (bleDevice && bleDevice.gatt.connected) {
//         sendCommandToBleDevice(selectedColor);
//     } else {
//         console.error('Bluetooth device is not connected');
//     }
// }
function rbLed() {
    var command = document.getElementById('rbLedSelect').value;
    if (command == 1) {
        if (bleDevice && bleDevice.gatt.connected) {
            let command = {
                msgType: 4,
                rbLed: true
            };
            sendJsonCommandToBleDevice(command);
        }
    } else {
        if (bleDevice && bleDevice.gatt.connected) {
            let command = {
                msgType: 4,
                rbLed: false
            };
            sendJsonCommandToBleDevice(command);
        }
    }
}
/**
 * @function setLightColor
 * @description Reads the selected color from the dropdown, maps it to specific RGB values,
 * builds a JSON command with msgType 3 (LED command), and sends the command via BLE.
 */
function setLightColor() {
    // Get the selected color code from the dropdown
    var selectedCode = document.getElementById('lightColorSelect').value;
    console.log('Selected Light Color:', selectedCode);

    // Mapping from dropdown color codes to RGB values.
    // Modify these values as needed to match your desired colors.
    const colorMap = {
        1: { r: 255, g: 0, b: 255 }, // Rainbow (placeholder value)
        2: { r: 255, g: 0, b: 0 }, // Red
        3: { r: 128, g: 50, b: 128 }, // Pink
        4: { r: 128, g: 0, b: 128 }, // Purple
        5: { r: 255, g: 165, b: 0 }, // Orange
        6: { r: 255, g: 140, b: 0 }, // Bright Orange
        7: { r: 255, g: 255, b: 0 }, // Yellow
        8: { r: 0, g: 255, b: 0 }, // Bright Green
        9: { r: 0, g: 128, b: 0 }, // Green
        10: { r: 0, g: 0, b: 255 }, // Blue
        11: { r: 0, g: 255, b: 255 }, // Cyan
        12: { r: 255, g: 255, b: 255 }, // White
        13: { r: 0, g: 0, b: 0 }  // Off
    };

    // If the BLE device is connected, send the new JSON command
    if (bleDevice && bleDevice.gatt.connected) {
        // Retrieve the RGB values from the mapping. If the code isn't found, default to "Off" (black).
        let color = colorMap[selectedCode] || { r: 0, g: 0, b: 0 };

        // Build the JSON command with msgType 3 (for LED command) and the RGB values.
        let command = {
            msgType: 3,  // MSG_LED as per your specification.
            r: color.r,
            g: color.g,
            b: color.b
        };

        // Send the JSON command using your helper function.
        sendJsonCommandToBleDevice(command);
    } else {
        console.error('Bluetooth device is not connected');
    }
}

/**
 * @function toggleSidebar
 * @description Toggles the visibility of the sidebar UI element.
 */
function toggleSidebar() {
    var sidebar = document.getElementById('sidebar');
    if (sidebar.style.left === '-250px') {
        sidebar.style.left = '0';
    } else {
        sidebar.style.left = '-250px';
    }
}

/**
 * @event Document#click
 * @description Hides the sidebar when a click occurs outside of it.
 */
document.addEventListener('click', function (event) {
    var sidebar = document.getElementById('sidebar');
    var hamburger = document.getElementById('hamburger');

    // If the click is outside the sidebar and hamburger, hide the sidebar.
    if (!sidebar.contains(event.target) && !hamburger.contains(event.target)) {
        sidebar.style.left = '-250px';
    }
});

/**
 * @function turnOffLight
 * @description Sends a JSON command via BLE to turn off the light.
 */
function turnOffLight() {
    if (bleDevice && bleDevice.gatt.connected) {
        let command = {
            msgType: 0,
            r: 0,
            g: 0,
            b: 0
        };
        sendJsonCommandToBleDevice(command);
    }
}

/**
 * @function musicActions
 * @description Sends a JSON command via BLE to activate the buzzer.
 */
function musicActions() {
    var selectedCode = document.getElementById('musicSelect').value;

    if (selectedCode == 1) {
        if (bleDevice && bleDevice.gatt.connected) {
            let command = {
                msgType: 2,
                buzzer: true,
            };
            sendJsonCommandToBleDevice(command);
        }
    } else {
        if (bleDevice && bleDevice.gatt.connected) {
            let command = {
                msgType: 2,
                buzzer: false,
            };
            sendJsonCommandToBleDevice(command);
        }
    }
}


/**
 * @function turnOffBuzzer
 * @description Sends a JSON command via BLE to deactivate the buzzer.
 */
function turnOffBuzzer() {
    if (bleDevice && bleDevice.gatt.connected) {
        let command = {
            msgType: 2,
            buzzer: false,
        };
        sendJsonCommandToBleDevice(command);
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
        1: 'Rainbow',
        2: 'Red',
        3: 'Pink',
        4: 'Purple',
        5: 'Orange',
        6: 'BrightOrange',
        7: 'Yellow',
        8: 'Bright Green',
        9: 'Green',
        10: 'Blue',
        11: 'Cyan',
        12: 'White',
        13: 'Off', // Assuming 'A' means off.
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
 * @function setPwrMode
 * @description Sends a JSON command via BLE to set the power mode.
 */
function setPwrMode() {
    var selectedMode = document.getElementById('powerModeSelect').value;
    console.log('Selected Power Mode:', selectedMode);

    let command = {
        msgType: 5,
        mode: selectedMode
    };

    if (bleDevice && bleDevice.gatt.connected) {
        sendJsonCommandToBleDevice(command);
    } else {
        console.error('Bluetooth device is not connected');
    }
}


/**
 * @function sendJsonCommandToBleDevice
 * @description Sends a JSON command to the BLE device by encoding a command object and writing it to the BLE characteristic.
 * @param {Object} command - The command object to send (e.g., {msgType: 2, buzzer: true}).
 */
function sendJsonCommandToBleDevice(command) {
    if (!bleDevice || !bleDevice.gatt.connected) {
        console.error('Bluetooth device is not connected');
        return;
    }
    if (!commandCharacteristic) {
        console.error('Command characteristic not set');
        return;
    }

    // Convert the command object to a JSON string.
    let jsonString = JSON.stringify(command);

    // Encode the JSON string into a Uint8Array.
    let encoder = new TextEncoder();
    let encodedCommand = encoder.encode(jsonString);

    // Write the encoded command to the BLE characteristic.
    commandCharacteristic.writeValue(encodedCommand)
        .then(() => {
            console.log('JSON Command sent:', jsonString);
        })
        .catch(error => {
            console.error('Error sending JSON command:', error);
        });
}

/**
 * @function sendCommandToBleDevice
 * @description Sends a simple command (as a string) to the BLE device.
 * @param {string} command - The command character to send.
 */
function sendCommandToBleDevice(command) {
    if (!bleDevice || !bleDevice.gatt.connected) {
        console.error('Bluetooth device is not connected');
        return;
    }

    if (!commandCharacteristic) {
        console.error('Command characteristic not set');
        return;
    }

    // Convert the command string to a Uint8Array.
    let encoder = new TextEncoder();
    let encodedCommand = encoder.encode(command);

    commandCharacteristic.writeValue(encodedCommand)
        .then(() => {
            console.log('Command sent:', command);
        })
        .catch(error => {
            console.error('Error sending command:', error);
        });
}

/**
 * @function updateArea
 * @description Updates the area based on the drawn polygon on the map.
 * @param {Object} e - The event object from Mapbox GL Draw.
 */
function updateArea(e) {
    var data = draw.getAll();
    if (data.features.length > 0) {
        var area = turf.area(data);
        console.log(`Area: ${area} square meters`);
    } else {
        // No features available.
    }
}

/**
 * @function getTileUrls
 * @description Generates an array of tile URLs based on map bounds and zoom levels.
 * @param {Array} bounds - The geographic bounds as [[minLng, minLat], [maxLng, maxLat]].
 * @param {number} minZoom - The minimum zoom level.
 * @param {number} maxZoom - The maximum zoom level.
 * @returns {Array} Array of tile URLs.
 */
function getTileUrls(bounds, minZoom, maxZoom) {
    var urls = [];
    console.log(minZoom);
    console.log(maxZoom);
    for (var z = minZoom; z <= maxZoom; z++) {
        var topLeftTile = pointToTile(bounds[0][1], bounds[0][0], z);
        var bottomRightTile = pointToTile(bounds[1][1], bounds[1][0], z);

        console.log(`Top Left Tile (x, y): (${topLeftTile[0]}, ${topLeftTile[1]})`);
        console.log(`Bottom Right Tile (x, y): (${bottomRightTile[0]}, ${bottomRightTile[1]})`);

        for (var x = topLeftTile[0]; x <= bottomRightTile[0]; x++) {
            console.log(`x Tile: ${x}`);
            for (var y = topLeftTile[1]; y <= bottomRightTile[1]; y++) {
                console.log(`y Tile: ${y}`);
                var url = `https://api.mapbox.com/styles/v1/mapbox/streets-v11/tiles/${z}/${x}/${y}?access_token=${pMapBox_Token}`;
                console.log(`Url Added: ${url}`);
                console.log('Url Added');
                urls.push(url);
            }
        }
    }
    console.log('Returning Urls');
    return urls;
}

/**
 * @function pointToTile
 * @description Converts a geographic coordinate (lat, lon) to tile coordinates at a given zoom level.
 * @param {number} lat - Latitude.
 * @param {number} lon - Longitude.
 * @param {number} zoom - Zoom level.
 * @returns {Array} Tile coordinates as [x, y].
 */
function pointToTile(lat, lon, zoom) {
    var clampedLat = Math.max(Math.min(lat, 85.0511), -85.0511);
    var x = Math.floor((lon + 180) / 360 * Math.pow(2, zoom));
    var y = Math.floor((1 - Math.log(Math.tan(clampedLat * Math.PI / 180) + 1 / Math.cos(clampedLat * Math.PI / 180)) / Math.PI) / 2 * Math.pow(2, zoom));
    console.log(`PointToTile (x, y): (${x}, ${y})`);
    return [x, y];
}

/**
 * @description Event listener for the "checkDownloadButton" to check for downloaded maps.
 */
document.getElementById('checkDownloadButton').addEventListener('click', function () {
    var mapName = prompt("Enter the map name to check:");
    if (mapName) {
        getTilesForMap(mapName);
    } else {
        alert("You must enter a map name.");
    }
});

/**
 * @description Event listener for the "downloadMapButton" to download map tiles.
 */
document.getElementById('downloadMapButton').addEventListener('click', async function () {
    var mapName = prompt("Enter a name for the map area you want to download:");
    if (!mapName) {
        alert("Download cancelled: You must provide a name for the map.");
        return;
    }

    var bounds = draw.getAll().features[0].geometry.coordinates[0];
    var minZoom = 9;
    var maxZoom = 12;

    // Flatten the bounds for compatibility with getTileUrls.
    var flatBounds = [
        [Math.min(...bounds.map(coord => coord[0])), Math.min(...bounds.map(coord => coord[1]))],
        [Math.max(...bounds.map(coord => coord[0])), Math.max(...bounds.map(coord => coord[1]))]
    ];

    console.log(flatBounds[0]);
    console.log(flatBounds[1]);
    urls = getTileUrls(flatBounds, minZoom, maxZoom);

    for (const url of urls) {
        console.log(`Downloading tile from: ${url}`);
        try {
            await downloadTile(url, mapName);
            console.log(`Downloaded tile from: ${url}`);
        } catch (error) {
            console.error(`Error downloading tile from ${url}:`, error);
        }
    }
    console.log('All tiles downloaded.');
});

/**
 * @function downloadTile
 * @description Downloads a tile from the given URL and stores it in IndexedDB.
 * @param {string} url - The URL of the tile to download.
 * @param {string} mapName - The name of the map area.
 * @returns {Promise} A promise that resolves when the tile is stored.
 */
function downloadTile(url, mapName) {
    console.log(`Starting download for ${url}`);
    return fetch(url)
        .then(response => {
            if (!response.ok) throw new Error('Network response was not ok');
            return response.blob();
        })
        .then(blob => {
            storeTileInIndexedDB(blob, url, mapName);
        })
        .catch(error => {
            console.error('There has been a problem with your fetch operation:', error);
        });
}

/**
 * @function storeTileInIndexedDB
 * @description Stores a downloaded tile (blob) in IndexedDB.
 * @param {Blob} blob - The blob data of the tile.
 * @param {string} url - The URL used as part of the key.
 * @param {string} mapName - The name of the map area.
 */
function storeTileInIndexedDB(blob, url, mapName) {
    var open = indexedDB.open('MyDatabase', pDBVersionNumber);
    open.onupgradeneeded = function (event) {
        var db = event.target.result;
        var store;
        if (!db.objectStoreNames.contains('MyObjectStore')) {
            store = db.createObjectStore('MyObjectStore', { keyPath: 'id' });
        } else {
            store = event.target.transaction.objectStore('MyObjectStore');
        }
        if (!store.indexNames.contains('mapNameIndex')) {
            store.createIndex('mapNameIndex', 'mapName', { unique: false });
        }
    };

    open.onsuccess = function () {
        var db = open.result;
        var tx = db.transaction('MyObjectStore', 'readwrite');
        var store = tx.objectStore('MyObjectStore');
        var tileId = `${mapName}_${url}`;
        var item = {
            id: tileId,
            blobData: blob,
            mapName: mapName
        };
        store.put(item);
        tx.oncomplete = function () {
            console.log('Tile stored successfully');
            db.close();
        };
        tx.onerror = function (event) {
            console.error('Error storing tile:', event.target.error);
        };
    };

    open.onerror = function (event) {
        console.error('Error opening IndexedDB:', event.target.error);
    };
}

/**
 * @function getTilesForMap
 * @description Retrieves all tiles for a given map area from IndexedDB.
 * @param {string} mapName - The name of the map area.
 */
function getTilesForMap(mapName) {
    var open = indexedDB.open('MyDatabase', pDBVersionNumber);
    open.onsuccess = function () {
        var db = open.result;
        var tx = db.transaction('MyObjectStore', 'readonly');
        var store = tx.objectStore('MyObjectStore');
        var index = store.index('mapNameIndex');
        var query = index.getAll(mapName);
        query.onsuccess = function () {
            var tiles = query.result;
            console.log('Tiles for ' + mapName + ':', tiles);
        };
        tx.oncomplete = function () {
            db.close();
        };
    };
}

/**
 * @function deleteTilesForMap
 * @description Deletes all tiles associated with a given map name from IndexedDB.
 * @param {string} mapName - The name of the map area.
 */
function deleteTilesForMap(mapName) {
    var open = indexedDB.open('MyDatabase', pDBVersionNumber);
    open.onsuccess = function () {
        var db = open.result;
        var tx = db.transaction('MyObjectStore', 'readwrite');
        var store = tx.objectStore('MyObjectStore');
        var index = store.index('mapNameIndex');
        var query = index.getAllKeys(mapName);
        query.onsuccess = function () {
            var keys = query.result;
            keys.forEach(function (key) {
                store.delete(key);
            });
        };
        tx.oncomplete = function () {
            console.log('All tiles for ' + mapName + ' have been deleted.');
            db.close();
        };
        tx.onerror = function (event) {
            console.error('Error deleting tiles for ' + mapName, event.target.error);
        };
    };
};

/**
 * @function getTileFromIndexedDB
 * @description Retrieves a tile from IndexedDB given tile coordinates.
 * @param {number} x - Tile x-coordinate.
 * @param {number} y - Tile y-coordinate.
 * @param {number} z - Zoom level.
 * @returns {Promise} A promise that resolves with the local URL for the tile, or rejects if not found.
 */
function getTileFromIndexedDB(x, y, z) {
    const id = `https://api.mapbox.com/styles/v1/mapbox/streets-v11/tiles/${z}/${x}/${y}?access_token=${pMapBox_Token}`;
    return new Promise((resolve, reject) => {
        const open = indexedDB.open('MyDatabase', pDBVersionNumber);
        open.onsuccess = function () {
            const db = open.result;
            const transaction = db.transaction('MyObjectStore', 'readonly');
            const store = transaction.objectStore('MyObjectStore');
            const request = store.get(id);
            request.onsuccess = function () {
                const data = request.result;
                if (data) {
                    const blob = data.blobData;
                    const url = URL.createObjectURL(blob);
                    resolve(url);
                } else {
                    reject(new Error('Tile not found in IndexedDB'));
                }
            };
            request.onerror = function () {
                reject(new Error('Error fetching tile from IndexedDB'));
            };
        };
    });
}

/**
 * @description Overrides the tile loading mechanism for the map once the style has loaded.
 *
 * It attempts to load tiles from IndexedDB first; if not found, it falls back to network requests.
 */
map.on('style.load', function () {
    function loadTileFromIndexedDB(coords) {
        const id = `https://api.mapbox.com/styles/v1/mapbox/streets-v11/tiles/${coords.z}/${coords.x}/${coords.y}?access_token=${pMapBox_Token}`;
        return new Promise((resolve, reject) => {
            const open = indexedDB.open('MyDatabase', pDBVersionNumber);
            open.onsuccess = function () {
                const db = open.result;
                const transaction = db.transaction('MyObjectStore', 'readonly');
                const store = transaction.objectStore('MyObjectStore');
                const request = store.get(id);

                request.onsuccess = function () {
                    const data = request.result;
                    if (data) {
                        const localUrl = URL.createObjectURL(data.blobData);
                        resolve(localUrl);
                    } else {
                        reject(new Error('Tile not found in IndexedDB'));
                    }
                };

                request.onerror = function () {
                    reject(new Error('Error fetching tile from IndexedDB'));
                };
            };
        });
    }

    // Update tile sources for raster layers.
    map.getStyle().layers.forEach(layer => {
        if (layer.type === 'raster' || layer.type === 'raster-dem') {
            const originalTiles = layer.source.tiles;
            map.removeSource(layer.source);
            map.addSource(layer.source, {
                type: layer.type,
                tiles: [function (coords) {
                    return loadTileFromIndexedDB(coords).catch(() => {
                        return originalTiles[0].replace('{x}', coords.x).replace('{y}', coords.y).replace('{z}', coords.z);
                    });
                }],
                tileSize: layer.tileSize || 256
            });
        }
    });
});

/**
 * @function constructTileURL
 * @description Constructs a tile URL from a template and tile coordinates.
 * @param {string} template - The URL template (with placeholders {x}, {y}, {z}).
 * @param {Object} coords - An object with x, y, and z properties representing tile coordinates.
 * @returns {string} The constructed URL.
 */
function constructTileURL(template, coords) {
    return template.replace('{x}', coords.x)
        .replace('{y}', coords.y)
        .replace('{z}', coords.z);
}
