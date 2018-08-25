# ezTime Documentation

## A brief history of ezTime

I was working on M5ez, an interface library to easily make cool-looking programs for the "M5Stack" ESP32 hardware. The status bar of that needed to display the time. I figured I would use [Time](https://github.com/PaulStoffregen/Time), Paul Stoffregen's library to do time things on Arduino. Then I needed to sync that to an NTP server, so I figured I would use [NTPclient](https://github.com/arduino-libraries/NTPClient), one of the existing NTP client libraries. And then I wanted it to show the local time, so I would need some way for the user to set an offset between UTC and local time. 

So far, so good.

Then I remembered how annoyed I always am when daylight savings time comes or goes, as I have to manually set some of my clocks such as the microwave oven, the clock in the car dashboard, etc etc. My clock would need to know about timezone rules. So I could get JChristensen's [Timezone library](https://github.com/JChristensen/Timezone). But it needs the timezone's rules, like "DST goes into effect on the last Sunday in March at 02:00 local time" told to it, and that seemed a hassle. So I would simply get this data from the internet.

And then I also wanted to print time in various formats. Wouldn't it be nice to have some function to print formatted time like many programming languages offer them?

Overlooking the battlefield after implementing some of this, it seemed like there had to be a better way. Especially, some way in which all this work would benefit more people. I decided to make the mother of all time libraries.

## ezTime is:

**self-contained**: It only depends on core ESP32 libraries (for networking and for storing its cached timezone data in flash). It uses [timezoneapi.io](https://timezoneapi.io/) to get its timezone data. 

**precise**: An NTP request to pool.ntp.org only takes 40ms round-trip on my home DSL, so adding sub-second precision to a time library makes sense. ezTime reads the fractional seconds and tries to account for network latency to give you precise time.

**backwards compatible**: Anything written for the existing Arduino time library will still work. You simply set which timezone the sketch should be in. (Or have it be in UTC, which is the default.) But you can also refer to the ezTime or Timezone objects directly and use additional power.

**robust**: Doesn't fail if the timezone api goes away. It is built to cache the data for any timezones used. If that server is unreachable it will not initialise new timezones or do a yearly update of the rules, but it will still work on timezones you have already used.

**informative**: No need to guess while you're working on something, ezTime can print messages to the serial port at your desired level of detail.

**time-saving**: No more time spent on writing code to print date or time in some nicer way. Print things like "9:20 AM" or "Saturday the 23rd of August 2018" with ease. Prevent display-flicker with minuteChanged() and secondChanged() functions without storing any values to compare.

**easy to use**: Don't believe it until you see it. Have a look at some examples to see how easy it is to use.

### Timezones

```
#include <ezTime.h>
#include <WiFi.h>

void setup() {
	Serial.begin(115200);
	WiFi.begin("your-ssid", "your-password");

	ezTime.waitForSync();

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
```

```
UTC:             Saturday, 25-Aug-18 14:13:03 UTC
Auckland:        Sunday, 26-Aug-18 03:13:04 NZST
Paris, Texas:    Saturday, 25-Aug-18 09:13:04 CDT
Your local time: Saturday, 25-Aug-18 16:13:04 CEST
```

### Formatted date and time

```
#include <ezTime.h>
#include <WiFi.h>

void setup() {

	Serial.begin(115200);
	WiFi.begin("your-ssid", "your-password");

	ezTime.waitForSync();

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
```

```
Time in various internet standard formats ...

ATOM:        2018-08-25T14:23:45+00:00
COOKIE:      Saturday, 25-Aug-2018 14:23:45 UTC
IS8601:      2018-08-25T14:23:45+0000
RFC822:      Sat, 25 Aug 18 14:23:45 +0000
RFC850:      Saturday, 25-Aug-18 14:23:45 UTC
RFC1036:     Sat, 25 Aug 18 14:23:45 +0000
RFC1123:     Sat, 25 Aug 2018 14:23:45 +0000
RFC2822:     Sat, 25 Aug 2018 14:23:45 +0000
RFC3339:     2018-08-25T14:23:45+00:00
RFC3339_EXT: 2018-08-25T14:23:45.846+00:00
RSS:         Sat, 25 Aug 2018 14:23:45 +0000
W3C:         2018-08-25T14:23:45+00:00

 ... and any other format, like "Saturday the 25th of August 2018, 2:23 PM"
```

### milliseconds

```
#include <ezTime.h>
#include <WiFi.h>

void setup() {
	Serial.begin(115200);
	WiFi.begin("your-ssid", "your-password");

	ezTime.setInterval(60);
	ezTime.waitForSync();

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
	ezTime.debug(INFO);
}

void loop() {
	now();
	delay(1000);
}
```

```
Saturday, 25-Aug-18 14:32:53.282 UTC
Saturday, 25-Aug-18 14:32:53.283 UTC
Saturday, 25-Aug-18 14:32:53.284 UTC
Saturday, 25-Aug-18 14:32:53.285 UTC
Saturday, 25-Aug-18 14:32:53.287 UTC
Saturday, 25-Aug-18 14:32:53.290 UTC
Saturday, 25-Aug-18 14:32:53.293 UTC
Saturday, 25-Aug-18 14:32:53.297 UTC
Saturday, 25-Aug-18 14:32:53.300 UTC
Saturday, 25-Aug-18 14:32:53.303 UTC

Those milliseconds between the first and the last line ...

     ... most of that is spent sending to the serial port.



And ezTime is not making those milliseconds up either.

      ... Stick around as we do an NTP request every minute.

ezTime debug level set to INFO
Querying pool.ntp.org ... success (round trip 47 ms)
Received time: Saturday, 25-Aug-18 14:33:53.363 UTC (internal clock was 31 ms slow)
Querying pool.ntp.org ... success (round trip 42 ms)
Received time: Saturday, 25-Aug-18 14:34:53.410 UTC (internal clock was 1 ms fast)
Querying pool.ntp.org ... success (round trip 43 ms)
Received time: Saturday, 25-Aug-18 14:35:53.480 UTC (internal clock was 1 ms slow)
Querying pool.ntp.org ... success (round trip 43 ms)
Received time: Saturday, 25-Aug-18 14:36:53.525 UTC (internal clock was 1 ms slow)
Querying pool.ntp.org ... success (round trip 36 ms)
Received time: Saturday, 25-Aug-18 14:37:53.573 UTC (internal clock was 4 ms slow)
Querying pool.ntp.org ... success (round trip 35 ms)
Received time: Saturday, 25-Aug-18 14:38:53.636 UTC (internal clock was spot on)
Querying pool.ntp.org ... success (round trip 32 ms)
Received time: Saturday, 25-Aug-18 14:39:53.674 UTC (internal clock was 1 ms slow)
```

## Getting started

SOON

# ezTime: complete documentation

SOON