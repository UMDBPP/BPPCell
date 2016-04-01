/* NMEA GPS Coordinate Parser for Arduino and Ublox MAX 7Q
 * Developed for the Space Systems Laboratory at the University of Maryland
 * Part of the BPPCell library
..
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

#include <Arduino.h>
//#include <Wire.h>
#include <BPPCell.h>
#include <I2C.h>

NMEAParser::NMEAParser() {
	 _MU_LOWERCASE = 0xB5;
	 _B_LOWERCASE = 0x62;
	 _DOLLAR_SIGN = 0x24;
	 _G_UPPERCASE = 0x47;
	 _P_UPPERCASE = 0x50;
	_BUFFER_CHAR = char(BUFFER_CHAR_VALUE);
	_NEWLINE = '\n';
	
}

GPSCoords NMEAParser::parseCoords(String GGAString) {
	//Start indices are the first characters indices of the given field
	//End indices are the indices of the comma after the given field
	int NUMBER_OF_COMMAS = 14; // Number of commas in a GGA string
	int indicesOfCommas[NUMBER_OF_COMMAS];
	int lastCommaIndex = 0;
	for(int i = 0; i < NUMBER_OF_COMMAS; i++) {
		indicesOfCommas[i] = GGAString.indexOf(",", lastCommaIndex);
		lastCommaIndex = indicesOfCommas[i] + 1;
	}
	//The field goes from the index immediately following the first comma to the index immediately preceding the second comma
	String time = GGAString.substring(indicesOfCommas[0] + 1, indicesOfCommas[1]);
	String latString = GGAString.substring(indicesOfCommas[1] + 1, indicesOfCommas[2]);
	String NSString = GGAString.substring(indicesOfCommas[2] + 1, indicesOfCommas[3]);
	String lonString = GGAString.substring(indicesOfCommas[3] + 1, indicesOfCommas[4]);
	String EWString = GGAString.substring(indicesOfCommas[4] + 1, indicesOfCommas[5]);
	String altString = GGAString.substring(indicesOfCommas[8] + 1, indicesOfCommas[9]);
	bool isNorth;
	if(NSString.equals("N"))
		isNorth = true;
	else
		isNorth = false;
	bool isEast;
	if(EWString.equals("E"))
		isEast = true;
	else
		isEast = false;
	float lat = parseLatFromGGA(latString, isNorth);
	float lon = parseLonFromGGA(lonString, isEast);
	float alt = altString.toFloat();
	return GPSCoords(time, lat, lon, alt);
}

/* Gets the $GPGGA message from the GPS module
 * Note: this can be a time-intensive (>1 sec) function, as it requires the GPS module to cycle through an entire message
 */
String NMEAParser::getGGAString() {
	String returnString = "";
	String readString = "";
	readString += consumeBuffer(); //Consumes the buffer and gets the first character
	do {
		readString += readFromI2C(128);
	} while(readString.indexOf(_BUFFER_CHAR) == -1); //Ends loop when the next buffer is reached 
	int GGAIndex = readString.indexOf("$GPGGA");
	int endOfGGAIndex = readString.indexOf("$", GGAIndex + 1);
	return readString.substring(GGAIndex, endOfGGAIndex);	
}

String NMEAParser::getNextLine()
{
	String returnString = "";
	consumeCurrentLine();
	
	return returnString;
}

int NMEAParser::sendMessageToGNSS(byte address, byte* msg, int msgLength)
{
  I2c.begin();
  I2c.write(GNSS_ADDRESS, GNSS_REGISTER, 0XFF); // wakes GNSS
  delay(100);
  int bytesSent = I2c.write(GNSS_ADDRESS, GNSS_REGISTER, msg, msgLength);
  I2c.end();
  return bytesSent;
}

/* Parses the latitude from an NMEA GGA string
 *
 *
 */
