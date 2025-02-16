#include <main.h>

char buf[8];

void drawbatt(int percent)
{
    Batt.drawBattery(percent, false);
}

void DisplayBattStatus()
{
    sprintf(buf, "%d%%", battStatus);
    Heltec.display->drawString(70, 0, buf);
    drawbatt(battStatus);
}

void DisplayData()
{
    Heltec.display->clear();
    DisplayBattStatus();

    // Heltec.display->drawString(0, 10, harnessPowerMode);

    // Heltec.display->drawString(0, 10, "Received " + packSize + " bytes");
    // Heltec.display->drawStringMaxWidth(0, 20, 128, packet);
    char RSSI[25] = "RSSI: ";
    char Hbatt[25] = "HBATT:          ";
    char SIV[25] = "SIV:               ";
    char Hdop[25] = "HDOP:            ";
    // char Light[25] =  "Light:        ";
    char Time[25] = "Last Update:  ";

    strcat(RSSI, rcvdPacket.repeatRSSI);
    strcat(Hbatt, rcvdPacket.HBatt);
    strcat(SIV, rcvdPacket.SIV);
    strcat(Hdop, rcvdPacket.hdop);
    strcat(Time, rcvdPacket.chour);
    strcat(Time, ":");
    strcat(Time, rcvdPacket.cmin);
    strcat(Time, ":");
    strcat(Time, rcvdPacket.csec);

    Heltec.display->drawString(0, 0, RSSI);

    Heltec.display->drawString(0, 11, Hbatt);
    Heltec.display->drawString(0, 22, SIV);
    Heltec.display->drawString(0, 33, Hdop);
    Heltec.display->drawString(0, 44, Time);
    //Heltec.display->drawStringMaxWidth(0, 11, 128, charPacket);

    Heltec.display->display();
}
