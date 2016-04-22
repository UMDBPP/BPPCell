/* UMD Balloon Payload Project GPS/Cellular Tracking Library (BPPCell)
 * Developed for the Space Systems Laboratory at the University of Maryland
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
 
#ifndef BPPCell_h
#define BPPCell_h

#include <Arduino.h>
//#include <Wire.h>

#define GNSS_ADDRESS 66
#define GNSS_REGISTER 0xFE
#define DEBUG_SERIAL Serial // The serial interface used for debugging
#define DEBUG_SERIAL_BAUD 9600
#define CELL_SERIAL Serial2 // The serial interface to use to communicate with the cell modem
#define CELL_SERIAL_BAUD 115200
#define DEFAULT_BYTES_TO_READ 32 // The most allowed by the Ninjablox I2c library
#define BUFFER_CHAR_VALUE 0xFF // The byte value of the buffer character; in this case, 0xFF, or ÿ
#define NULL_CHAR_VALUE 0x00
#define FLIGHT_MODE 6 // The GNSS should be set to flight mode 6 (Aerospace, <1g). See uBlox documentation for UBX-CFG-NAV5 for further information.
#define DEFAULT_FLIGHT_MODE 3 // The GNSS defaults to this flight mode on reset

#define BYTE_OF_FLIGHT_MODE_IN_UBX_CFG_NAV5 8 // The index of the byte for flight mode within the CFG-NAV5 message, inlcuding headers. See Ublox GNSS documentation for details.

struct DMSCoords {
	int latDegs;
	int latMins;
	float latSecs;
	int lonDegs;
	int lonMins;
	float lonSecs;
	bool isNorth;
	bool isEast;
};

// GPS Coordinates structure in decimal degrees format
struct DecDegsCoords {
	int latChar; // Characteristic (integer part) of the latitude
	float latMant; // Mantissa (decimal part) of the latitude
	int lonChar; // Characteristic (integer part) of the longitude
	float lonMant; // Mantissa (decimal part) of the longitude
};

class GPSCoords {
	public:
		GPSCoords(String time, long lat, long lon, float alt);
		void setTime(String time);
		void setLat(long lat);
		void setLon(long lon);
		void setAlt(float Alt);
		String getTime();
		long getLat();
		long getLon();
		float getAlt();
		const static int MINUTES_PER_DEGREE = 60;
		const static long TEN_THOUSANDTHS_PER_MINUTE = 10000;
		const static int SECONDS_PER_MINUTE = 60;
		const static long TEN_THOUSANDTHS_PER_DEGREE = TEN_THOUSANDTHS_PER_MINUTE * MINUTES_PER_DEGREE;
		String formatCoordsForText(int format);
		String getFormattedTimeString();
		DMSCoords getLatLonInDMS();
		DecDegsCoords getLatLonInDecDegs();
		String getDecDegsLatString(void);
		String getDecDegsLonString(void);
		String getDecDegsLatString(DecDegsCoords);
		String getDecDegsLonString(DecDegsCoords);
		
		// Specify the various formats
		const static int FORMAT_DMS = 1; // Degrees, minutes, and seconds; multiple lines
		const static int FORMAT_DMS_ONELINE = 2; // Degrees, minutes, and seconds; one line
		const static int FORMAT_DMS_CSV = 3; // Degrees, minutes, and seconds; comma-separated on one line
		const static int FORMAT_DEC_DEGS = 4; // Decimal degrees; multiple lines
		const static int FORMAT_DEC_DEGS_CSV = 5; // Decimal degrees; comma-separated on one line
	
	private:
		String _time;
		long _lat; //Stored in ten-thousandths of a minute (minute * 10^-4)
		long _lon; //Stored in ten-thousandths of a minute (minute * 10^-4)
		float _alt; // Stored in meters above mean sea level
};

class NMEAParser {
	public:
		NMEAParser();
		GPSCoords parseCoords(String GGAString);

		
	private:

		long parseLatFromGGA(String latString, bool isNorth);
		long parseLonFromGGA(String lonString, bool isEast);
};

class GNSSComm {
	public:
	GNSSComm();
	String getGGAString();
	String getNextLine();
	int sendMessageToGNSS(byte* msg, int msgSize);
	bool configUbloxGNSSFlightMode(byte mode);
	int getCurrentFlightMode();
	void appendChecksum(byte* msg, int msgLength);
	String getMessage(int timeout);
	void getMessageBytesFromString(String, byte*, int, int);
	
	private:
		int _DEFAULT_BYTES_TO_READ;
		char _BUFFER_CHAR;
		char _NULL_CHAR;
		char _NEWLINE;
		byte _MU_LOWERCASE;
		byte _B_LOWERCASE;
		byte _DOLLAR_SIGN;
		byte _G_UPPERCASE;
		byte _P_UPPERCASE;
		char readOneCharFromI2C();
		String readFromI2C(int bytes);
		String readFromI2CPretty(int bytes);
		char consumeBuffer();
		
		void consumeCurrentLine();
		
		String readUBXMessageFromI2C(int timeout);
		String readNMEAMessageFromI2C(byte messageTypeId, int timeout);
	
};

class CellComm {
	public:
		CellComm();
		void setup();
		void sendMessage(String number, String message);
		int getCSQ();
		int getNumMessages();
		String getMessage(int index);
		bool deleteAllMessages();
		int countOccurences(String stringToSearch, String target, int startingIndex = 0);
		
	private:
		String readSerial();
};
#endif