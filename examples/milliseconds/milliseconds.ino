#include <ezTime.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

void setup() {

	Serial.begin(115200);
	while (!Serial) { ; }		// wait for Serial port to connect. Needed for native USB port only
	WiFi.begin("your-ssid", "your-password");
	
	setInterval(60);
	waitForSync();

	Serial.println();

	for (int n = 0; n < 10; n++) {
		Serial.println(UTC.dateTime("l, d-M-y H:i:s.v T"));
	}

	Serial.println();
	Serial.println("Those milliseconds between the first and the last line ...");
	Serial.println();
	Serial.println("     ... most of that is spent sending to the serial port.");
	
	Serial.println();
	Serial.println();
	Serial.println();
	Serial.println("And ezTime is not making those milliseconds up either.");
	Serial.println();
	Serial.println("      ... Stick around as we do an NTP request every minute.");
	setDebug(INFO);
	
}

void loop() {
	events();
}
