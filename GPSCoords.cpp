/* GPS Coordinate Parser for Arduino
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

GPSCoords::GPSCoords(String time, long lat, long lon, float alt) {
	_time = time;
	_lat = lat; //Stored in ten-thousandths of a minute (minute * 10^-4)
	_lon = lon; //Stored in ten-thousandths of a minute (minute * 10^-4)
	_alt = alt; // Stored in meters above mean sea level
}

void GPSCoords::setTime(String time) {
	_time = time;
}

// Sets the latitude; unit is ten-thousandths of a minute (minute * 10^-4)
void GPSCoords::setLat(long lat) {
	_lat = lat;
}

// Sets the longitude; unit is ten-thousandths of a minute (minute * 10^-4)
void GPSCoords::setLon(long lon) {
	_lon = lon;
}

// Sets the altitude; unit is meters
void GPSCoords::setAlt(float alt) {
	_alt = alt;
}

String GPSCoords::getTime() {
	return _time;
}

// Gets the latitude; unit is ten-thousandths of a minute (minute * 10^-4)
long GPSCoords::getLat() {
	return _lat;
}

// Gets the longitude; unit is ten-thousandths of a minute (minute * 10^-4)
long GPSCoords::getLon() {
	return _lon;
}

// Gets the altitude; unit is meters
float GPSCoords::getAlt() {
	return _alt;
}

// Formats the coordinates for text according to one of the FORMAT constants
String GPSCoords::formatCoordsForText(int format) {
	String returnString = "";
	switch (format) {
		case FORMAT_DMS: { // Degrees, minutes, and seconds; multiple lines
			DMSCoords coords = getLatLonInDMS();
			returnString += ("Time: " + getFormattedTimeString() + " UTC\n");
			returnString += "Lat: ";
			returnString += coords.latDegs;
			returnString += char(0xB0);
			returnString += " ";
			returnString += coords.latMins;
			returnString += "' ";
			returnString += coords.latSecs;
			returnString += "\" ";
			if(coords.isNorth) 
				returnString += "N ";
			else
				returnString += "S ";
			returnString += "\n";
			returnString += "Lon: ";
			returnString += coords.lonDegs;
			returnString += char(0xB0);
			returnString += " ";
			returnString += coords.lonMins;
			returnString += "' ";
			returnString += coords.lonSecs;
			returnString += "\" ";
			if(coords.isEast) 
				returnString += "E ";
			else
				returnString += "W ";
			returnString += "\n";
			returnString += (String("Alt: ") + getAlt() + "m MSL");
			break;
		}
		case FORMAT_DMS_ONELINE: {
			DMSCoords coords = getLatLonInDMS();
			returnString += ("Time: " + getFormattedTimeString() + " UTC ");
			returnString += "Lat: ";
			returnString += coords.latDegs;
			returnString += char(0xB0);
			returnString += " ";
			returnString += coords.latMins;
			returnString += "' ";
			returnString += coords.latSecs;
			returnString += "\" ";
			if(coords.isNorth) 
				returnString += "N ";
			else
				returnString += "S ";
			returnString += "Lon: ";
			returnString += coords.lonDegs;
			returnString += char(0xB0);
			returnString += " ";
			returnString += coords.lonMins; 
			returnString += "' ";
			returnString += coords.lonSecs;
			returnString += "\" ";
			if(coords.isEast) 
				returnString += "E ";
			else
				returnString += "W ";
			returnString += (String("Alt: ") + getAlt() + "m MSL");
			break;
		}
		case FORMAT_DMS_CSV: {
			DMSCoords coords = getLatLonInDMS();
			returnString += getFormattedTimeString();
			returnString += ",";
			if(!coords.isNorth) 
				returnString += "-";
			returnString += coords.latDegs;
			returnString += ",";
			returnString += coords.latMins;
			returnString += ",";
			returnString += coords.latSecs;
			returnString += ",";

			if(!coords.isEast) 
				returnString += "-";
			returnString += coords.lonDegs;
			returnString += ",";
			returnString += coords.lonMins;
			returnString += ",";
			returnString += coords.lonSecs;
			returnString += ",";

			returnString += getAlt();
			break;
		}
		case FORMAT_DEC_DEGS: {
			DecDegsCoords coords = getLatLonInDecDegs();
			returnString += ("Time: " + getFormattedTimeString() + " UTC \n");
			returnString += "Lat: ";
			returnString += getDecDegsLatString(coords);
			returnString += char(0xB0);
			returnString += "\n";
			returnString += "Lon: ";
			returnString += getDecDegsLonString(coords);
			returnString += char(0xB0);
			returnString += "\n";
			returnString += "Alt: ";
			returnString += getAlt();
			returnString += "m MSL";
			break;
		}
		case FORMAT_DEC_DEGS_CSV: {
			DecDegsCoords coords = getLatLonInDecDegs();
			returnString += (getFormattedTimeString() + ",");
			returnString += getDecDegsLatString(coords);
			returnString += ",";
			returnString += getDecDegsLonString(coords);
			returnString += ",";
			returnString += getAlt();
			break;
		}
	}
	return returnString;
}


// Gets the coordinates stored by this GPSCoords object in a DMSCoords struct, which gives degree-minute-second formatting
DMSCoords GPSCoords::getLatLonInDMS(void) {
	DMSCoords coords;
	long localLat = abs(_lat);
	long localLon = abs(_lon);
	
	int latDegs = (int) (localLat/(MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE));
	localLat = localLat % (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE);
	coords.latDegs = latDegs;
	int lonDegs = (int) (localLon/(MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE));
	localLon = localLon % (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE);
	coords.lonDegs = lonDegs;
	
	int latMins = (int) (localLat/TEN_THOUSANDTHS_PER_MINUTE);
	localLat = localLat % (TEN_THOUSANDTHS_PER_MINUTE);
	coords.latMins = latMins;
	int lonMins = (int) (localLon/TEN_THOUSANDTHS_PER_MINUTE);
	localLon = localLon % (TEN_THOUSANDTHS_PER_MINUTE);
	coords.lonMins = lonMins;
	
	float latSecs = ((float) (localLat*SECONDS_PER_MINUTE)/TEN_THOUSANDTHS_PER_MINUTE);
	coords.latSecs = latSecs;
	float lonSecs = ((float) (localLon*SECONDS_PER_MINUTE)/TEN_THOUSANDTHS_PER_MINUTE);
	coords.lonSecs = lonSecs;
	if(_lat >= 0)
		coords.isNorth = true;
	else
		coords.isNorth = false;
	
	if(_lon >= 0)
		coords.isEast = true;
	else
		coords.isEast = false;
	
	return coords;
}

/* Gets the latitude and longitude of this GPSCoords object a DecDegsCoords struct, which gives decimal degree formatting
 * North latitude and east longitude are positive.
 */
