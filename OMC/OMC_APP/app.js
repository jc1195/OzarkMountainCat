var map; // Global variable to hold the map instance
window.map = map;

var trackerMarker; // Global variable to hold the tracker's map marker
var userLocationMarker; // Global variable to hold the user's location marker
var bleDevice = null; 
let commandCharacteristic;
var draw;
var urls = [];

// Mapbox Info
var pMapBox_Token = 'pk.eyJ1IjoiamMxMTk1IiwiYSI6ImNsaW54c2tuZjAzaDIzcm53YjQ2MHFkNTYifQ.Gvtk1A-oWSIUPeGLi1Pflg';
var pMapBox_URL = 'mapbox://mapbox.mapbox-terrain-dem-v1';

// BLE Info
var pBLE_PrimaryGUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b'
var pBLE_CharacteristicGUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8'
// var pBLE_PrimaryGUID = '0x1234'
// var pBLE_CharacteristicGUID = '0x4231'

//
var pDBVersionNumber = 3

document.addEventListener('DOMContentLoaded', (event) => {
    console.log("PWA Pet Tracker is ready!");
    updateBatteryLevel(0, 1);
    updateBatteryLevel(0, 2);

    // Initialize the map
    mapboxgl.accessToken = pMapBox_Token; // Replace with your actual MapBox access token
    map = new mapboxgl.Map({
        container: 'map', // container ID
        style: 'mapbox://styles/mapbox/outdoors-v11', // style URL
        center: [-74.5, 40], // starting position [lng, lat]
        zoom: 9 // starting zoom
    });

     // Initialize Mapbox GL Draw after the map is set up
    map.on('load', function() {
        // Now that the map is loaded, initialize Mapbox GL Draw
        draw = new MapboxDraw({
            displayControlsDefault: false,
            controls: {
                polygon: true,
                trash: true
            }
        });
        map.addControl(draw);

        // Set up the rest of your event listeners here
        map.on('draw.create', updateArea);
        map.on('draw.delete', updateArea);
        map.on('draw.update', updateArea);

        map.addSource('mapbox-dem', {
        'type': 'raster-dem',
        'url': pMapBox_URL,
        'tileSize': 512,
        'maxzoom': 14
        });
        map.setTerrain({ 'source': 'mapbox-dem', 'exaggeration': 1.5 });

        // Add a sky layer to simulate the atmosphere's color
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

    map.addControl(new mapboxgl.NavigationControl());
    const zoomDisplay = document.createElement('div');
    zoomDisplay.setAttribute('id', 'zoom-level');
    zoomDisplay.style.background = 'white';
    zoomDisplay.style.padding = '5px';
    zoomDisplay.style.margin = '10px';
    zoomDisplay.style.borderRadius = '3px';
    zoomDisplay.textContent = `Zoom Level: ${map.getZoom().toFixed(2)}`;
    document.body.appendChild(zoomDisplay);

    // Update the zoom level display on the map when the zoom changes
    map.on('zoom', () => {
        zoomDisplay.textContent = `Zoom Level: ${map.getZoom().toFixed(2)}`;
    });

    // Initialize geolocation
    if ('geolocation' in navigator) {
        userLocationMarker = new mapboxgl.Marker();
        var initialPositionSet = false;

        const locationWatchID = navigator.geolocation.watchPosition(position => {
            var lat = position.coords.latitude;
            var lng = position.coords.longitude;

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

        window.onbeforeunload = function() {
            navigator.geolocation.clearWatch(locationWatchID);
        };
    } else {
        console.log('Geolocation is not supported by this browser.');
    }

    let isConnected = false;
    const connectButton = document.getElementById('connectButton');
    connectButton.addEventListener('click', function() {
        console.log('Connect button clicked');

        navigator.bluetooth.requestDevice({
            filters: [{ name: 'Mount' }],
            optionalServices: [pBLE_PrimaryGUID, pBLE_CharacteristicGUID] // Use both service UUIDs
        })
        .then(device => {
            bleDevice = device; // Set the global bleDevice variable here
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

    const disconnectButton = document.getElementById('disconnectButton');
    disconnectButton.addEventListener('click', function() {
        if (bleDevice && bleDevice.gatt.connected) {
            bleDevice.gatt.disconnect();
            console.log('Device disconnected');
        }
    });

    function onDisconnected() {
        console.log('Device disconnected');
        isConnected = false;
        updateConnectionStatus(isConnected);
    }

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

function setLightColor() {
    var selectedColor = document.getElementById('lightColorSelect').value;
    console.log('Selected Light Color:', selectedColor);

    if (bleDevice && bleDevice.gatt.connected) {
        sendCommandToBleDevice(selectedColor);
    } else {
        console.error('Bluetooth device is not connected');
    }
}

// function setPowerMode() {
//     var selectedMode = document.getElementById('powerModeSelect').value;
//     console.log('Selected Power Mode:', selectedMode);

//     if (bleDevice && bleDevice.gatt.connected) {
//         sendCommandToBleDevice(selectedMode);
//     } else {
//         console.error('Bluetooth device is not connected');
//     }
// }

function toggleSidebar() {
    var sidebar = document.getElementById('sidebar');
    if (sidebar.style.left === '-250px') {
        sidebar.style.left = '0';
    } else {
        sidebar.style.left = '-250px';
    }
}
document.addEventListener('click', function(event) {
    var sidebar = document.getElementById('sidebar');
    var hamburger = document.getElementById('hamburger');
    
    // Check if the clicked area is outside the sidebar and not the hamburger menu button
    if (!sidebar.contains(event.target) && !hamburger.contains(event.target)) {
        sidebar.style.left = '-250px'; // Hide the sidebar
    }
});



// function parseLightStatus(status) {
//     return status === '0' ? 'Off' : 'On';
// }

function turnOffLight() {
    if (bleDevice && bleDevice.gatt.connected) {
        let command = {
            msgType: 0,
            r: 0,
            g: 0,
            b: 0
        }
        sendJsonCommandToBleDevice(command);
    }
}

function turnOnBuzzer() {
    if (bleDevice && bleDevice.gatt.connected) {
        let command = {
            msgType: 2,
            buzzer: true,
        }
        sendJsonCommandToBleDevice(command);
    }
}

function turnOffBuzzer() {
    if (bleDevice && bleDevice.gatt.connected) {
        let command = {
            msgType: 2,
            buzzer: false,
        }
        sendJsonCommandToBleDevice(command);
    }
}

function setPwrMode() {
    var selectedMode = document.getElementById('powerModeSelect').value;
    console.log('Selected Power Mode:', selectedMode);

    let command = {
        msgType: 5,
        mode: selectedMode
    }

    if (bleDevice && bleDevice.gatt.connected) {
        sendJsonCommandToBleDevice(command);
    } else {
        console.error('Bluetooth device is not connected');
    }
}


function pickColorAndTurnOnLight() {
    const colorMap = {
        'Rainbow': 'M',
        'Red': 'L',
        'Pink': 'K',
        'Purple': 'J',
        'Orange': 'I',
        'BrightOrange': 'H',
        'Yellow': 'G',
        'Bright Green': 'F',
        'Green': 'E',
        'Blue': 'D',
        'Cyan': 'C',
        'White': 'B',
        'Off': 'A' // Though this option is redundant here
    };

    let colorChoice = prompt("Enter color (Rainbow, Red, Pink, etc.):");
    let command = colorMap[colorChoice];

    if (command && bleDevice && bleDevice.gatt.connected) {
        sendCommandToBleDevice(command);
    } else {
        console.error('Invalid color choice or device not connected');
    }
}

function sendJsonCommandToBleDevice(command) {
    if (!bleDevice || !bleDevice.gatt.connected) {
        console.error('Bluetooth device is not connected');
        return;
    }
    if (!commandCharacteristic) {
        console.error('Command characteristic not set');
        return;
    }
    
    // Convert the object to a JSON string.
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

 // This should be set when connecting to the BLE device
function sendCommandToBleDevice(command) {
    if (!bleDevice || !bleDevice.gatt.connected) {
        console.error('Bluetooth device is not connected');
        return;
    }

    if (!commandCharacteristic) {
        console.error('Command characteristic not set');
        return;
    }

    // Convert the command to a format that the Bluetooth device can understand
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

function updateArea(e) {
    var data = draw.getAll();
    if (data.features.length > 0) {
        var area = turf.area(data);
        // Display the area of the polygon in square meters
        console.log(`Area: ${area} square meters`);
        // You can also get the bounding box or coordinates of the polygon here
    } else {
        // No features in draw.getAll()
    }
}

function getTileUrls(bounds, minZoom, maxZoom) {
    var urls = [];
    console.log(minZoom);
    console.log(maxZoom);
    for (var z = minZoom; z <= maxZoom; z++) {
        var topLeftTile = pointToTile(bounds[0][1], bounds[0][0], z);
        var bottomRightTile = pointToTile(bounds[1][1], bounds[1][0], z);

        console.log(`Top Left Tile (x, y): (${topLeftTile[0]}, ${topLeftTile[1]})`);
        console.log(`Bottom Right Tile (x, y): (${bottomRightTile[0]}, ${bottomRightTile[1]})`);
        //console.log(`Zoom Level (z): ${z}`);

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

// Converts latitude and longitude to tile coordinates
function pointToTile(lat, lon, zoom) {
    // Clamp latitude to the range supported by the Mercator projection
    var clampedLat = Math.max(Math.min(lat, 85.0511), -85.0511);

    var x = Math.floor((lon + 180) / 360 * Math.pow(2, zoom));
    var y = Math.floor((1 - Math.log(Math.tan(clampedLat * Math.PI / 180) + 1 / Math.cos(clampedLat * Math.PI / 180)) / Math.PI) / 2 * Math.pow(2, zoom));

    console.log(`PointToTile (x, y): (${x}, ${y})`);
    return [x, y];
}

document.getElementById('checkDownloadButton').addEventListener('click', function() {
    var mapName = prompt("Enter the map name to check:");
    if (mapName) {
        getTilesForMap(mapName);
    } else {
        alert("You must enter a map name.");
    }
});


// Download offline maps
document.getElementById('downloadMapButton').addEventListener('click', async function () {
    var mapName = prompt("Enter a name for the map area you want to download:");
    if (!mapName) {
        alert("Download cancelled: You must provide a name for the map.");
        return;
    }
    
    var bounds = draw.getAll().features[0].geometry.coordinates[0];
    var minZoom = 9; // Set minimum zoom level for tiles to download
    var maxZoom = 12; // Set maximum zoom level for tiles to download

    // Flatten the bounds to be compatible with getTileUrls function
    var flatBounds = [
        [Math.min(...bounds.map(coord => coord[0])), Math.min(...bounds.map(coord => coord[1]))],
        [Math.max(...bounds.map(coord => coord[0])), Math.max(...bounds.map(coord => coord[1]))]
    ];

    console.log(flatBounds[0])
    console.log(flatBounds[1])
    // Now urls is globally accessible, it can be populated here
    urls = getTileUrls(flatBounds, minZoom, maxZoom); // Populate the global urls variable

    // Now, use the urls to download tiles and store them
    for (const url of urls) {
        console.log(`Downloading tile from: ${url}`);
        try {
            await downloadTile(url,mapName);
            console.log(`Downloaded tile from: ${url}`);
        } catch (error) {
            console.error(`Error downloading tile from ${url}:`, error);
        }
    }

    console.log('All tiles downloaded.');
    
});

// function showDownloadSuccess(url) {
//   console.log(`Successfully downloaded: ${url}`);
//   // Update the UI to show that the tile has been downloaded
// }

function downloadTile(url, mapName) {
    console.log(`Starting download for ${url}`);
  return fetch(url)
    .then(response => {
      if (!response.ok) throw new Error('Network response was not ok');
      return response.blob();
    })
    .then(blob => {
      // Here you would add the logic to store the blob in IndexedDB
      storeTileInIndexedDB(blob, url, mapName);
    })
    .catch(error => {
      console.error('There has been a problem with your fetch operation:', error);
    });
}
    
function storeTileInIndexedDB(blob, url, mapName) {
    var open = indexedDB.open('MyDatabase', pDBVersionNumber);

    // This event is only triggered if the database needs to be created or upgraded.
    open.onupgradeneeded = function(event) {
        var db = event.target.result; // Get the database from the event target

        // Create the object store with 'id' as the key path if it doesn't exist
        var store;
        if (!db.objectStoreNames.contains('MyObjectStore')) {
            store = db.createObjectStore('MyObjectStore', { keyPath: 'id' });
        } else {
            store = event.target.transaction.objectStore('MyObjectStore');
        }

        // Create an index on 'mapName' if it doesn't exist
        if (!store.indexNames.contains('mapNameIndex')) {
            store.createIndex('mapNameIndex', 'mapName', { unique: false });
        }
    };

    open.onsuccess = function() {
        var db = open.result;
        var tx = db.transaction('MyObjectStore', 'readwrite');
        var store = tx.objectStore('MyObjectStore');

        var tileId = `${mapName}_${url}`; // Construct a unique ID for each tile

        var item = {
            id: tileId,
            blobData: blob,
            mapName: mapName // Include the map name when storing the tile
        };

        store.put(item); // Store or update the tile in the object store

        tx.oncomplete = function() {
            console.log('Tile stored successfully');
            db.close();
        };

        tx.onerror = function(event) {
            console.error('Error storing tile:', event.target.error);
        };
    };

    open.onerror = function(event) {
        console.error('Error opening IndexedDB:', event.target.error);
    };
}

function getTilesForMap(mapName) {
    var open = indexedDB.open('MyDatabase', pDBVersionNumber);
    
    open.onsuccess = function() {
        var db = open.result;
        var tx = db.transaction('MyObjectStore', 'readonly');
        var store = tx.objectStore('MyObjectStore');
        var index = store.index('mapNameIndex');
        
        var query = index.getAll(mapName);
        
        query.onsuccess = function() {
            var tiles = query.result;
            console.log('Tiles for ' + mapName + ':', tiles);
            // Display the tiles or their count in the UI
        };
        
        tx.oncomplete = function() {
            db.close();
        };
    };
}

function deleteTilesForMap(mapName) {
    var open = indexedDB.open('MyDatabase', pDBVersionNumber);
    
    open.onsuccess = function() {
        var db = open.result;
        var tx = db.transaction('MyObjectStore', 'readwrite');
        var store = tx.objectStore('MyObjectStore');
        var index = store.index('mapNameIndex');
        
        // Get all entries for the given mapName
        var query = index.getAllKeys(mapName);
        
        query.onsuccess = function() {
            var keys = query.result;
            keys.forEach(function(key) {
                store.delete(key); // Delete each tile
            });
        };
        
        tx.oncomplete = function() {
            console.log('All tiles for ' + mapName + ' have been deleted.');
            db.close();
        };
        
        tx.onerror = function(event) {
            console.error('Error deleting tiles for ' + mapName, event.target.error);
        };
    };
};

// Function to serve tiles from IndexedDB
function getTileFromIndexedDB(x, y, z) {
    // This should match how you've stored the tiles in IndexedDB
    const id = `https://api.mapbox.com/styles/v1/mapbox/streets-v11/tiles/${z}/${x}/${y}?access_token=${pMapBox_Token}`;

    
    
    // Open the IndexedDB and fetch the tile blob using the id
    return new Promise((resolve, reject) => {
        const open = indexedDB.open('MyDatabase', pDBVersionNumber);
        open.onsuccess = function() {
            const db = open.result;
            const transaction = db.transaction('MyObjectStore', 'readonly');
            const store = transaction.objectStore('MyObjectStore');
            const request = store.get(id);
            request.onsuccess = function() {
                const data = request.result;
                if (data) {
                    const blob = data.blobData;
                    const url = URL.createObjectURL(blob); // Create a local URL to be used as the tile source
                    resolve(url);
                } else {
                    reject(new Error('Tile not found in IndexedDB'));
                }
            };
            request.onerror = function() {
                reject(new Error('Error fetching tile from IndexedDB'));
            };
        };
    });
}
// When the map loads, override the way it requests tiles to check for local tiles first
// map.on('style.load', function() {
   
//     const layers = map.getStyle().layers;

//     for (const layer of layers) {
//         if (layer.type === 'raster' || layer.type === 'raster-dem') {
//             layer.source.tiles = [function(coords) {
//             // Construct the URL for the local tiles
//                 const url = constructTileURL(layer.source.tiles[0], coords);
//                 // Try to load from IndexedDB first
//                 return getTileFromIndexedDB(coords.x, coords.y, coords.z).catch(() => {
//                 // If not found in IndexedDB, fall back to the network URL
//                     return url;
//                 });
//             }];
//         }
//     }
// });

// Override the tile loading mechanism when the map's style finishes loading
map.on('style.load', function() {
    // Define a function to replace the network-based tile request with the local IndexedDB request
    function loadTileFromIndexedDB(coords) {
        // Construct the tile ID
        const id = `https://api.mapbox.com/styles/v1/mapbox/streets-v11/tiles/${coords.z}/${coords.x}/${coords.y}?access_token=${pMapBox_Token}`;
        
        // Return a promise that resolves with the object URL or rejects if not found
        return new Promise((resolve, reject) => {
            const open = indexedDB.open('MyDatabase', pDBVersionNumber);
            open.onsuccess = function() {
                const db = open.result;
                const transaction = db.transaction('MyObjectStore', 'readonly');
                const store = transaction.objectStore('MyObjectStore');
                const request = store.get(id);
                
                request.onsuccess = function() {
                    const data = request.result;
                    if (data) {
                        // Create a local URL for the Blob
                        const localUrl = URL.createObjectURL(data.blobData);
                        resolve(localUrl);
                    } else {
                        reject(new Error('Tile not found in IndexedDB'));
                    }
                };

                request.onerror = function() {
                    reject(new Error('Error fetching tile from IndexedDB'));
                };
            };
        });
    }

    // Loop through all layers and update the source if it's a raster type
    map.getStyle().layers.forEach(layer => {
        if (layer.type === 'raster' || layer.type === 'raster-dem') {
            // Here you would override the source to use local tiles
            const originalTiles = layer.source.tiles;
            // Replace the original source with a new one that includes the override logic
            map.removeSource(layer.source);
            map.addSource(layer.source, {
                type: layer.type,
                tiles: [function(coords) {
                    return loadTileFromIndexedDB(coords).catch(() => {
                        // Optional: If not found in IndexedDB, fall back to the original URL
                        // Remove this line if you don't want any network requests to occur
                        return originalTiles[0].replace('{x}', coords.x).replace('{y}', coords.y).replace('{z}', coords.z);
                    });
                }],
                tileSize: layer.tileSize || 256
            });
        }
    });
});

// map.on('style.load', function() {
//     // Go through each layer and update the source if it's of the type that should be overridden
//     map.getStyle().layers.forEach(layer => {
//         if (layer.type === 'raster' || layer.type === 'raster-dem') {
//             const originalTiles = map.getSource(layer.source).tiles;
//             map.removeSource(layer.source); // Remove the existing source

//             // Add a new source with the same name that includes the override for IndexedDB tiles
//             map.addSource(layer.source, {
//                 type: 'raster',
//                 tiles: [function(coords) {
//                     // Attempt to load from IndexedDB first, and if not available, fall back to the original source URLs
//                     return getTileFromIndexedDB(coords.x, coords.y, coords.z)
//                         .catch(() => constructTileURL(originalTiles[0], coords));
//                 }],
//                 tileSize: layer.source.tileSize
//             });

//             // Reference the updated source in the existing layer
//             map.setLayoutProperty(layer.id, 'source', layer.source);
//         }
//     });
// });


// // This function will replace the network-based tile request with the local IndexedDB request
// function loadTileFromIndexedDB(url) {
//   return new Promise((resolve, reject) => {
//     // Open the IndexedDB database
//     const open = indexedDB.open('MyDatabase', 1);

//     open.onsuccess = function() {
//       const db = open.result;
//       const transaction = db.transaction('MyObjectStore', 'readonly');
//       const store = transaction.objectStore('MyObjectStore');
//       const request = store.get(url);

//       request.onsuccess = function(event) {
//         const data = event.target.result;
//         if (data) {
//           // Create a local URL for the Blob
//           const localUrl = URL.createObjectURL(data.blobData);
//           resolve(localUrl);
//         } else {
//           reject('Tile not found in IndexedDB');
//         }
//       };

//       request.onerror = function(event) {
//         reject('Error fetching tile from IndexedDB');
//       };
//     };
//   });
// }

// // When the map loads, override the way it requests tiles
// map.on('style.load', function() {
//   const layers = map.getStyle().layers;

//   for (const layer of layers) {
//     if (layer.type === 'raster' || layer.type === 'raster-dem') {
//       // Here you would override the source to use local tiles
//       // This is just an example and will need to be adapted
//       layer.source.tiles.forEach((tileUrl, index, array) => {
//         array[index] = function(tileCoords) {
//           const url = constructTileURL(layer.source.tiles[0], tileCoords);
//           return loadTileFromIndexedDB(url).catch(() => {
//             // If not found in IndexedDB, fall back to the original URL
//             return url;
//           });
//         };
//       });
//     }
//   }
// });

// Helper function to construct the URL for a given tile based on coordinates
function constructTileURL(template, coords) {
  return template.replace('{x}', coords.x)
                 .replace('{y}', coords.y)
                 .replace('{z}', coords.z);
}