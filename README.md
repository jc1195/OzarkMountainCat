# OzarkMountainCat

OzarkMountainCat is an open-source DIY pet tracker project designed specifically for RAK WisBlock Modular Arduino chips. This project allows you to track your pet off-grid using LoRa for long-range communication, GPS for precise location data, and BLE for interfacing with a web app on any Chromium-based device. The project was developed to track my cat, Frizzle, and is ideal for outdoor adventures like camping, where cellular coverage is not available.

## Features

- **LoRa Communication:**  
  The harness communicates with a receiver through LoRa, acting like a walkie-talkie for off-grid communication. All packets sent and received via LoRa are in JSON format, which makes the code easier to maintain and debug.

- **GPS Tracking:**  
  Provides real-time location data (latitude, longitude, altitude, satellites in view, HDOP, and local time).

- **Buzzer Alerts:**  
  A buzzer module is used to emit sound alerts when GPS signals are weak (for example, when your pet is hiding under cars or rocks), helping you locate your pet.

- **RGB Lighting:**  
  An Adafruit Flora RGB Smart NeoPixel is used for customizable color lighting on the harness (for night-time visibility). *(RGB code is coming soon.)*

- **Waterproof Design:**  
  Both the harness and receiver housings are waterproof, thanks to 3D-printed enclosures.

- **Offline Mapping:**  
  The OMC_APP web application allows you to download maps in advance, so you can monitor your pet's location even without cellular or internet service. The app features 3D maps for an enhanced tracking experience.

## Hardware Requirements

This project is built specifically for RAK WisBlock Modular Arduino chips. The following components are required:

- **RAK4631 Core Module:**  
  The main computing module controlling all functions.  
  [RAK4631 LPWAN Node](https://store.rakwireless.com/products/rak4631-lpwan-node?f=5&variant=37505443987654)

- **RAK WisBlock Baseboards:**  
  - **Mini Baseboard (RAK19003):**  
    [RAK19003 WisBlock Base Board](https://store.rakwireless.com/products/wisblock-base-board-rak19003?f=5&s=1)
  - **2nd Gen Baseboard (RAK19007):**  
    [RAK19007 WisBlock Base Board 2nd Gen](https://store.rakwireless.com/products/rak19007-wisblock-base-board-2nd-gen?f=5&s=1)

- **GNSS Module:**  
  Provides GPS location data.  
  [RAK12500 WisBlock GNSS Location Module](https://store.rakwireless.com/products/wisblock-gnss-location-module-rak12500?f=5&s=4)

- **Buzzer Module:**  
  Used for sound alerts to help locate your pet when GPS signals are weak.  
  [RAK18001 WisBlock Buzzer Module](https://store.rakwireless.com/products/wisblock-buzzer-module-rak18001?f=5)

- **RGB LED:**  
  Adafruit Flora RGB Smart NeoPixels are used for customizable lighting.  
  [Adafruit Flora RGB Smart NeoPixels](https://www.amazon.com/dp/B00KBXTJRQ?ref=ppx_yo2ov_dt_b_fed_asin_title)

- **Additional Components:**  
  Battery, cables, connectors, and a power supply as required.
  Waterproof enclosures (3D printed) for both the harness and receiver.

## Software Requirements

This project is developed using PlatformIO for the RAK WisBlock Modular Arduino chips. It is specific to these chips and will only work with the following libraries (install them via PlatformIO):

- **Adafruit NeoPixel** by Adafruit  
- **ArduinoJson** by Benoit Blanchon  
- **SX126x-Arduino** by Bernd Giesecke  
- **SparkFun u-blox GNSS Arduino Library** by SparkFun Electronics

Additionally, it uses:
- **Adafruit Bluefruit nRF52 Library** for BLE communication.
- **FreeRTOS** (integrated in the board support package).
- **SX126x-RAK4630** (RAK provided LoRa library).


