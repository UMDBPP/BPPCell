UMD Balloon Payload Project GPS/Cellular Tracking Library (BPPCell)
Developed for the Space Systems Laboratory at the University of Maryland
See GitHub.com/UMDBPP/BPPCell for further details.

This library was developed to facilitate tracking of high-altitude balloon payloads for the University of Maryland's Space systems laboratory.
Supported hardware is an Arduino Mega 2560 and an Embedded Artists Cellular and Positioning Shield with ublox MAX-7Q GPS and SARA-G350 cellular module.
Optionally, an Adafruit SD logger can be included to enable logging capabilities.
Other hardware configurations are not supported, but, under the terms of the license, you are free to modify the software as you see fit.
Use of an Arduino Uno or similar rather than a Mega is not recommended for due to memory constraints.
The included example sketch illustrates how to use the library to send a text with coordinates every 5 minutes while also logging all GPS packets.
It is fully functional; the only modification needed to before running it is entering a 9-digit cell phone number as the number string on line 12.

The library author can be contacted through GitHub with any questions. Usage notes, suggestions for improvement, and bug reports are greatly appreciated.