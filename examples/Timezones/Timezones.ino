#include <ezTime.h>
#include <WiFi.h>

void setup() {

	Serial.begin(115200);
	WiFi.begin("your-ssid", "your-password");

	// Uncomment the line below to see what it does behind the scenes
	// ezTime.debug(INFO);
	
	ezTime.waitForSync();

	Serial.println();
	Serial.println("UTC:             " + UTC.dateTime());

	Timezone myTZ;

	// Anything with a slash in it is interpreted as an official timezone name
	// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
	myTZ.setLocation("Pacific/Auckland");
	Serial.println("Auckland:        " + myTZ.dateTime());

	// Anything else is parsed as an address to see if that resolves
	myTZ.setLocation("Paris, Texas");
	Serial.println("Paris, Texas:    " + myTZ.dateTime());

	// The empty string is resolved to the GeoIP location of your IP-address
	myTZ.setLocation(""); 
	Serial.println("Your local time: " + myTZ.dateTime());

}

void loop() {

}
