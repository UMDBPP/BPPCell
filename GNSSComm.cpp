#include "Arduino.h"
#include "BPPCell.h"
#include <I2C.h>
#include "stdlib.h"

/**
 * Initializes the object and attempts to set the GNSS Flight mode to FLIGHT_MODE.	
 */
GNSSComm::GNSSComm() {
	
	 _MU_LOWERCASE = 0xB5;
	 _B_LOWERCASE = 0x62;
	 _DOLLAR_SIGN = 0x24;
	 _G_UPPERCASE = 0x47;
	 _P_UPPERCASE = 0x50;
	_BUFFER_CHAR = char(BUFFER_CHAR_VALUE);
	_NULL_CHAR = char(NULL_CHAR_VALUE);
	_NEWLINE = '\n';
	I2c.begin();
}

/* Gets the $GPGGA message from the GPS module
 * Note: this can be a time-intensive (>1 sec) function, as it requires the GPS module to cycle through an entire message
 */
String GNSSComm::getGGAString() {
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

String GNSSComm::getNextLine()
{
	String returnString = "";
	consumeCurrentLine();
	
	return returnString;
}

int GNSSComm::sendMessageToGNSS(byte* msg, int msgLength)
{
  I2c.begin();
  I2c.write(GNSS_ADDRESS, GNSS_REGISTER, 0XFF); // wakes GNSS
  delay(100);
  int bytesSent = I2c.write(GNSS_ADDRESS, GNSS_REGISTER, msg, msgLength);
  I2c.end();
  return bytesSent;
}

char GNSSComm::readOneCharFromI2C()
{
	if(I2c.available() == 0)
	{
		I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
	}
	return (char) I2c.receive();
}

String GNSSComm::readFromI2C(int bytes)
{
	String s = "";
	while(bytes > 0)
	{
		while(I2c.available()) { 
			byte b = I2c.receive();
			if(b != NULL_CHAR_VALUE) // TODO figure out why nulls are an issue
			{	char c = (char) b;
				s += c;
				bytes--;
				//DEBUG_SERIAL.print(c);
			}
		}
		
		if(I2c.available() == 0)
		{
			I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
		}		
	}
	return s;
}

String GNSSComm::readFromI2CPretty(int bytes)
{
	
	String s = "";
	while(bytes > 0)
	{
		while(I2c.available()) {
			byte b = I2c.receive();
			if(b != BUFFER_CHAR_VALUE) // Ensures that the 'no data available' byte (0xFF) is not written
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
char GNSSComm::consumeBuffer() {
	char c = char(0x00);
	do {
		c = readOneCharFromI2C();
	} while((c == _BUFFER_CHAR) || (c == _NULL_CHAR)); // TODO nulls
	return c;
}

void GNSSComm::appendChecksum(byte* msg, int msgLength)
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

void GNSSComm::consumeCurrentLine()
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

String GNSSComm::getMessage(int timeout) {
	I2c.begin();
	long startTime = millis();
	byte b1 = 0;
	byte b2 = 0;
	while((millis() - startTime) < timeout) {
		if(I2c.available() > 0) { // If bytes are available
			b1 = b2;
			b2 = I2c.receive();
			if((b1 == _MU_LOWERCASE) && (b2 == _B_LOWERCASE)) { // Check if it is a proprietary UBX message (0xB5 0x62)
				I2c.end();
				return readUBXMessageFromI2C(timeout);
			}
			else if ((b1 == _DOLLAR_SIGN) && (b2 == _G_UPPERCASE)) { // Check if it is a GPS NMEA message ($G)
				I2c.end();
				return readNMEAMessageFromI2C(_G_UPPERCASE, timeout); 
			}
			else if((b1 == _DOLLAR_SIGN) && (b2 == _P_UPPERCASE)) { // Check if it is a properiatry U-blox NMEA message ($P)
				I2c.end();
				return readNMEAMessageFromI2C(_P_UPPERCASE, timeout);
			}
		}
		else { // Wait for a byte to become available
			delay(50);
			I2c.read(GNSS_ADDRESS, GNSS_REGISTER, DEFAULT_BYTES_TO_READ); 
		}
	}
	I2c.end();
	return "No message."; // If the timeout is reached 
}

/*
 * Reads one UBX message from the I2C bus and returns it as a String.
 * Assumes the first two characters (0xB5 0x62) have already been consumed from the bus.
 * Timeout is in milliseconds.
 */
String GNSSComm::readUBXMessageFromI2C(int timeout) {
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
	
	int payloadLength = 256*header[5] + header[4] + 2; // Gets the length of the payload by checking the length bytes, which are in Little Endian order; 2 extra bytes for checksum
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
String GNSSComm::readNMEAMessageFromI2C(byte messageTypeId, int timeout) {
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

/* Configures the flight mode of the uBlox GNSS
 * Returns 0 for successful completion, 1 for unsuccessful completion; times out after 1 second.
 * Flight mode parameter: 3 for standard, 6 for high-altitude. Refer to uBlox documentation (UBX-CFG-NAV5) for further information.
 */
bool GNSSComm::configUbloxGNSSFlightMode(byte mode) {
	int maxValidMode = 8; // Valid modes are 0-8
	if(mode > maxValidMode) { // Mode is invalid
		return 1;
	}
	byte msg[] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, // Message header - NAV5
				0xFF, 0xFF, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, // Mask, dynamic platform mode (controlled by mode parameter), auto 2D-3D
				0x16, 0x2C, 0x00, 0x00, 0x05, 0x00, 0xA3, 0x00, // Defualt
				0xA3, 0x00, 0x64, 0x00, 0x27, 0x01, 0x00, 0x3C, // Default
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Default, reserved2 and reserved3
				0x00, 0x00, 0x00, 0x00, // Default, reserved4
				0x00, 0x00 }; // For the checksum
	int msgLength = 44; // Length of the message
	int indexOfMode = 8; // Index of the mode in the message
	msg[indexOfMode] = mode;
	appendChecksum(msg, msgLength); // Set the checksum of the message
	sendMessageToGNSS(msg, msgLength); // Send the message to the GNSS
	
	String expectedResponse = "B5 62 5 1 2 0 6 24 32 5B "; // The ACK response, in String format
	
	int timeout = 1000; // 1 second timeout
	int startTime = millis();
	
	// See if the GNSS acknowledges the configuration message
	while((millis() - startTime) <= timeout) {
		String response = getMessage(timeout/4); // Gets a message. Providing a smaller timeout allows for the possibility that the GNSS might transmit something between receiving the command and sending the ACK
		if(response.indexOf(expectedResponse) > -1) { // ACK received; mesage was sent and received
			return 0;
		}
	}
	return 1; // No ACK was received
}

/**
 * Gets the current flight mode setting of the Ublox GNSS receiver
 * See Ublox GNSS documentation for the CFG-NAV5 message for return code definitions
 * Returns -1 if unable to get response from GNSS unit
 */
int GNSSComm::getCurrentFlightMode() {
	byte msg[] = {0xB5, 0x62, 0x06, 0x24, 0x00, 0x00, 0X2A, 0x84}; // Poll request for CFG-NAV5
	int msgLength = 8;
	sendMessageToGNSS(msg, msgLength); // Send the message to the GNSS

	String expectedResponseHeader = "B5 62 6 24 24 0"; // The header of the CFG-NAV5 message; the program tries to match against this
	
	int timeout = 1500; // 1 second timeout
	int startTime = millis();
	
	// Wait for the GNSS to respond to the CFG-NAV5 request
	while((millis() - startTime) <= timeout) {
		String response = getMessage(timeout/4); // Gets a message. Providing a smaller timeout allows for the possibility that the GNSS might transmit something between receiving the command and sending the response
		if(response.indexOf(expectedResponseHeader) > -1) { // This is the poll response
			byte buf[1] = {0 };
			//getMessageBytesFromString(response, buf, BYTE_OF_FLIGHT_MODE_IN_UBX_CFG_NAV5, BYTE_OF_FLIGHT_MODE_IN_UBX_CFG_NAV5 + 1); // Populate the buffer with the flight mode byte
			getMessageBytesFromString(response, buf, 8, 8 + 1); // Populate the buffer with the flight mode byte
			return buf[0];
		}
	}
	return -1; // Error code
}

/**
 * Gets a subset of the bytes of a UBX message from the space-separated String representation thereof.
 * The subset of bytes returned is from startByteIndex, inclusive, to stopByteIndex, exclusive. These indices are zero-indexed and refer to the
 * space-seperated bytes in the String, not character indices within the String.
 * The bytes are populated in buf, starting at buf[0]. The length of buf must be at least (stopByteIndex - stopByteIndex); violating this condition may result in a buffer overrun.
 * 
 */
void GNSSComm::getMessageBytesFromString(String msg, byte* buf, int startByteIndex, int stopByteIndex) {
	int currentStringIndex = 0; // Index in the String
	int currentByteIndex = 0; // Index of bytes in the message
	int currentArrayIndex = 0; // Index in buffer
	int msgLength = msg.length();
	while((currentByteIndex < stopByteIndex) && (currentStringIndex < msgLength)) {
		int indexOfNextSpace = msg.indexOf(' ', currentStringIndex);
		String currentSubstring = msg.substring(currentStringIndex, indexOfNextSpace);
		currentSubstring.trim(); // Removes any whitespace
		
		if(currentByteIndex >= startByteIndex)
		{
			// Character array for string-to-int conversion
			int charBufLength = 5; // Shouldn't be more than 2 characters in the string, so 5 should be more than sufficient
			char charBuf[charBufLength];
			currentSubstring.toCharArray(charBuf, charBufLength);
			buf[currentArrayIndex] = strtol(charBuf, NULL, 16); // Converts the base-16 text to a numeric value
			currentArrayIndex++;
		}
		
		currentStringIndex = indexOfNextSpace + 1; // Start after the next space
		currentByteIndex++;
	}
}