long NMEAParser::parseLatFromGGA(String latString, bool isNorth)
{
	int degrees = latString.substring(0, 2).toInt(); //Gets the degree component of the latitude
	int minutes = latString.substring(2, 4).toInt(); //Gets the integral minutes component of the latitude
	float decMinutes = latString.substring(4).toFloat(); //Gets the decimal minutes component of the latitude
	int NSMultiplier ; //North-south multiplier indicates if the latitude is north or south, with positive values being north
	if(isNorth)
		NSMultiplier = 1;
	else
		NSMultiplier = -1;
	int MINUTES_PER_DEGREE = 60;
	const long TEN_THOUSANDTHS_PER_MINUTE = 10000;
	return NSMultiplier*(degrees*MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE + minutes*TEN_THOUSANDTHS_PER_MINUTE + ((long) (decMinutes*(TEN_THOUSANDTHS_PER_MINUTE))));
}

long NMEAParser::parseLonFromGGA(String lonString, bool isEast)
{
	int degrees = lonString.substring(0, 3).toInt(); //Gets the degree component of the longitude
	int minutes = lonString.substring(3, 5).toInt(); //Gets the integral minutes component of the longitude
	float decMinutes = lonString.substring(5).toFloat(); //Gets the decimal minutes component of the longitude
	int EWMultiplier ; //East-west multiplier indicates if the longitude is east or west, with positive values being east
	if(isEast)
		EWMultiplier = 1;
	else
		EWMultiplier = -1;
	int MINUTES_PER_DEGREE = 60;
	const long TEN_THOUSANDTHS_PER_MINUTE = 10000;
	return EWMultiplier*(degrees*MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE + minutes*TEN_THOUSANDTHS_PER_MINUTE + ((long) (decMinutes*(TEN_THOUSANDTHS_PER_MINUTE))));
}


char NMEAParser::readOneCharFromI2C()
{
	if(I2c.available() == 0)
	{
		I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
	}
	return (char) I2c.receive();
}

String NMEAParser::readFromI2C(int bytes)
{
	String s = "";
	while(bytes > 0)
	{
		while(I2c.available()) { 
			byte b = I2c.receive();
			char c = (char) b;
			s += c;
			bytes--;
		}
		
		if(I2c.available() == 0)
		{
			I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
		}		
	}
	return s;
}

String NMEAParser::readFromI2CPretty(int bytes)
{
	
	String s = "";
	while(bytes > 0)
	{
		while(I2c.available()) {
			byte b = I2c.receive();
			if(b != 255) // Ensures that the 'no data available' byte (0xFF) is not written
			{
			  char c = (char) b; 
			   s += c;
			}
			bytes--;
		}
		
		if(I2c.available() == 0)
		{
			I2c.read(66, 0xFF, 16);  //TODO Fails here on balloonduino
		}		
	}
	return s;
}
	
/* Consumes the buffer and returns the first non-buffer character */
char NMEAParser::consumeBuffer() {
	char c;
	do {
		c = readOneCharFromI2C();
	} while(c != _BUFFER_CHAR);
	return c;
}

void NMEAParser::appendChecksum(byte* msg, int msgLength)
{

	byte CK_A = 0;
	byte CK_B = 0;
	for(int i = 2; i< (msgLength - 2); i++)
	{
		CK_A = CK_A + msg[i];
		CK_B = CK_B + CK_A;
	}
	
	
	msg[msgLength - 2] = CK_A;
	msg[msgLength - 1] = CK_B;
}

void NMEAParser::consumeCurrentLine()
{
	byte current = 0;
	do {
		if(I2c.available() == 0)
		{
			I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
		}
		current = I2c.receive();
	} while(current != _BUFFER_CHAR );
}

