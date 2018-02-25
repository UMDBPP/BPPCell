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

NMEAParser::NMEAParser()
{
}

GPSCoords NMEAParser::parseCoords(String GGAString)
{
    //Start indices are the first characters indices of the given field
    //End indices are the indices of the comma after the given field
    int NUMBER_OF_COMMAS = 14; // Number of commas in a GGA string
    int indicesOfCommas[NUMBER_OF_COMMAS];
    int lastCommaIndex = 0;
    for (int i = 0; i < NUMBER_OF_COMMAS; i++)
    {
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
    String geoString = GGAString.substring(indicesOfCommas[10] + 1, indicesOfCommas[11]); // geoid separation

    bool isNorth;
    if (NSString.equals("N"))
        isNorth = true;
    else
        isNorth = false;

    bool isEast;
    if (EWString.equals("E"))
        isEast = true;
    else
        isEast = false;

    float lat = parseLatFromGGA(latString, isNorth);
    float lon = parseLonFromGGA(lonString, isEast);
    float alt = altString.toFloat() + geoString.toFloat();
    return GPSCoords(time, lat, lon, alt);
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
    int NSMultiplier; //North-south multiplier indicates if the latitude is north or south, with positive values being north
    if (isNorth)
        NSMultiplier = 1;
    else
        NSMultiplier = -1;
    int MINUTES_PER_DEGREE = 60;
    const long TEN_THOUSANDTHS_PER_MINUTE = 10000;
    return NSMultiplier * (degrees * MINUTES_PER_DEGREE * TEN_THOUSANDTHS_PER_MINUTE + minutes * TEN_THOUSANDTHS_PER_MINUTE + ((long) (decMinutes * (TEN_THOUSANDTHS_PER_MINUTE))));
}

long NMEAParser::parseLonFromGGA(String lonString, bool isEast)
{
    int degrees = lonString.substring(0, 3).toInt(); //Gets the degree component of the longitude
    int minutes = lonString.substring(3, 5).toInt(); //Gets the integral minutes component of the longitude
    float decMinutes = lonString.substring(5).toFloat(); //Gets the decimal minutes component of the longitude
    int EWMultiplier; //East-west multiplier indicates if the longitude is east or west, with positive values being east
    if (isEast)
        EWMultiplier = 1;
    else
        EWMultiplier = -1;
    int MINUTES_PER_DEGREE = 60;
    const long TEN_THOUSANDTHS_PER_MINUTE = 10000;
    return EWMultiplier * (degrees * MINUTES_PER_DEGREE * TEN_THOUSANDTHS_PER_MINUTE + minutes * TEN_THOUSANDTHS_PER_MINUTE + ((long) (decMinutes * (TEN_THOUSANDTHS_PER_MINUTE))));
}


