#include <ezTime.h>
#include <WiFi.h>

void setup() {

	Serial.begin(115200);
	
	WiFi.begin("your-ssid", "your-password");
	while (!Serial) { ; }		// wait for Serial port to connect. Needed for native USB port only
	waitForSync();

	Serial.println();
	Serial.println("Time in various internet standard formats ...");
	Serial.println();
	Serial.println("ATOM:        " + dateTime(ATOM));	
	Serial.println("COOKIE:      " + dateTime(COOKIE));
	Serial.println("IS8601:      " + dateTime(ISO8601));
	Serial.println("RFC822:      " + dateTime(RFC822));
	Serial.println("RFC850:      " + dateTime(RFC850));
	Serial.println("RFC1036:     " + dateTime(RFC1036));
	Serial.println("RFC1123:     " + dateTime(RFC1123));
	Serial.println("RFC2822:     " + dateTime(RFC2822));
	Serial.println("RFC3339:     " + dateTime(RFC3339));
	Serial.println("RFC3339_EXT: " + dateTime(RFC3339_EXT));
	Serial.println("RSS:         " + dateTime(RSS));
	Serial.println("W3C:         " + dateTime(W3C));
	Serial.println();
	Serial.println(" ... and any other format, like \"" + dateTime("l ~t~h~e jS ~o~f F Y, g:i A") + "\"");
}

void loop() {
	events();
}