String NMEAParser::getMessage(int timeout) {
	
	long startTime = millis();
	byte b1 = 0;
	byte b2 = 0;
	while((millis() - startTime) < timeout) {
		if(I2c.available() > 0) { // If bytes are available
			b1 = b2;
			b2 = I2c.receive();
			if((b1 == _MU_LOWERCASE) && (b2 == _B_LOWERCASE)) { // Check if it is a proprietary UBX message (0xB5 0x62)
				Serial.println("case 1");
				return readUBXMessageFromI2C(timeout);
			}
			else if ((b1 == _DOLLAR_SIGN) && (b2 == _G_UPPERCASE)) { // Check if it is a GPS NMEA message ($G)
				Serial.println("case 2");
				return readNMEAMessageFromI2C(_G_UPPERCASE, timeout); 
			}
			else if((b1 == _DOLLAR_SIGN) && (b2 == _P_UPPERCASE)) { // Check if it is a properiatry U-blox NMEA message ($P)
				Serial.println("case 3");
				return readNMEAMessageFromI2C(_P_UPPERCASE, timeout);
			}
		}
		else { // Wait for a byte to become available
			delay(50);
			I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
		}
	}
	return "No message."; // If the timeout is reached 
}

/*
 * Reads one UBX message from the I2C bus and returns it as a String.
 * Assumes the first two characters (0xB5 0x62) have already been consumed from the bus.
 * Timeout is in milliseconds.
 */
String NMEAParser::readUBXMessageFromI2C(int timeout) {
	String returnString = "";
	long startTime = millis();
	byte header[] = {0xB5, 0x62, 0x00, 0x00, 0x00, 0x00 }; // First two characters and four blank spaces for the rest of the header
	int headerLength = 6;
	int currentHeaderByteIndex = 2; // index in the header from which content is unknown, since the first two characters have been consumed
	
	// Gets the header of the message
	while(((millis() - startTime) < timeout) && (currentHeaderByteIndex < headerLength)) {

		if(I2c.available() > 0) { // If bytes are available
			header[currentHeaderByteIndex] = I2c.receive(); // Writes one byte to the header
			currentHeaderByteIndex++;
		}
		else { // Wait for a byte to become available
			delay(50);
			I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
		}
	}
	
	// Writes the header to the return string
	for(int i = 0; i < headerLength; i++) {
		String s = String(header[i], HEX);
		s.toUpperCase(); // Changes value stored in s
		returnString += s;
		returnString += ' ';
	}
	
	int payloadLength = 256*header[5] + header[4]; // Gets the length of the payload by checking the length bytes, which are in Little Endian order
	int currentPayloadIndex = 0; // Start at the beginning of the payload
	
	while(((millis() - startTime) < timeout) && (currentPayloadIndex < payloadLength)) {
		if(I2c.available() > 0) { // If bytes are available
			String s = String(I2c.receive(), HEX);
			s.toUpperCase(); // Changes value stored in s
			returnString += s;
			returnString += ' ';
			currentPayloadIndex++;			
		}
		else { // Wait for a byte to become available
			delay(50);
			I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
		}
	}
	return returnString; // In the event of a timeout
	
}


// Message type: G for gps, P for proprietary
String NMEAParser::readNMEAMessageFromI2C(byte messageTypeId, int timeout) {
	String returnString = "";
	long startTime = millis();
	
	byte CR = 0x0D; // Carriage return
	byte LF = 0x0A; // Line feed
	
	byte b1 = _DOLLAR_SIGN;
	byte b2 = messageTypeId;
	
	char catChar = (char) b1; // TODO
	returnString += catChar;
	
	// Read bytes until the end of the message or the timeout is reached
	while((millis() - startTime) < timeout) {
		
		if((b1 == CR) && (b2 == LF)) { // If the end of the message has been reached; write the endline to the string and break
			catChar = (char) b1;
			returnString += catChar;
			catChar = (char) b2;
			returnString += catChar;
			break;
		}

		if(I2c.available() > 0) { // If bytes are available
			b1 = b2;
			if(b1 != CR) {
				catChar = (char) b1;
				returnString += catChar; // TODO verify logic
			}
			b2 = I2c.receive(); // Writes one byte to the header			
		}
		else { // Wait for a byte to become available
			delay(50);
			I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
		}
	}
	//Serial3.println(returnString);
	return returnString;
}


