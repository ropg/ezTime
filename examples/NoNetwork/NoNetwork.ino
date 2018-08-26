/*
 * This example also does something useful with "#define EZTIME_NETWORK_ENABLE" at the 
 * start of ezTime.h commented out. It will start the time at the time the code was
 * compiled. You have to set your local timezone information by hand in the 
 * LOCALTZ_POSIX define. (The string contains the names for your TZ in standard and
 * Daylight Saving Time, as well as the starting and ending point for DST and the 
 * offset to UTC.
 */


#include <ezTime.h>
#include <WiFi.h>

#define LOCALTZ_POSIX	"PST+8PDT,M3.2.0/2,M11.1.0/2"		// US Pacific time

Timezone local;
Timezone berlin;

void setup() {

	Serial.begin(115200);
	Serial.println();

	local.setPosix(LOCALTZ_POSIX);
	local.setTime(compileTime());
	Serial.println("Local time  :  " + local.dateTime());

	berlin.setPosix("CET-1CEST,M3.4.0/2,M10.4.0/3");
	Serial.println("Berlin time :  " + berlin.dateTime());

	Serial.println("UTC         :  " + UTC.dateTime());
	
}

void loop() {
}


// Kindly borrowed from one of Jack Christensen's Timezone examples
time_t compileTime()
{
    const time_t FUDGE(10);     // fudge factor to allow for compile time (seconds, YMMV)
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char chMon[3], *m;
    tmElements_t tm;

    strncpy(chMon, compDate, 3);
    chMon[3] = '\0';
    m = strstr(months, chMon);
    tm.Month = ((m - months) / 3 + 1);
    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);

    time_t t = makeTime(tm);
    
    return t + FUDGE;           // add fudge factor to allow for compile time
}
