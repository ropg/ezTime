# ezTime Documentation

**ezTime, pronounced "Easy Time", is a very easy to use Arduino time and date library that provides NTP network time lookups, extensive timezone support, formatted time and date strings, millisecond precision and more.**

## A brief history of ezTime

I was working on M5ez, an interface library to easily make cool-looking programs for the "M5Stack" ESP32 hardware. The status bar of that needed to display the time. I figured I would use [Time](https://github.com/PaulStoffregen/Time), Paul Stoffregen's library to do time things on Arduino. Then I needed to sync that to an NTP server, so I figured I would use [NTPclient](https://github.com/arduino-libraries/NTPClient), one of the existing NTP client libraries. And then I wanted it to show the local time, so I would need some way for the user to set an offset between UTC and local time. 

So far, so good.

Then I remembered how annoyed I always am when daylight savings time comes or goes, as I have to manually set some of my clocks such as the microwave oven, the clock in the car dashboard, etc etc. My clock would need to know about timezone rules. So I could get Jack Christensen's [Timezone library](https://github.com/JChristensen/Timezone). But it needs the timezone's rules, like "DST goes into effect on the last Sunday in March at 02:00 local time" told to it. I figured I would simply get this data from the internet and parse it.

And then I also wanted to print time in various formats. Wouldn't it be nice to have some function to print formatted time like many programming languages offer them?

Overlooking the battlefield after implementing some of this, it seemed like there had to be a better way. Especially, some way in which all this work would benefit more people. I decided to make the mother of all time libraries.

## ezTime is:

**self-contained**: It only depends on other libraries for networking (to do NTP and timezone data lookups). And if it is running on an ESP32, it will use the `Preferences` library to store cached timezone data to minimize lookups. It uses [timezoneapi.io](https://timezoneapi.io/) to get its timezone data, they provide 50 free lookups per user per day. (You can turn all of this off at compile time, e.g. if you have no networking and/or another time source.)

**precise**: An NTP request to pool.ntp.org only takes 40ms round-trip on my home DSL, so adding sub-second precision to a time library makes sense. ezTime reads the fractional seconds and tries to account for network latency to give you precise time.

**backwards compatible**: Anything written for the existing Arduino time library will still work. You can set which timezone the sketch should be in, or have it be in UTC which is the default. But you can also set and express time referring to multiple timezones, all very easy and intuitive.

**robust**: On ESP32 it doesn't fail if the timezone api goes away: it will cache the data for any timezones used. If that server is unreachable it will not initialise new timezones or do a yearly update of the rules, but it will still work on timezones you have already used.

**informative**: No need to guess while you're working on something, ezTime can print messages to the serial port at your desired level of detail, telling you when it gets an NTP update and how much your internal clock was off, for instance.

**time-saving**: No more time spent on writing code to print date or time in some nicer way. Print things like "8:20 PM" or "Saturday the 23rd of August 2018" with ease. Prevent display-flicker with minuteChanged() and secondChanged() functions without storing any values to compare.

**easy to use**: Don't believe it until you see it. Have a look at some of these examples to see how easy it is to use.

&nbsp;

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

&nbsp;

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

&nbsp;

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

&nbsp;

## Getting started

ezTime is an Arduino library. To start using it with the Arduino IDE:

* Choose Sketch -> Include Library -> Manage Libraries...
* Type `ezTime` into the search box.
* Click the row to select the library.
* Click the Install button to install the library.

in File -> Examples you will now see an ezTime heading down under "Examples from custom libraries". You can try running some of these examples to see if it all works. ezTime is made to be, as the name implies, quite easy to use. So you'll probably understand a lot of how things work from just looking at the examples.

When you include the ezTime library, it creates two objects for you to interact with. One object is called `ezTime`, and its methods allow you to change defaults like which NTP server is used, the interval at which it is polled as well as many other more general time related things. Then it also creates an instance of the `Timezone` class called `UTC`. UTC's methods allow you to do things that refer to time as specified in UTC. So for instance, `UTC.dateTime()` returns a string representation of the current date and time in UTC.

If your code says
```
Timezone CEST;
CEST.setPosix("CST-2");
```
it will create a timezone with a very simple rule: it is called CST and it is two hours later there than at UTC. Now you can use `CEST.dateTime()`, to get the current time in CEST. But this will not change to and from Daylight Saving Time if your timezone has that. The more powerful way of initialising a timezone is using 'getLocation' like:
```
Timezone berlin;
berlin.setLocation("Europe/Berlin");
```


# &nbsp;

# &nbsp;

# ezTime: complete documentation

`#include <ezTime.h>` includes the library, creates `ezTime` object and `UTC` instance of `Timezone` class, as well as `defaultTZ`, which is a reference to UTC unless you set it to another timzone by calling `someTZ.setDefault()`. 

## Index

* ezTime object
	* breakTime
	* clearCache
	* compileTime
	* debug
	* error
	* errorString
	* getBetween
	* makeTime
	* makeUmpteenthTime
	* now
	* queryNTP
	* setInterval
	* setServer
	* timeStatus
	* timezoneAPI
	* updateNow
	* urlEncode
	* waitForSync
	* zeropad
* Timezone class
	* dateTime
	* day
	* getOffset
	* getPosix
	* getTimezoneName
	* hour
	* isDST
	* isDST_UTC
	* isDST_local
	* minute
	* minuteChanged
	* month
	* ms
	* now
	* second
	* secondChanged
	* setDefault
	* setPosix
	* setTime
	* weekday
	* year


## Work in progress...



`bool setPosix(String posix)`

`String getPosix()`

`void setDefault()`

`bool isDST_local(time_t t = TIME_NOW)`

`bool isDST_UTC(time_t t = TIME_NOW)`

`bool isDST()`

`String getTimezoneName(time_t t = TIME_NOW)`

`int32_t getOffset(time_t t = TIME_NOW)`

`time_t now(bool update_last_read = true)`

`void setTime(time_t t)`

`void setTime(const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t mnth, uint16_t yr)`

`String dateTime(String format = DEFAULT_TIMEFORMAT)`

`String dateTime(time_t t, String format = DEFAULT_TIMEFORMAT)`

`uint8_t hour(time_t t = TIME_NOW)`

`uint8_t minute(time_t t = TIME_NOW)`

`uint8_t second(time_t t = TIME_NOW)`

`uint16_t ms(time_t t = TIME_NOW)`

`uint8_t day(time_t t = TIME_NOW)`

`uint8_t weekday(time_t t = TIME_NOW)`

`uint8_t month(time_t t = TIME_NOW)`

`uint16_t year(time_t t = TIME_NOW)`

`bool secondChanged()`

`bool minuteChanged()`


`void breakTime(time_t time, tmElements_t &tm)`

`void clearCache();`

`time_t compileTime(String compile_date = __DATE__, String compile_time = __TIME__);`

`void debug(ezDebugLevel_t level);`

`ezError_t error();`

`String errorString(ezError_t err);`

`String getBetween(String &haystack, String before_needle, String after_needle = "");`

`time_t makeTime(tmElements_t &tm);`

`time_t makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, int16_t year);`

`time_t makeUmpteenthTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t umpteenth, uint8_t wday, uint8_t month, int16_t year);`

`time_t now();`

`bool queryNTP(String server, time_t &t, unsigned long &measured_at);`

`void setInterval(uint16_t seconds = 0);`

`void setServer(String ntp_server = NTP_SERVER);`

`timeStatus_t timeStatus();`

`bool timezoneAPI(String location, String &olsen, String &posix);`

`void updateNow();`

`String urlEncode(String str);`

`bool waitForSync(uint16_t timeout = 0);`

`String zeropad(uint32_t number, uint8_t length);`


