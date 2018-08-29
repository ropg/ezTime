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


## Documentation

### Starting it all

It all starts when you include the library with  `#include <ezTime.h>`. From that point forward there is an object instance called `ezTime` with methods to control the behaviour of ezTime, as well as a timezone object called `UTC`. But nothing happens until you ask the library what time it is. This can be done by using any of the methods that tell time, like the `.dateTime` method of the timezone object. For instance, your code could do 
`Serial.println(UTC.dateTime());` to print a complete textual representation of date and time to the serial port. The library would find out that time had not been synchronized yet, and it would send off an NTP request to one of the NTP servers that `pool.ntp.org` resolves to. If your Arduino has just woken up, it is probably not connected to the WiFi network just yet, and so the call to `.dateTime` would return a String with the date and time just after midnight on the 1st of January 1970: the zero-point for the unix-style time counter used by ezTime.

## Setting and synchronising time

The NTP request from the scenario above failed because the network wasn't up yet, so the clock would still not be synchronized. Subsequent requests will retry the NTP query, but only if they happen at least 3 seconds later. (These 3 seconds are settable with the `NTP_RETRY` define from `ezTime.h`.) 

### ezTime.timeStatus

`timeStatus_t ezTime.timeStatus();`

Returns what state the clock is in. `ezTime.timeStatus()` will return either `timeNotSet`, `timeNeedsSync` or `timeSet`.

* `timeNotSet` means no NTP update or other setting of the clock (with the `.settime` method) has taken place
* `timeSet` means the clock should have the current time
*  `timeNeedsSync` means a scheduled NTP request has been due for more than an hour. (The time an update needs to be due before `timeNeedsSync` is set is configured by the `NTP_STALE_AFTER` define in the `ezTime.h` file.)

### ezTime.waitForSync

`bool ezTime.waitForSync(uint16_t timeout = 0);`

If your code uses timezones other than UTC, it might want to wait to initialise them until there is a valid time to see if the cached timezone definitions are still current. And if you are displaying a calendar or clock, it might look silly if it first says midnight on January 1st 1970 before showing the real time. `ezTime.waitForSync` will wait for the network to connect, and then for the time to be synchronized before returning `true`. If you specify a timeout (in seconds), it will return after that many seconds even if the clock is not in sync yet, returning `false`.

### ezTime.setServer and ezTime.setInterval

`void ezTime.setServer(String ntp_server = NTP_SERVER);`

`void ezTime.setInterval(uint16_t seconds = 0);`

By default, ezTime is set to poll `pool.ntp.org` every 10 minutes. These defaults should work for most people, but you can change them by specifying a new server with `ezTime.setServer` or a new interval (in seconds) with ezTime.setInterval. If you call setInterval with an interval of 0 seconds or call it as `ezTime.setInterval()`, no more NTP queries will be made.

### ezTime.updateNow

`void ezTime.updateNow();`

Schedules the next update to happen immediately, and then tries to query the NTP server. If that fails, it will keep retrying every 3 seconds.

### ezTime.queryNTP

`bool ezTime.queryNTP(String server, time_t &t, unsigned long &measured_at);`

This will send a single query to the NTP server your specify. It will put, in the `t` and `measured_at` variables passed by reference, the UTC unix-time and the `millis()` counter at the time the exact second happened. It does this by subtracting from `millis()` the fractional seconds received in the answer, as well as half the time it took to get an answer. This means it assumes the network delay was symmetrical, meaning it took just as long for the request to get to the server as for the answer to get back. 

If the time server answers, `ezTime.queryNTP` returns `true`. If `false` is returned, `ezTime.error()` will return either `NO_NETWORK` (if the WiFi is not connected) or `TIMEOUT` if a response took more than 1500 milliseconds (defined by `NTP_TIMEOUT` in `ezTime.h`). 

Note that this function is used internally by ezTime, but does not by itself set the time ezTime keeps. You will likely never need to call this from your code.

## Errors and debug information

`void ezTime.debug(ezDebugLevel_t level);`

`ezError_t ezTime.error();`

`String ezTime.errorString(ezError_t err);`

## Timezones

`#include <ezTime.h>` includes the library, creates `ezTime` object and `UTC` instance of `Timezone` class, as well as `defaultTZ`, which is a reference to UTC unless you set it to another timzone by calling `someTZ.setDefault()`. 

`bool someTZ.setPosix(String posix)`

`String someTZ.getPosix()`

`void someTZ.setDefault()`

`bool someTZ.isDST_local(time_t t = TIME_NOW)`

`bool someTZ.isDST_UTC(time_t t = TIME_NOW)`

`bool someTZ.isDST()`

`String someTZ.getTimezoneName(time_t t = TIME_NOW)`

`int32_t someTZ.getOffset(time_t t = TIME_NOW)`

`bool ezTime.timezoneAPI(String location, String &olsen, String &posix);`

## Getting/setting date and time

`time_t someTZ.now(bool update_last_read = true)`

`void someTZ.setTime(time_t t)`

`void someTZ.setTime(const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t mnth, uint16_t yr)`

```
String someTZ.dateTime(String format = DEFAULT_TIMEFORMAT);
String someTZ.dateTime(time_t t, String format = DEFAULT_TIMEFORMAT);
```

```
uint8_t someTZ.hour(time_t t = TIME_NOW);
uint8_t someTZ.minute(time_t t = TIME_NOW);
uint8_t someTZ.second(time_t t = TIME_NOW);
uint16_t someTZ.ms(time_t t = TIME_NOW);
uint8_t someTZ.day(time_t t = TIME_NOW);
uint8_t someTZ.weekday(time_t t = TIME_NOW);
uint8_t someTZ.month(time_t t = TIME_NOW);
uint16_t someTZ.year(time_t t = TIME_NOW);
```

```
bool someTZ.secondChanged();
bool someTZ.minuteChanged();
```

## Compatibility with Arduino `Time` library

`time_t ezTime.makeTime(tmElements_t &tm);`

`time_t ezTime.makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, int16_t year);`

`void ezTime.breakTime(time_t time, tmElements_t &tm)`

## Various functions

`time_t ezTime.makeUmpteenthTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t umpteenth, uint8_t wday, uint8_t month, int16_t year);`

`time_t ezTime.compileTime(String compile_date = __DATE__, String compile_time = __TIME__);`

`String ezTime.getBetween(String &haystack, String before_needle, String after_needle = "");`

`String urlEncode(String str);`

`String zeropad(uint32_t number, uint8_t length);`

`void ezTime.clearCache();`