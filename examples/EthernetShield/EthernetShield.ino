/* 
 *  Note: to use an ethernet shield, You must also set #define EZTIME_ETHERNET in $sketch_dir/libraries/ezTime/src/ezTime.h
 *  
 *  Also note that all ezTime examples can be used with an Ethernet shield if you just replace the beginning of the sketch 
 *  with the beginning of this one.
 */

#include <ezTime.h>

#include <Ethernet.h>

// Enter a MAC address for your controller below. (Or use address below, just make sure it's unique on your network)
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
#define MAC_ADDRESS		{ 0xBA, 0xDB, 0xAD, 0xC0, 0xFF, 0xEE }

void setup() {

	// Open serial communications and wait for port to open:
	Serial.begin(115200);
	while (!Serial) { ; }		// wait for serial port to connect. Needed for native USB port only
	Serial.println();

	// You can use Ethernet.init(pin) to configure the CS pin
	//Ethernet.init(10);  // Most Arduino shields (default if unspecified)
	//Ethernet.init(5);   // MKR ETH shield
	//Ethernet.init(0);   // Teensy 2.0
	//Ethernet.init(20);  // Teensy++ 2.0
	//Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
	//Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet
	
	Serial.print(F("Ethernet connection ... "));
	byte mac [] = MAC_ADDRESS;
	if (Ethernet.begin(mac) == 0) {
		Serial.println(F("failed. (Reset to retry.)"));
		while (true) { ; };			// Hang
	} else {
		Serial.print(F("got DHCP IP: "));
		Serial.println(Ethernet.localIP());
	}
	// give the Ethernet shield a second to initialize:
	delay(1000);

	// OK, we're online...  So the part above here is what you swap in before the waitForSync() in the other examples...


	// Wait for ezTime to get its time synchronized
	waitForSync();

	Serial.println();
	Serial.println("UTC:             " + UTC.dateTime());

	Timezone myTZ;

	// Provide official timezone names
	// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
	myTZ.setLocation(F("Pacific/Auckland"));
	Serial.print(F("New Zealand:     "));
	Serial.println(myTZ.dateTime());

	// Wait a little bit to not trigger DDoS protection on server
	// See https://github.com/ropg/ezTime#timezonedropnl
	delay(5000);

	// Or country codes for countries that do not span multiple timezones
	myTZ.setLocation(F("de"));
	Serial.print(F("Germany:         "));
	Serial.println(myTZ.dateTime());

	// Same as above
	delay(5000);

	// See if local time can be obtained (does not work in countries that span multiple timezones)
	Serial.print(F("Local (GeoIP):   "));
	if (myTZ.setLocation()) {
		Serial.println(myTZ.dateTime());
	} else {
		Serial.println(errorString());
	}

	Serial.println();
	Serial.println(F("Now ezTime will show an NTP sync every 60 seconds"));

	// Set NTP polling interval to 60 seconds. Way too often, but good for demonstration purposes.
	setInterval(60);

	// Make ezTime show us what it is doing
	setDebug(INFO);

}

void loop() {
	
	events();

}
