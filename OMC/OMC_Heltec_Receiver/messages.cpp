#include <main.h>

Packet1 rcvdPacket;
int hour;
int mint;
int sec;
int prevHour = 0;
int prevMin = 0;
int prevSec = 0;

String packet;
int packetSize;

void LoadCharPacket()
{
    for (int i = 0; i < 150; i++){
        charPacket[i] = packet[i];
    }
}

void SetPacketData()
{
    char data[150];
    strcpy(data, charPacket);
    char *token = strtok(data, " ");
    int count = 0;
    while (token != NULL)
    {
        switch (count)
        {
        case 0:
            strcpy(rcvdPacket.deviceID, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 1:
            strcpy(rcvdPacket.latitude, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 2:
            strcpy(rcvdPacket.longitude, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 3:
            strcpy(rcvdPacket.HBatt, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 4:
            strcpy(rcvdPacket.SIV, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 5:
            strcpy(rcvdPacket.hdop, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 6:
            strcpy(rcvdPacket.powerMode, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 7:
            strcpy(rcvdPacket.lightStatus, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 8:
            strcpy(rcvdPacket.colorChoice, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 9:
            strcpy(rcvdPacket.chour, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 10:
            strcpy(rcvdPacket.cmin, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 11:
            strcpy(rcvdPacket.csec, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 12:
            strcpy(rcvdPacket.repeatBatt, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 13:
            strcpy(rcvdPacket.repeatRSSI, token);
            token = strtok(NULL, " ");
            count += 1;
            break;
        case 14:
            token = NULL;
            break;
        }
    }
    // String test = "";
    // for (int i = 0; i < 12; i++)
    // {
    //     test += rcvdPacket.latitude[i];
    // }
}

void LoadPacket(int packetSize)
{
    packet = "";
    int battStatus = Batt.GetVBatt();
    String bleRSSI = " " + String(LoRa.packetRssi(), DEC);
    for (int i = 0; i < packetSize; i++)
    {
        packet += (char)LoRa.read();
    }
    packet = packet + " " + (String)battStatus + bleRSSI + " ";
    LoadCharPacket();
    SetPacketData();
}

void SendPacket(char *message)
{
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
}
