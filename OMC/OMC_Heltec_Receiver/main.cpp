#include <main.h>

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

bool extremePSM = false;
bool pwrSaveMde = false;
bool lightOn = true;
char charPacket[150];
bool isActivated = false;

String rssi = "RSSI --";
String packSize = "--";
String activationStatus;
String deviceName;

int analogPin = 37; // Wifi Kit 32 shows analog value of voltage in A4
                    // variable to store the value read

//******************************** needed for running average (smoothening the analog value)
const int numReadings = 32; // the higher the value, the smoother the average
int readings[numReadings];  // the readings from the analog input
int readIndex = 0;          // the index of the current reading
int total = 0;              // the running total
int average = 0;            // the average
int count = 0;

char data[150];

void ShortPress();
void LongPress();
Button button1(0, INTERNAL_RESISTOR, 1000, ShortPress, LongPress);
bool displayStatus = true;
uint32_t currentMillis = 0;
uint32_t battPrevMilli = 0;
uint32_t currentTransmition;
uint32_t previousTransmition;
int battStatus;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  String readData = "               ";
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
  for (int i = 0; strlen(data); i++)
  {
    readData[i] = data[i];
  }
  return readData;
}

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    pCharacteristic->setValue(charPacket);
  }

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();
    char cValue[15];

    for (int i = 0; i < 15; i++)
    {
      cValue[i] = value[i];
    }
    SendPacket(cValue);
  }
};

void enableBLE()
{
  // Create the BLE Device
  BLEDevice::init("CatTracker");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->setCallbacks(new MyCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
}

void enableLora()
{
  LoRa.setTxPower(20, RF_PACONFIG_PASELECT_PABOOST);
  LoRa.setSpreadingFactor(10);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(4);
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
}

void setup()
{
  pinMode(analogPin, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(0, INPUT);

  Heltec.begin(false /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  Heltec.display->init();
  // Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);

  delay(1500);
  Heltec.display->clear();
  enableLora();
  Heltec.display->drawStringMaxWidth(0, 0, 120, "Welcome to Ozark Mountain Cat!");
  Heltec.display->drawString(0, 25, "Checking Activation...");
  Heltec.display->display();
  delay(2000);
  // checkActivation();
  enableBLE();
  BLEDevice::startAdvertising();
  Batt.begin(false);
  battStatus = Batt.GetVBatt();

  for (int thisReading = 0; thisReading < numReadings; thisReading++) // used for smoothening
  {
    readings[thisReading] = 0;
  }
  // LoRa.onReceive(cbk);
  LoRa.receive();
}

void loop()
{
  currentMillis = millis();
  if ((currentMillis - battPrevMilli) >= 30000)
  {
    battPrevMilli = currentMillis;
    battStatus = Batt.GetVBatt();
    if (displayStatus)
    {
      DisplayData();
    }
  }
  button1.checkPress();

  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    LoadPacket(packetSize);
    DisplayData();
    if (deviceConnected)
    {
      pCharacteristic->setValue(charPacket);
      pCharacteristic->notify();
      // pCharacteristic->setValue(cstring);
      delay(10);
    }
  }

  
  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

void ShortPress()
{
  LoRa.beginPacket();
  LoRa.print("2");
  LoRa.endPacket();
  // if (pwrSaveMde){
  //   delay(1000);
  //   LoRa.beginPacket();
  //   LoRa.print("2");
  //   LoRa.endPacket();
  // }
}

void LongPress()
{
  if (displayStatus)
  {
    Heltec.display->displayOff();
    displayStatus = false;
  }
  else
  {
    Heltec.display->displayOn();
    displayStatus = true;
  }
}