DecDegsCoords GPSCoords::getLatLonInDecDegs(void) {
	DecDegsCoords coords; 
	coords.latChar = _lat / (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE); // Characteristic is whole number of degrees

	coords.latMant = (abs(_lat) % (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE))
		/ ((float) (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE)); // Mantissa is remainder
	coords.lonChar = _lon / (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE); // Characteristic is whole number of degrees
	coords.lonMant = (abs(_lon) % (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE))
		/ ((float) (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE)); // Mantissa is remainder
	return coords;
}

// Gets a time string with punctuation based on this GPSCoords object's time
// Assumes this GPSCoords object's time is in the format of a $GPGGA string (see UBlox documentation for futher details)
String GPSCoords::getFormattedTimeString() {
	String returnString = "";
	returnString += (_time.substring(0, 2) + ":" + _time.substring(2,4) + ":" + _time.substring(4));
	return returnString;	
}

// Gets the string representation of the latitude in decimal degrees
String GPSCoords::getDecDegsLatString() {
	DecDegsCoords coords = getLatLonInDecDegs();
	return getDecDegsLatString(coords);
}

// Gets the string representation of the longitude in decimal degrees
String GPSCoords::getDecDegsLonString() {
	DecDegsCoords coords = getLatLonInDecDegs();
	return getDecDegsLonString(coords);
}

// Gets the string representation of the latitude in decimal degrees
// Input is the DecDegs struct representing the coordinates to convert
String GPSCoords::getDecDegsLatString(DecDegsCoords coords) {
	String decLatString = "";
	decLatString += coords.latChar; // Append the characteristic
	
	const int mantissaDisplayLength = 7; // Number of digits of the mantissa to display
	const int mantissaStrArrLength = 9; // Must have a place for the leading zero and decimal point
	char latMantStrArray[mantissaStrArrLength];
	dtostrf(coords.latMant, mantissaStrArrLength, mantissaDisplayLength, latMantStrArray); // Convert mantissa to a char array
	
	String latMantString = "";
	latMantString += latMantStrArray; // Convert the char array to a string
	
	decLatString += latMantString.substring(1); // Drop the leading zero of the mantissa and append it to the string
	return decLatString;
}

// Gets the string representation of the longitude in decimal degrees
// Input is the DecDegs struct representing the coordinates to convert
String GPSCoords::getDecDegsLonString(DecDegsCoords coords) {
	String decLonString = "";
	decLonString += coords.lonChar; // Append the characteristic
	
	const int mantissaDisplayLength = 7; // Number of digits of the mantissa to display
	const int mantissaStrArrLength = 9; // Must have a place for the leading zero and decimal point
	char lonMantStrArray[mantissaStrArrLength];
	dtostrf(coords.lonMant, mantissaStrArrLength, mantissaDisplayLength, lonMantStrArray); // Convert mantissa to a char array
	
	String lonMantString = "";
	lonMantString += lonMantStrArray; // Convert the char array to a string
	
	decLonString += lonMantString.substring(1); // Drop the leading zero of the mantissa and append it to the string
	return decLonString;
}
