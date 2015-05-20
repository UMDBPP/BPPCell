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
	_lat = lat;
	_lon = lon;
	_alt = alt;
}

void GPSCoords::setTime(String time) {
	_time = time;
}

void GPSCoords::setLat(long lat) {
	_lat = lat;
}

void GPSCoords::setLon(long lon) {
	_lon = lon;
}

void GPSCoords::setAlt(float alt) {
	_alt = alt;
}

String GPSCoords::getTime() {
	return _time;
}

long GPSCoords::getLat() {
	return _lat;
}

long GPSCoords::getLon() {
	return _lon;
}

float GPSCoords::getAlt() {
	return _alt;
}

String GPSCoords::formatCoordsForText(int format) {
	String returnString = "";
	switch (format) {
		case FORMAT_DMS: {
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
				returnString += "N";
			else
				returnString += "S";
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
				returnString += "E";
			else
				returnString += "W";
			returnString += "\n";
			returnString += (String("Alt: ") + getAlt() + "m MSL\n");
			break;
		}
		case FORMAT_DMS_ONELINE: {
			DMSCoords coords = getLatLonInDMS();
			returnString += ("Time: " + getFormattedTimeString() + " UTC");
			returnString += "Lat: ";
			returnString += coords.latDegs;
			returnString += char(0xB0);
			returnString += " ";
			returnString += coords.latMins;
			returnString += "' ";
			returnString += coords.latSecs;
			returnString += "\" ";
			if(coords.isNorth) 
				returnString += "N";
			else
				returnString += "S";
			returnString += "Lon: ";
			returnString += coords.lonDegs;
			returnString += char(0xB0);
			returnString += " ";
			returnString += coords.lonMins; 
			returnString += "' ";
			returnString += coords.lonSecs;
			returnString += "\" ";
			if(coords.isEast) 
				returnString += "E";
			else
				returnString += "W";
			returnString += (String("Alt: ") + getAlt() + "m MSL");
			break;
		}
		case FORMAT_DEC_DEGS: {
			DecDegsCoords coords = getLatLonInDecDegs();
			String decLatString = "";
			String decLonString = "";
			const int lengthOfDecLatString = 9;
			const int lengthOfDecLonString = 10;
			char decLatStrArray[lengthOfDecLatString];
			char decLonStrArray[lengthOfDecLonString];
			dtostrf(coords.lat,lengthOfDecLatString,6, decLatStrArray);
			dtostrf(coords.lon,lengthOfDecLonString,6, decLonStrArray);
			decLatString += decLatStrArray;
			decLonString += decLonStrArray;
			returnString += ("Time: " + getFormattedTimeString() + " UTC \n");
			returnString += "Lat: ";
			returnString += decLatString;
			returnString += char(0xB0);
			returnString += "\n";
			returnString += "Lon: ";
			returnString += decLonString;
			returnString += char(0xB0);
			returnString += "\n";
			returnString += "Alt: ";
			returnString += getAlt();
			returnString += "m MSL\n";
			break;
		}
		
		case FORMAT_DEC_DEGS_CSV: {
			DecDegsCoords coords = getLatLonInDecDegs();
			String decLatString = "";
			String decLonString = "";
			const int lengthOfDecLatString = 9;
			const int lengthOfDecLonString = 10;
			char decLatStrArray[lengthOfDecLatString];
			char decLonStrArray[lengthOfDecLonString];
			dtostrf(coords.lat,lengthOfDecLatString,6, decLatStrArray);
			dtostrf(coords.lon,lengthOfDecLonString,6, decLonStrArray);
			decLatString += decLatStrArray;
			decLonString += decLonStrArray;
			returnString += (getFormattedTimeString() + ",");
			returnString += decLatString;
			returnString += ",";
			returnString += decLonString;
			returnString += ",";
			returnString += getAlt();
			break;
		}
		
		case FORMAT_DMS_CSV: {
			DMSCoords coords = getLatLonInDMS();
			if(!coords.isNorth) 
				returnString += "-";
			returnString += getFormattedTimeString();
			returnString += ",";
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

			returnString += (getAlt());
			break;
		}
	}
	// Uncomment the line below to enable debug output
	//Serial3.println(returnString);
	return returnString;
}


/* Updates the DMSArray to the current coordinate values in degrees, minutes, and seconds
 * DMSArray has six fields. In order: lat degs, lat mins, lat secs, lon degs, lon mins, lon secs
 */
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

/* Gets the latitude and longitude in decimal degrees.
 * WARNING: The Arduino is limited to 8-bit precision when working with floating point numbers. This may result in a loss of precision in the coordinates.
 * Input is an array of floating point numbers containing at least two elements. The method will not function properly if the input array does not have at least two elements.
 * Method populates the first and second elements of the input array with the latitude and longitude, respectively, of the coordinates.
 */
DecDegsCoords GPSCoords::getLatLonInDecDegs(void) {
	DecDegsCoords coords;
	float decLat = _lat/((float) MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE);
	float decLon = _lon/((float) MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE);
	coords.lat = decLat;
	coords.lon = decLon;
	return coords;
}

String GPSCoords::getFormattedTimeString() {
	String returnString = "";
	returnString += (_time.substring(0, 2) + ":" + _time.substring(2,4) + ":" + _time.substring(4));
	return returnString;	
}
