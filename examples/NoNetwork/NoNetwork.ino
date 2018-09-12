/*
 * This example also does something useful with "#define EZTIME_NETWORK_ENABLE" at the 
 * start of ezTime.h commented out. It will start the time at the time the code was
 * compiled. You have to set your local timezone information by hand in the 
 * LOCALTZ_POSIX define. (The string contains the names for your TZ in standard and
 * Daylight Saving Time, as well as the starting and ending point for DST and the 
 * offset to UTC.
 * 
 * If you do not want to look up the posix string you can simply provide a name and
 * the current UTC offset in hours _west_ of UTC, like "PDT+7" 
 */


#include <ezTime.h>

#define LOCALTZ_POSIX	"CET-1CEST,M3.4.0/2,M10.4.0/3"		// Time in Berlin

Timezone local;
Timezone pacific;

void setup() {

	Serial.begin(115200);
	while (!Serial) { ; }		// wait for Serial port to connect. Needed for native USB port only
	Serial.println();

	local.setPosix(LOCALTZ_POSIX);
	local.setTime(compileTime());
	Serial.print(F("Local time   :  "));
	Serial.println(local.dateTime());

	pacific.setPosix(F("PST+8PDT,M3.2.0/2,M11.1.0/2"));
	Serial.print(F("Pacific time :  "));
	Serial.println(pacific.dateTime());

	Serial.print(F("UTC          :  "));
	Serial.println(UTC.dateTime());
	
}

void loop() {
}
