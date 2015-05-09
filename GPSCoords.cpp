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
			String DMSArray[] = {"", "", "", "", "", ""};
			getLatLonInDMS(DMSArray);
			returnString += ("Time: " + getFormattedTimeString() + " UTC\n");
			returnString += ("Lat: " + DMSArray[DMS_LAT_DEG_INDEX] + char(0xB0) + " ");
			returnString += (DMSArray[DMS_LAT_MIN_INDEX] + "' ");
			returnString += (DMSArray[DMS_LAT_SEC_INDEX] + "\" ");
			if(getLat() >= 0) 
				returnString += "N";
			else
				returnString += "S";
			returnString += "\n";
			returnString += ("Lon: " + DMSArray[DMS_LON_DEG_INDEX] + char(0xB0) + " ");
			returnString += (DMSArray[DMS_LON_MIN_INDEX] + "' ");
			returnString += (DMSArray[DMS_LON_SEC_INDEX] + "\" ");
			if(getLon() >= 0) 
				returnString += "E";
			else
				returnString += "W";
			returnString += "\n";
			returnString += (String("Alt: ") + getAlt() + "m MSL\n");
			break;
		}
		case FORMAT_DMS_ONELINE: {
			String DMSArray[] = {"", "", "", "", "", ""};
			getLatLonInDMS(DMSArray);
			returnString += ("Time: " + getFormattedTimeString() + " UTC,");
			returnString += ("Lat: " + DMSArray[DMS_LAT_DEG_INDEX] + char(0xB0) + " ");
			returnString += (DMSArray[DMS_LAT_MIN_INDEX] + "' ");
			returnString += (DMSArray[DMS_LAT_SEC_INDEX] + "\" ");
			if(getLat() >= 0) 
				returnString += "N";
			else
				returnString += "S";
			returnString += ",";
			returnString += ("Lon: " + DMSArray[DMS_LON_DEG_INDEX] + char(0xB0) + " ");
			returnString += (DMSArray[DMS_LON_MIN_INDEX] + "' ");
			returnString += (DMSArray[DMS_LON_SEC_INDEX] + "\" ");
			if(getLon() >= 0) 
				returnString += "E";
			else
				returnString += "W";
			returnString += ",";
			returnString += (String("Alt: ") + getAlt() + "m MSL");
			break;
		}
		case FORMAT_DEC_DEGS: {
			String coordsArray[] = {"                 ", "                    "};
			getLatLonInDecDegs(coordsArray);
			Serial.println("Constructing return string");
			returnString += ("Time: " + getFormattedTimeString() + " UTC\n");
			returnString += ("Lat: " + coordsArray[DEC_DEGS_LAT_INDEX] + char(0xB0) + "\n");
			returnString += ("Lon: " + coordsArray[DEC_DEGS_LON_INDEX] + char(0xB0) + "\n");
			//returnString += (String("Alt: ") + getAlt() + "m MSL\n");
			returnString += "Alt: ";
			returnString += getAlt();
			returnString += "m MSL\n";
			Serial.println("here");
			Serial.println("Finished constructing return string");
			break;
		}
		/*
		case FORMAT_DEC_DEGS_CSV: {
			String coordsArray[] = {"", ""};
			getLatLonInDecDegs(coordsArray);
			//Serial.println("Constructing return string");
			returnString += getFormattedTimeString();
			returnString += ",";
			returnString += coordsArray[DEC_DEGS_LAT_INDEX];
			returnString += ",";
			returnString += coordsArray[DEC_DEGS_LON_INDEX];
			returnString += ",";
			returnString += getAlt();
			returnString += char(0xD);
			returnString += char(0xA);
			//Serial.println("here");
			//Serial.println("Finished constructing return string");
			break;
			break;
		}
		*/
		case FORMAT_DMS_CSV: {
			String DMSArray[] = {"", "", "", "", "", ""};
			getLatLonInDMS(DMSArray);
			if(getLat() <= 0) 
				returnString += "-";
			returnString += (getFormattedTimeString() + ",");
			returnString += (DMSArray[DMS_LAT_DEG_INDEX] + ",");
			returnString += (DMSArray[DMS_LAT_MIN_INDEX] + ",");
			returnString += (DMSArray[DMS_LAT_SEC_INDEX] + ",");

			if(getLon() <= 0) 
				returnString += "-";
			returnString += (DMSArray[DMS_LON_DEG_INDEX] + ",");
			returnString += (DMSArray[DMS_LON_MIN_INDEX] + ",");
			returnString += (DMSArray[DMS_LON_SEC_INDEX] + ",");

			returnString += (getAlt());
			break;
		}
	}
	//Serial3.println(returnString);
	return returnString;
}


