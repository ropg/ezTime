#include <ezTime.h>
#include <WiFi.h>

void setup() {

	Serial.begin(115200);
	WiFi.begin("your-ssid", "your-password");

	time.waitForSync();

	Serial.println();
	Serial.println("Time in various internet standard formats ...");
	Serial.println();
	Serial.println("ATOM:        " + UTC.dateTime(ATOM));	
	Serial.println("COOKIE:      " + UTC.dateTime(COOKIE));
	Serial.println("IS8601:      " + UTC.dateTime(ISO8601));
	Serial.println("RFC822:      " + UTC.dateTime(RFC822));
	Serial.println("RFC850:      " + UTC.dateTime(RFC850));
	Serial.println("RFC1036:     " + UTC.dateTime(RFC1036));
	Serial.println("RFC1123:     " + UTC.dateTime(RFC1123));
	Serial.println("RFC2822:     " + UTC.dateTime(RFC2822));
	Serial.println("RFC3339:     " + UTC.dateTime(RFC3339));
	Serial.println("RFC3339_EXT: " + UTC.dateTime(RFC3339_EXT));
	Serial.println("RSS:         " + UTC.dateTime(RSS));
	Serial.println("W3C:         " + UTC.dateTime(W3C));
	Serial.println();
	Serial.println(" ... and any other format, like \"" + UTC.dateTime("l ~t~h~e jS ~o~f F Y, g:i A") + "\"");
}

void loop() {

}
