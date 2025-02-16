#include <Arduino.h>
#include <heltec.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Button.h>
#include <EEPROM.h>
#include <battery.h>
//#include <str.h>
#include <string.h>


#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BAND 915E6 // you can set band here directly,e.g. 868E6,915E6
#define POWER_SAVING_MODE 'N'
#define WAKING_UP 'O'
#define EXTREME_PSM 'Q'

void SetPacketData();
void LoRaData();
void LoadPacket(int packetSize);
void DisplayData();
void SendPacket(char *message);

struct Packet1
{
    char deviceID[15];
    char longitude[12];
    char latitude[12];
    char HBatt[5];
    char SIV[4];
    char hdop[6];
    char powerMode[3];
    char lightStatus[3];
    char colorChoice[3];
    char chour[4];
    char cmin[4];
    char csec[4];
    char repeatBatt[5];
    char repeatRSSI[5];
};

extern Packet1 rcvdPacket;
extern String packet;
extern char charPacket[150];
extern int battStatus;
