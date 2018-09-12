/*
 * This sketch prints a message at noon UTC, every second Tuesday of the month. 
 * Not very useful, but demonstrates events and ordinal time.
 */

#include <ezTime.h>
#include <WiFi.h>

void setup() {

	Serial.begin(115200);
	while (!Serial) { ; }		// wait for Serial port to connect. Needed for native USB port only
	WiFi.begin("your-ssid", "your-password");

	waitForSync();

	// Set the event to trigger for the first time
	setEvent( itIsTheSecondTuesday, nextSecondTuesday() );

}

void loop() {

	events();

}

void itIsTheSecondTuesday() {
	Serial.print(F("It's the second Tuesday: "));
	Serial.println(UTC.dateTime());

	// The event then sets a new event for the next time
	setEvent( itIsTheSecondTuesday, nextSecondTuesday() );
}

time_t nextSecondTuesday() {

	int8_t m = UTC.month();
	int16_t y = UTC.year();
	time_t t = 0;
	
	while (t <= UTC.now()) {
		// Try in current month first, if that has passed, loop once more for next month
		t = makeOrdinalTime(12, 0, 0, SECOND, TUESDAY, m, y);
		m++;
		if (m == 13) {
			m = 1;
			y++;
		}
	}
	return t;
}
