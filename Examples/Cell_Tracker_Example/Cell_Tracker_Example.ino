#include <SPI.h>
#include <SD.h>
#include <I2C.h>
#include <BPPCell.h>

NMEAParser parser;
CellComm cellComm;
GNSSComm gnssComm;


unsigned long lastMillisOfMessage = 0;
bool sendingMessages = true;
const String number = ""; // put your cell number here, eg. number = "8001234567";
long messageTimeInterval = 300000; // In milliseconds; 300000 is 5 minutes; defines how frequenty the program sends messages
long shutdownTimeInterval = 18000000; // In milliseconds; 18000000 is 5 hours; defines after what period of time the program stops sending messages
long startTime; // The start time of the program

void setup() {
    startTime = millis();
    Serial3.begin(9600); // Debug interface
    cellComm.setup(); // Sets up the SARA-G350
    String ss = gnssComm.getGGAString(); // Gets the current gps coodinates
    GPSCoords coords = parser.parseCoords(ss);
    String s = coords.formatCoordsForText(2);
    cellComm.sendMessage(number, s);
    const int chipSelect = 4; // pPn for SPI
    SD.begin(chipSelect); // 
}

void loop() {
    Serial3.println("\n");
    String ggaString = gnssComm.getGGAString();
    GPSCoords coords = parser.parseCoords(ggaString);
    String coordsString = coords.formatCoordsForText(3);
    int CSQ = cellComm.getCSQ();

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
    
    if((CSQ > 0 && (millis() - lastMillisOfMessage) > messageTimeInterval) && ((millis() - startTime) < shutdownTimeInterval) {
        cellComm.sendMessage(number, coordsString);
        lastMillisOfMessage = millis();
    }
    dataFile.close();
    Serial3.println("\n");
}