/* Updates the DMSArray to the current coordinate values in degrees, minutes, and seconds
 * DMSArray has six fields. In order: lat degs, lat mins, lat secs, lon degs, lon mins, lon secs
 */
void GPSCoords::getLatLonInDMS(String DMSArray[]) {
	long localLat = abs(_lat);
	long localLon = abs(_lon);
	
	int latDegs = (int) (localLat/(MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE));
	localLat = localLat % (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE);
	DMSArray[DMS_LAT_DEG_INDEX] += latDegs;
	int lonDegs = (int) (localLon/(MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE));
	localLon = localLon % (MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE);
	DMSArray[DMS_LON_DEG_INDEX] += lonDegs;
	
	int latMins = (int) (localLat/TEN_THOUSANDTHS_PER_MINUTE);
	localLat = localLat % (TEN_THOUSANDTHS_PER_MINUTE);
	DMSArray[DMS_LAT_MIN_INDEX] += latMins;
	int lonMins = (int) (localLon/TEN_THOUSANDTHS_PER_MINUTE);
	localLon = localLon % (TEN_THOUSANDTHS_PER_MINUTE);
	DMSArray[DMS_LON_MIN_INDEX] += lonMins;
	
	float latSecs = ((float) (localLat*SECONDS_PER_MINUTE)/TEN_THOUSANDTHS_PER_MINUTE);
	DMSArray[DMS_LAT_SEC_INDEX] += latSecs;
	float lonSecs = ((float) (localLon*SECONDS_PER_MINUTE)/TEN_THOUSANDTHS_PER_MINUTE);
	DMSArray[DMS_LON_SEC_INDEX] += lonSecs;
}

/* Gets the latitude and longitude in decimal degrees.
 * WARNING: The Arduino is limited to 8-bit precision when working with floating point numbers. This may result in a loss of precision in the coordinates.
 * Input is an array of floating point numbers containing at least two elements. The method will not function properly if the input array does not have at least two elements.
 * Method populates the first and second elements of the input array with the latitude and longitude, respectively, of the coordinates.
 */
void GPSCoords::getLatLonInDecDegs(String* coordsArray) {
	float decLat = _lat/((float) MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE);
	float decLon = _lon/((float) MINUTES_PER_DEGREE*TEN_THOUSANDTHS_PER_MINUTE);
	String decLatString = "";
	String decLonString = "";
	const int lengthOfDecLatString = 10;
	const int lengthOfDecLonString = 11;
	const char space = char(0x20);
	char decLatStrArray[lengthOfDecLatString];
	char decLonStrArray[lengthOfDecLonString];
	dtostrf(decLat,10,6, decLatStrArray);
	dtostrf(decLon,11,6, decLonStrArray);
	for(int i = 0; i < lengthOfDecLatString; i++) {
		if(decLatStrArray[i] != space) {
			decLatString += decLatStrArray[i];
		}
	}
	for(int i = 0; i < lengthOfDecLonString; i++) {
		if(decLonStrArray[i] != space) {
			decLonString += decLonStrArray[i];
		}
	}
	Serial3.print("DecLatString: ");
	Serial3.println(decLatString);
	coordsArray[0] = decLatString;
	coordsArray[1] = decLonString;
}

String GPSCoords::getFormattedTimeString() {
	String returnString = "";
	returnString += (_time.substring(0, 2) + ":" + _time.substring(2,4) + ":" + _time.substring(4));
	return returnString;	
}
