#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <BPPCell.h>

NMEAParser parser;
CellComm comm;

unsigned long lastMillisOfMessage = 0;
bool sendingMessages = true;
long timeInterval = 300000; // In milliseconds; 300000 is 5 minutes
const String number = ""; // put your cell number here, eg. number = "8001234567";

void setup() {
    Serial3.begin(9600); // Debug interface
    comm.setup(); // Sets up the SARA-G350
    String ss = parser.getGGAString(); // Gets the current gps coodinates
    GPSCoords coords = parser.parseCoords(ss);
    String s = coords.formatCoordsForText(2);
    comm.sendMessage(number, s);
    const int chipSelect = 4; // pPn for SPI
    SD.begin(chipSelect); // 
}

void loop() {
    Serial3.println("\n");
    String ggaString = parser.getGGAString();
    GPSCoords coords = parser.parseCoords(ggaString);
    String coordsString = coords.formatCoordsForText(3);
    int CSQ = comm.getCSQ();

    Serial3.println(coordsString);

    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    if (dataFile) {
        String logString = "";
        logString += coordsString;
        logString += ",";
        logString += CSQ;
        dataFile.println(logString);
    }
    else {
        Serial3.println("error opening datalog.txt");
    }  
    Serial3.print("CSQ: ");
    Serial3.println(CSQ);
    
       
    coordsString = coords.formatCoordsForText(2);
    
    if((CSQ > 0 && (millis() - lastMillisOfMessage) > timeInterval)) {
        comm.sendMessage(number, coordsString);
        lastMillisOfMessage = millis();
    }
    dataFile.close();
    Serial3.println("\n");
}
