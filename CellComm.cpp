/* Cellular Communication Library for Arduino and Ublox SARA G350
 * Developed for the Space Systems Laboratory at the University of Maryland
 * Part of the BPPCell library
 * See GitHub.com/UMDBPP/BPPCell or the accompanying readme for further details.
 *
 * Copyright (c) 2015 Luke Renegar
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Arduino.h"
#include "BPPCell.h"
#include <HardwareSerial.h>

CellComm::CellComm() {}

// Call this method after
void CellComm::setup() {
	Serial.begin(115200);
	Serial.println("AT+CMGF=1"); // Changes io mode to text (cf. hex)
	readSerial();
}

/* Sends a SMS message.
 * Input number is the phone number of the recipient.
 * Input message is the message to be sent
 */
void CellComm::sendMessage(String number, String message) {
	String commandString = "AT+CMGS=\"";
	commandString += number;
	commandString +="\"";
	Serial.println(commandString);
	delay(20);
	readSerial();
	Serial.print(message);
	Serial.println(char(0x1A));
	delay(1000);
	readSerial();
}

/* Gets the messages waiting on the cell module.
 * This method may contain bugs. Use caution.
 */
String CellComm::getMessages() {
	Serial.println("AT+CMGL");
	delay(100);
	String s = "";
	char buffer[64];
	while(Serial.available() > 0) {
		int available = Serial.available();
		Serial.readBytes(buffer, available);
		s += buffer;
		delay(100);
	}
	//Serial3.println("Messages are: ");
	//Serial3.println(s);
	return s;
}

/* Deletes all received SMS messages from the cell module.
 * Returns true if cell module reports successful execution of method,
 * false otherwise.
 * A true return result does not necessarily indicate that messages were
 * deleted, merely that the cell module processed the command.
 */
bool CellComm::deleteAllMessages() {
	Serial.println("AT+CMGD=1,4");
	delay(5);
	int available = Serial.available();	
	char* buffer = new char[available];
	Serial.readBytes(buffer, available);
	String s = "";
	s += buffer;
	if(s.indexOf("OK") > -1)
		return true;
	return false;
}

// Consumes the output from the cell module on the Serial interface.
void CellComm::readSerial() { 
    String n = "";
    while(Serial.available() > 0) {
      char c = Serial.read();
      n+=c;
      delay(10);
    }
}

/* Gets the cell signal quality from the cell module
 * Output is the received signal strength indicator (RSSI)
 * 0 -> RSSI <= -113 dBm
 * 1 -> RSSI = -111 dBm
 * 2..30 -> RSSI = -109 dBm to -53 dBm in 2 dBm increments
 * 31 -> RSSI >= -51 dBm
 * 99 -> error
 * See Ublox documentation on 'AT+CSQ' for more details
 */
int CellComm::getCSQ() {
	Serial.println("AT+CSQ");
	int CSQ = Serial.parseInt();
	while(Serial.available() > 0) {
		Serial.read();
		delay(5);
	}
	return CSQ;
}
