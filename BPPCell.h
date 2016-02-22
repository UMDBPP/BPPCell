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
#include <Wire.h>

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

struct DecDegsCoords {
	float lat;
	float lon;
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
		const int MINUTES_PER_DEGREE = 60;
		const long TEN_THOUSANDTHS_PER_MINUTE = 10000;
		const int SECONDS_PER_MINUTE = 60;
		String formatCoordsForText(int format);
		String getFormattedTimeString();
		DMSCoords getLatLonInDMS();
		DecDegsCoords getLatLonInDecDegs();
		/*
		// Describe the structure for degree-minute-second format
		const int DMS_LAT_DEG_INDEX = 0;
		const int DMS_LAT_MIN_INDEX = 1;
		const int DMS_LAT_SEC_INDEX = 2;
		const int DMS_LON_DEG_INDEX = 3;
		const int DMS_LON_MIN_INDEX = 4;
		const int DMS_LON_SEC_INDEX = 5;
		const int LENGTH_OF_DMS_ARRAY = 6;
		
		// Describe the structure of decimal degree format
		const int DEC_DEGS_LAT_INDEX = 0;
		const int DEC_DEGS_LON_INDEX = 1;
		*/
		
		// Specify the various formats
		const static int FORMAT_DMS = 1;
		const static int FORMAT_DMS_ONELINE = 2;
		const static int FORMAT_DMS_CSV = 3;
		// NOTE: Decimal degree format is NOT SUPPORTED as of this version (1.0/May 2015)
		const static int FORMAT_DEC_DEGS = 4;
		const static int FORMAT_DEC_DEGS_CSV = 5;
	
	private:
		String _time;
		long _lat; //Stored in ten-thousandths of a minute (minute * 10^-4)
		long _lon; //Stored in ten-thousandths of a minute (minute * 10^-4)
		float _alt;
};

class NMEAParser {
	public:
		NMEAParser();
		GPSCoords parseCoords(String GGAString);
		String getGGAString();
		String getNextLine();
		int sendMessageToGNSS(byte address, byte* msg, int msgSize);
		
	//private:
		int _GNSS_ADDRESS;
		int _DEFAULT_BYTES_TO_READ;
		char _BUFFER_CHAR;
		char _NEWLINE;
		 byte _MU_LOWERCASE;
		 byte _B_LOWERCASE;
		 byte _DOLLAR_SIGN;
		 byte _G_UPPERCASE;
		 byte _P_UPPERCASE;
		char readOneCharFromWire();
		String readFromWire(int bytes);
		String readFromWirePretty(int bytes);
		char consumeBuffer();
		long parseLatFromGGA(String latString, bool isNorth);
		long parseLonFromGGA(String lonString, bool isEast);
		byte calcChecksum(byte* msg, int msgLength);
		void appendChecksum(byte* msg, int msgLength);
		void consumeCurrentLine();
		String getMessage(int timeout);
		String readUBXMessageFromWire(int timeout);
		String readNMEAMessageFromWire(byte messageTypeId, int timeout);
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
		void readSerial();
};
#endif