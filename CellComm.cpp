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
	CELL_SERIAL.begin(CELL_SERIAL_BAUD);
	CELL_SERIAL.println("AT+CMGF=1"); // Changes io mode to text (cf. hex)
	CELL_SERIAL.println("Go Terps");
	delay(50);
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
	CELL_SERIAL.println(commandString);
	delay(20);
	readSerial();
	CELL_SERIAL.print(message);
	CELL_SERIAL.println(char(0x1A));
	delay(1000);
	readSerial();
}

/* Gets the number of messages waiting on the cell module.
 */
int CellComm::getNumMessages() {
	String keyString = "+CMGL";
	CELL_SERIAL.println("AT+CMGL");
	//delay(20);
	//CELL_SERIAL.println(char(0x1A));
	delay(1000);
	String prevString = "";
	String currentString = "";
	
	int numMessages = -1; // TODO
	
	/**************/
	long startTime = millis();
	while((millis() - startTime) <= 15000) {
		int b = CELL_SERIAL.read();
		char c = (char) b;
		if (b != -1)
			CELL_SERIAL.print(c);
		
	}
	/************/
	while(CELL_SERIAL.available() > 0) {
		char buffer[64];
		int available = CELL_SERIAL.available();
		CELL_SERIAL.readBytes(buffer, available);
		prevString = currentString;
		currentString = "";
		currentString += buffer;
		
		String searchString = prevString + currentString;
		numMessages += countOccurences(searchString, keyString, 0);
		delay(20);
	}
	return numMessages;
}

/* Gets the message at the given index, if one exists. 
 * Index must be strictly greater than 0 and less than or equal to the number of messages.
 * If the index is invalid, returns the empty string.
 */
String CellComm::getMessage(int index) {
	String command = "AT+CMGR=";
	command += index;
	CELL_SERIAL.println(command);
	delay(100);
	String s = "";
	while(CELL_SERIAL.available() > 0) {
		char buffer[64];
		int available = CELL_SERIAL.available();
		CELL_SERIAL.readBytes(buffer, available);
		s += buffer;
		delay(50);
	}
	
	String errorString = "+CMS ERROR: invalid memory index";
	if(s.indexOf(errorString) < 0)
		return "";
	return s;
}

/* Deletes all received SMS messages from the cell module.
 * Returns true if cell module reports successful execution of method,
 * false otherwise.
 * A true return result does not necessarily indicate that messages were
 * deleted, merely that the cell module processed the command.
 */
bool CellComm::deleteAllMessages() {
	CELL_SERIAL.println("AT+CMGD=1,4");
	delay(5);
	int available = CELL_SERIAL.available();	
	char* buffer = new char[available];
	CELL_SERIAL.readBytes(buffer, available);
	String s = "";
	s += buffer;
	delete[] buffer;
	if(s.indexOf("OK") > -1)
		return true;
	return false;
}

// Consumes the output from the cell module on the Serial interface and returns the output.
String CellComm::readSerial() { 
    String n = "";
    while(CELL_SERIAL.available() > 0) {
      char c = CELL_SERIAL.read();
      n+=c;
      delay(10);
    }
	return n;
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
	CELL_SERIAL.println("AT+CSQ");
	int CSQ = CELL_SERIAL.parseInt();
	return CSQ;
}

/* Counts the number of occurences of target in stringToSearch occuring at or after startingIndex.
 */
int CellComm::countOccurences(String stringToSearch, String target, int startingIndex)
{
	int indexOfKey = stringToSearch.indexOf(target, startingIndex);
	if(indexOfKey < 0) // Base case; target is not in stringToSearch at or after startingIndex
		return 0;
	else // Reduction; return 1 for this occurence plus the number of occurences in the rest of the string
		return (1 + countOccurences(stringToSearch, target, (indexOfKey + 1)));
}