# ezTime, an Arduino library for all of time <sup>*</sup>

**ezTime &mdash; pronounced "Easy Time" &mdash; is a very easy to use Arduino time and date library that provides NTP network time lookups, extensive timezone support, formatted time and date strings, user events, millisecond precision and more.**

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<sup>* limitations may apply, see "2036 and 2038" chapter</sup>

<img src ="images/moving-clock.gif" />

## A brief history of ezTime

I was working on [M5ez](https://github.com/ropg/M5ez), an interface library to easily make cool-looking programs for the "M5Stack" ESP32 hardware. The status bar of that needed to display the time. That was all, I swear. I figured I would use [Time](https://github.com/PaulStoffregen/Time), Paul Stoffregen's library to do time things on Arduino. Then I needed to sync that to an NTP server, so I figured I would use [NTPclient](https://github.com/arduino-libraries/NTPClient), one of the existing NTP client libraries. And then I wanted it to show the local time, so I would need some way for the user to set an offset between UTC and local time. 

So far, so good.

Then I remembered how annoyed I always am when daylight savings time comes or goes, as I have to manually set some of my clocks such as the microwave oven, the clock in the car dashboard, etc etc. My clock would need to know about timezone rules. So I could get Jack Christensen's [Timezone library](https://github.com/JChristensen/Timezone). But it needs the timezone's rules, like "DST goes into effect on the last Sunday in March at 02:00 local time" told to it. I figured I would simply get this data from the internet and parse it.

And then I wanted 12 or 24 hour time displayed, and thought about various formats for date and time. Wouldn't it be nice to have some function to print formatted time like many programming languages offer them?

Overlooking the battlefield after implementing some part of this, it seemed like there had to be a better way. Especially, some way in which all this work would benefit more people. This is how ezTime &mdash; the project that was only going to take a few days &mdash; came to be. 

## ezTime is ...

**self-contained**: It only depends on other libraries to get online, but then it doesn't need other libraries for NTP and timezone data lookups.

**precise**: Unlike other libraries, ezTime does not throw away or mangle the fractional second information from the NTP server. An NTP request to pool.ntp.org only takes 40ms round-trip on home DSL these days, so adding sub-second precision to a time library makes sense. ezTime reads the fractional seconds and tries to account for network latency to give you precise time.

**backwards compatible**: Anything written for the existing Arduino time library will still work. You can set which timezone the sketch should be in, or have it be in UTC which is the default. But you can also set and express time referring to multiple timezones, all very easy and intuitive.

**eventful**: You can set events to have ezTime execute your own functions at a given time, and delete the events again if you change your mind.

**robust**: It doesn't fail if the timezone api goes away: it can use cached data, which ezTime can store in EEPROM (AVR Arduinos) or NVS (ESP32 through Preferences library). 

**informative**: No need to guess while you're working on something, ezTime can print messages to the serial port at your desired level of detail, telling you about the timezone's daylight savings info it receives or when it gets an NTP update and by how much your internal clock was off, for instance.

**time-saving**: No more time spent on writing code to print date or time in some nicer way. Print things like "8:20 PM" or "Saturday the 23rd of August 2018" with ease. Prevent display-flicker with `minuteChanged()` and `secondChanged()` functions without storing any values to compare.

**small enough**: Works with all features and full debugging information on an Arduino Uno with an Ethernet Shield, leaving 2/3 of RAM but not too much flash for you to work with. (Even on a good day there isn't that much left if you want to do anything that involves networking.) Various `#define` options let you leave parts of the library out if you want to make it smaller: you can even leave out the networking altogether if you have a different time source.

**easy to use**: Don't believe it until you see it. Have a look at some of these examples to see how easy it is to use.

&nbsp;

### Timezones 

(a complete sketch to show how simple it is)

```
#include <ezTime.h>
#include <WiFi.h>

void setup() {
	Serial.begin(115200);
	WiFi.begin("your-ssid", "your-password");

	time.waitForSync();

	Serial.println("UTC: " + UTC.dateTime());
	
	Timezone NewZealand;
	NewZealand.setLocation("Pacific/Auckland");
	Serial.println("New Zealand time: " + NewZealand.dateTime());
}

void loop() { }
```

```
UTC: Friday, 07-Sep-2018 11:25:10 UTC
New Zealand time: Friday, 07-Sep-2018 23:25:11 NZST
```

&nbsp;

### Formatted date and time

```
Serial.println("COOKIE:      " + UTC.dateTime(COOKIE));
Serial.println("IS8601:      " + UTC.dateTime(ISO8601));
Serial.println("RFC822:      " + UTC.dateTime(RFC822));
Serial.println("RFC850:      " + UTC.dateTime(RFC850));
Serial.println("RFC3339:     " + UTC.dateTime(RFC3339));
Serial.println("RFC3339_EXT: " + UTC.dateTime(RFC3339_EXT));
Serial.println("RSS:         " + UTC.dateTime(RSS));
Serial.println();
Serial.println("or like " + UTC.dateTime("l ~t~h~e jS ~o~f F Y, g:i A") );
```

```
COOKIE:      Saturday, 25-Aug-2018 14:23:45 UTC
IS8601:      2018-08-25T14:23:45+0000
RFC822:      Sat, 25 Aug 18 14:23:45 +0000
RFC850:      Saturday, 25-Aug-18 14:23:45 UTC
RFC3339:     2018-08-25T14:23:45+00:00
RFC3339_EXT: 2018-08-25T14:23:45.846+00:00
RSS:         Sat, 25 Aug 2018 14:23:45 +0000

or like Saturday the 25th of August 2018, 2:23 PM
```

&nbsp;

### milliseconds

```
for (int n = 0; n < 10; n++) {
	Serial.println(UTC.dateTime("l, d-M-y H:i:s.v T"));
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
```

> *This is on my ESP32. See how it fills up the serial buffer real fast at first, and then has to wait for the characters to be sent before it can return?*

&nbsp;

### Rich information and *... oh my just look at these NTP updates*

```
[...]
	time.setInterval(60);
	time.setDebugLevel(INFO);
}

void loop() {
	now();
	delay(1000);
}
```

```
ezTime debug level set to INFO
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

in File -> Examples you will now see an ezTime heading down under "Examples from custom libraries". You can try running some of these examples to see if it all works. ezTime is made to be, as the name implies, quite easy to use. So you'll probably understand a lot of how things work from just looking at the examples. Now either just play with those and use the rest of this documentation only when you get stuck, or keep reading to see how things work in time.

# &nbsp;

# ezTime User Manual

> *If it's not in here, it's wrong...*

## About this manual

### Semi-internal functions

Some functions are not necessarily useful for everyday users of this library, but might be useful to someone someday. For instance, this library checks with the NTP servers automatically, there should be no need to ever "manually" get an NTP response. But the function to do that is still exposed to the user. Even some functions that have nothing to do with time, like `urlEncode` are there for you to use, simply because they *might* be useful to someone, and the library needed them internally so they come at no extra cost in terms of size. In this manual, the names of these functions are printed in *italics* in their chapter headings, just to make it a easier for you to see which functions are core functionality and which are really not needed in everyday use.

### Specifying time

I hate documentation that still makes me reach for for the source code, so this manual supplies the function prototype with each function so you can see what types or arguments each function takes and what type the return value is. I took one shortcut though. A lot of functions allow you to specify a time. In the function prototype this looks like:

`time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL`

Throughout this manual, we replace these two optional arguments in the function definitions with:

`TIME` 

That's because the prior is just a little too long to be repeating a thousand times, and it also makes things look more complicated than they need to be. In most places where you specify a time in ezTime, you are most likely to mean "right now". This can be done by supplying no arguments at all, or `TIME_NOW`. You might make a number of requests in a row, and want to make sure that the time didn't change between them. No need to stick the time value in a variable. After you have made a call specifying no time (meaning `TIME_NOW`), you can specify `LAST_READ` to use the time from the exact moment you made that first call.

Otherwise, you can specify a `time_t` value, a well-known 32-bit signed integer way of specifying time in seconds elapsed since 00:00 Jan 1st 1970. If you specify a value other than `TIME_NOW` or `LAST_READ`, you can then specify whether you mean in UTC or local time, by following it with a second argument that is either `UTC_TIME` or `LOCAL_TIME`.

For example, if you have set up a timezone called Berlin, `Berlin.isDST(1536314299, UTC_TIME)` tells you whether Daylight Savings Time is in effect on that time, as seconds from 00:00 Jan 1st 1970 UTC, as opposed to that many seconds from that time in Berlin (which would be the default). There will be some examples later on, showing you how to create and process such timestamps. Mostly though, you don't need specify anything at all because you just want something time-related about "right now".

> *Time-geek sidenote: ezTime does not have historical information about the daylight davings rules of the past or future, it only applies the rules it has now as if they also applied in the past or future. Check [here](https://www.timeanddate.com/) for historical records for timezones.* 
 

## How it all works
 
### What happens when you include the library

It all starts when you include the library with  `#include <ezTime.h>`. From that point forward there is an object instance called `time` with methods to control the behaviour of ezTime, as well as a timezone object called `UTC`, and a reference to this object called `defaultTZ` (which you may point to a different timezone later).

### No daemons here

It is important to understand what ezTime does NOT do. It does not somehow create a background process that keeps time, contacts servers, or whatever. The Arduino does the timekeeping for us with its `millis()` counter, which keeps the time in milliseconds since the Arduino started. All ezTime does when it synchronizes time is to store a time (in seconds since 1970) and the position of the millis counter when that was. By seeing how much the millis counter has advanced and adding that starting point since 1970, ezTime tells time. But that internal clock isn't perfect, it may &mdash; very slowly &mdash; drift away from the actual time. So periodically when you ask it something, it will discover that it's time to re-synchronize its own clock with the NTP server, and then it'll go out, do that and take, say, 50 ms longer to respond back to your code. But it all happens only when you ask for it.

### But I only just woke up !

Your code might call `Serial.println(UTC.dateTime());` to print a complete textual representation of date and time in the default format to the serial port. The library would find out that time had not been synchronized yet, and it would send off an NTP request to one of the NTP servers that `pool.ntp.org` resolves to. If your Arduino has just woken up, it probably hasn't gotten its DHCP information, or is not connected to the WiFi network just yet. And so the time lookup would fail and the call to `.dateTime` would return a String with the date and time just after midnight on the 1st of January 1970: the zero-point for the unix-style time counter used by ezTime. It would later correct to the real time, but that's not pretty.

Worse is when you set up a timezone for which you would like to retrieve the daylight savings rules from the server: it can't do that if the connection isn't up yet. So that's why there's a function called `time.waitForSync` that simply requests the time until it is synchronized (or until a set number of seconds passes, see below).

&nbsp;

## Setting and synchronising time

The NTP request from the scenario above failed because the network wasn't up yet, so the clock would still not be synchronized. Subsequent requests will retry the NTP query, but only if they happen at least 5 seconds later. 

&nbsp;

### time.timeStatus

`timeStatus_t time.timeStatus();`

Returns what state the clock is in. `time.timeStatus()` will return one of:

| timeStatus | meaning |
|----|----|
| `timeNotSet` | No NTP update or manual setting of the clock (by calling the `.setTime` method of a timezone) has taken place |
| `timeSet` | The clock should have the current time |
| `timeNeedsSync` | A scheduled NTP request has been due for more than an hour. (The time an update needs to be due before `timeNeedsSync` is set is configured by the `NTP_STALE_AFTER` define in the `ezTime.h` file.) |

&nbsp;

### time.waitForSync

`bool time.waitForSync(uint16_t timeout = 0);`

If your code uses timezones other than UTC, it might want to wait to initialise them until there is a valid time to see if the cached timezone definitions are still current. And if you are displaying a calendar or clock, it might look silly if it first says midnight on January 1st 1970 before showing the real time. `time.waitForSync` will wait for the network to connect, and then for the time to be synchronized before returning `true`. If you specify a timeout (in seconds), it will return after that many seconds even if the clock is not in sync yet, returning `false`. (ezTime error `TIMEOUT`, see the chapter on error and debug messages further down)

&nbsp;

### *time.setServer and time.setInterval*

`void time.setServer(String ntp_server = NTP_SERVER);`

`void time.setInterval(uint16_t seconds = 0);`

By default, ezTime is set to poll `pool.ntp.org` every 10 minutes. These defaults should work for most people, but you can change them by specifying a new server with `time.setServer` or a new interval (in seconds) with time.setInterval. If you call setInterval with an interval of 0 seconds or call it as `time.setInterval()`, no more NTP queries will be made.

&nbsp;

### *time.updateNTP*

`void time.updateNTP();`

Updates the time from the NTP server immediately. Will keep retrying every 5 seconds (defined by `NTP_RETRY` in `ezTime.h`), will schedule the next update to happen after the normal interval.

&nbsp;

### *time.queryNTP*

`bool time.queryNTP(String server, time_t &t, unsigned long &measured_at);`

This will send a single query to the NTP server your specify. It will put, in the `t` and `measured_at` variables passed by reference, the UTC unix-time and the `millis()` counter at the time the exact second happened. It does this by subtracting from `millis()` the fractional seconds received in the answer, as well as half the time it took to get an answer. This means it assumes the network delay was symmetrical, meaning it took just as long for the request to get to the server as for the answer to get back. 

If the time server answers, `time.queryNTP` returns `true`. If `false` is returned, `time.error()` will return either `NO_NETWORK` (if the WiFi is not connected) or `TIMEOUT` if a response took more than 1500 milliseconds (defined by `NTP_TIMEOUT` in `ezTime.h`). 

Note that this function is used internally by ezTime, but does not by itself set the time ezTime keeps. You will likely never need to call this from your code.

&nbsp;

## Timezones

> *If only it was as uncomplicated as this map suggests. Every band is actually made up of countries that all change to their Daylight Saving Time on different dates, and they even change the rules for when that happens frequently.*

![](images/timezones.gif)

Timezones in ezTime are objects. They can be created with `Timezone yourTZ`, where `yourTZ` is the name you choose to refer to the timezone. In this manual, this name will be used from now on. But you can naturally choose any name you want.

Internally, ezTime stores everything it knows about a timezone as two strings. One is the official name of the timezone in "Olsen" format (like `Europe/Berlin`). That name is used to then update when needed all the other information needed to represent time in that timezone. This is in another string, in so-called "posix" format. It's often a little longer and for Berlin it is `CET-1CEST,M3.4.0/2,M10.4.0/3`. The elements of this string have the following meanings:

| Element | meaning |
| ---- | ---- |
| `CET` | Name of timezone in standard time (CET = Central European Time in this case.)
| `-1` | Hours offset from UTC, meaning subtract one hour from this time to get to UTC. (Note offset is often written elsewhere the other way around (so +1 in this case), just to confuse things.) Could also specify minutes, like `-05:30` for India. | 
| `CEST` | Name of timezone in Daylight Saving  Time (DST), CEST stands for Central European Summer Time |
| `,M3` | DST starts in March |
| `.4` | On the fourth occurence of
| `.0` | a Sunday |
| `/2` | at 02:00 local time |
| `,M10` | DST ends in October |
| `.4` | on the fourth ocurrence of |
| `.0` | a Sunday |
| `/3` | at 03:00 local time |

&nbsp;

### yourTZ.setDefault

`void yourTZ.setDefault()`

`#include <ezTime.h>` includes the library, creates `ezTime` object and `UTC` instance of `Timezone` class, as well as `defaultTZ`, which is a reference to UTC unless you set it to another timzone by calling `yourTZ.setDefault()`. The functions in the root namespace like `hour()` and `minute()` &mdash; without a timezone in front &mdash; are interpreted as if passed to the default timezone. So if you have ezisting code, just setting up a timezone and making it the default should cause that code to work as if the time was set in local time. 

&nbsp;

### yourTZ.setPosix

`bool yourTZ.setPosix(String posix)`

Allows you to directly enter the posix information for a timezone. For simple timezones, you could set things up manually. For example for India, a mere

```
Timezone India
India.setPosix("IST-5:30")
Serial.println(India.dateTime());
```

is enough, because the time in India doesn't go back and forth with the coming and going of Daylight Saving Time (even though the half hour offset to UTC is pretty weird.)

&nbsp;

### yourTZ.getPosix

`String yourTZ.getPosix()`

`getPosix` does what you would expect and simply returns the posix string stored in ezTime for a given timezone. 

&nbsp;

### yourTZ.isDST

`bool yourTZ.isDST(TIME);`

Tells you whether DST is in effect at a given time in this timezone. If you do not provide arguments, it's interpreted as 'right now'. You can also specify a time (in seconds since 1970, we'll get back to that) in the first argument. If you want to know for a time in UTC, you can set the second argument to `false`, otherwise you are assumed to mean in local time.

&nbsp;

### yourTZ.getTimezoneName

`String getTimezoneName(TIME);`

Provides the current short code for the timezone, like `IST` for India, or `CET` (during standard time) or `CEST` (during Daylight Saving Time) for most of Europe. 

&nbsp;

### yourTZ.getOffset

`int16_t yourTZ.getOffset(TIME)`

Provide the offset from UTC in minutes at the indicated time (or now if you do not specify anything). The offset here is in the same direction as the posix information, so -120 means 2 hours east of UTC.

&nbsp;

### yourTZ.setLocation and timezoneapi.com

`bool yourTZ.setLocation(String location = "")`

With `setLocation` you can provide a string to do an internet lookup for a timezone. If the string contains a forward slash, the string is taken to be on Olsen timezone name, like `Europe/Berlin`. If it does not, it is parsed as a free form address, for which the system will try to find a timezone. You can enter "Paris" and get the info for "Europe/Paris", or enter "Paris, Texas" and get the timezone info for "America/Chicago", which is the Central Time timezone that Texas is in. After the information is retrieved, it is loaded in the current timezone, and cached if a cache is set (see below). `setLocation` will return `false` (Setting either `NO_NETWORK`, `CONNECT_FAILED` or `DATA_NOT_FOUND`) if it cannot find the information online. 

&nbsp;

### Timezone caching, EEPROM or NVS

You can create a place for ezTime to store the data about the timezone. That way, it doens't need to get the information anew every time the Arduino boots. There are two ways to do caching. You can supply an EEPROM location. A single timezone needs 50 bytes to cache. The data is written in compressed form so that the Olsen and Posix strings fit in 3/4 of the space they would normally take up, and alomg with it is stored a checksum, a length field and a single byte for the month in which the cache was retrieved, in months after January 2018.

&nbsp;

### setCache

`bool yourTZ.setCache(const int16_t address)`

`bool yourTZ.setCache(const String name, const String key)`

&nbsp;

### clearCache

`void time.clearCache(bool delete_section = false);`


## Getting date and time

&nbsp;

### yourTZ.dateTime

```
String yourTZ.dateTime(TIME, String format = DEFAULT_TIMEFORMAT);
```

We'll start with one of the most powerful functions of ezTime. With `dateTime` you can represent a date and/or a time in any way you want. You do this in the same way you do in many programming languages: by providing a special formatting string. Many characters in this string have special meanings and will be replaced. What this means is that `UTC.dateTime("l, d-M-y H:i:s.v T")` might return `Saturday, 25-Aug-18 14:32:53.282 UTC`. Below is the list of characters and what they are replaced by. Any characters not on this list are simply not replaced and stay as is. See the last two entries for a way to use characters on this list in your string.

| char | replaced by 
| ----- | :----- 
| `d` | Day of the month, 2 digits with leading zeros 
| `D` | First three letters of day in English, like `Tue` 
| `j` | Day of the month without leading zeros 
| `l` | (lowercase L) Day of the week in English, like `Tuesday` 
| `N` | // ISO-8601 numeric representation of the day of the week. (1 = Monday, 7 = Sunday)
| `S` | English ordinal suffix for the day of the month, 2 characters (st, nd, rd, th) 
| `w` | Numeric representation of the day of the week (0 = Sunday) 
| `F` | A month's name, such as `January` 
| `m` | Numeric representation of a month, with leading zeros 
| `M` | Three first letters of a monthin English, like `Apr` 
| `n` | Numeric representation of a month, without leading zeros 
| `t` | Number of days in the given month 
| `Y` | A full numeric representation of the year, 4 digits 
| `y` | Last two digits of the year 
| `a` | am or pm 
| `A` | AM or PM 
| `g` | 12-hour format of an hour without leading zeros 
| `G` | 24-hour format of an hour without leading zeros 
| `h` | 12-hour format of an hour with leading zeros 
| `H` | 24-hour format of an hour with leading zeros 
| `i` | Minutes with leading zeros 
| `s` | Seconds with leading zero 
| `T` | abbreviation for timezone, like `CEST` 
| `v` | milliseconds as three digits 
| `e` | Timezone identifier (Olsen name), like `Europe/Berlin` 
| `O` | Difference to Greenwich time (GMT) in hours and minutes written together, like `+0200`. Here a positive offset means east of UTC. 
| `P` | Same as O but with a colon between hours and minutes, like `+02:00` 
| `Z` | Timezone offset in seconds. West of UTC is negative, east of UTC is positive. 
| `z` | The day of the year (starting from 0) 
| `W` | ISO-8601 week number. See right below for explanation link.
| `X` | ISO-8601 year for year-week notation as four digit year. Warning: Not guaranteed to be same as current year, may be off by one at start or end of year. See [here](https://en.wikipedia.org/wiki/ISO_week_date) 
| `\` | Not printed, but escapes the following character, meaning it will not be replaced. But inserting a backslash in the string means you have to supply two backslashes `\\` to be interpreted as one. 
| `~` | (tilde) Same as backslash above, except easier to insert in the string. Example: `~t~h~e` will print the word `the` in the string. Letters should be escaped even if they are not on the list because they may be repolaced in future versions. 

So as an example: `UTC.dateTime("l ~t~h~e jS ~o~f F Y, g:i A")` yields date and time in this format: `Saturday the 25th of August 2018, 2:23 PM`.

#&nbsp;

### Built-in date and time formats

There are built-in values to specify some standard date and time formats. For example: `UTC.dateTIme(RSS)` (without quotes around RSS) returns something like `Sat, 25 Aug 2018 14:23:45 +0000`. Here's a list of all these built in format abbreviations.

| name        | formatted date and time
|:------|:------|
| ATOM        | 2018-08-25T14:23:45+00:00	
| COOKIE      | Saturday, 25-Aug-2018 14:23:45 UTC
| IS8601      | 2018-08-25T14:23:45+0000
| RFC822      | Sat, 25 Aug 18 14:23:45 +0000
| RFC850      | Saturday, 25-Aug-18 14:23:45 UTC
| RFC1036     | Sat, 25 Aug 18 14:23:45 +0000	
| RFC1123     | Sat, 25 Aug 2018 14:23:45 +0000	
| RFC2822     | Sat, 25 Aug 2018 14:23:45 +0000	
| RFC3339     | 2018-08-25T14:23:45+00:00
| RFC3339_EXT | 2018-08-25T14:23:45.846+00:00
| RSS         | Sat, 25 Aug 2018 14:23:45 +0000
| W3C         | 2018-08-25T14:23:45+00:00	
| ISO8601_YWD | 2018-W34-5

&nbsp;

### now

`time_t yourTZ.now(bool update_last_read = true)`


```
uint8_t yourTZ.hour(TIME);
uint8_t yourTZ.minute(TIME);
uint8_t yourTZ.second(TIME);
uint16_t yourTZ.ms(TIME);
uint8_t yourTZ.day(TIME);
uint8_t yourTZ.weekday(TIME);
uint8_t yourTZ.month(TIME);
uint16_t yourTZ.year(TIME);
```

```
uint8_t weekISO(TIME);
uint16_t yearISO(TIME);
```

```
bool time.secondChanged();
bool time.minuteChanged();
```

## Events

`uint8_t setEvent(void (*function)(), TIME)`

`uint8_t setEvent(void (*function)(), const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t mnth, uint16_t yr)`

`void time.deleteEvent(uint8_t event_handle)`

`void time.deleteEvent(void (*function)())`

## Setting date and time manually

![](images/setting-clock.jpg)

### yourTZ.setTime

`void yourTZ.setTime(time_t t, uint16_t ms = 0)`

`void yourTZ.setTime(const uint8_t hr, const uint8_t min, const uint8_t sec,`<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`const uint8_t day, const uint8_t mnth, uint16_t yr)`

`setTime` pretty much does what it says on the package: it sets the time to the time specified, aither as separate elements or as a time_t value in seconds since Jan 1st 1970. If you have another source of time  &mdash; say, a GPS receiver &mdash; you can use `setTime` to set the time in the UTC timezone. Or you can set the local time in any other timezone you have set up and ezTime will set it's internal offset to the corresponding time in UTC so all timezones stay at the correct time.

It's important to realise however that NTP updates will still become due and when they do time will be set to the time returned by the NTP server. If you do not want that, you can turn off NTP updates with `eztime.setInterval()`. If you do not use NTP updates at all and do not use the network lookups for timezone information either, you can compile ezTime with no network support by commenting out `#define EZTIME_NETWORK_ENABLE` in the `ezTime.h` file, creating a smaller library.

## Working with time values

### time.breakTime

`void time.breakTime(time_t time, tmElements_t &tm)`

If you create a `tmElements_t` structure and pass it to `breakTime`, it will be filled with the various numeric elements of the time value specified. tmElements_t looks as follows:

```
typedef struct  { 
	uint8_t Second; 
	uint8_t Minute; 
	uint8_t Hour; 
	uint8_t Wday;   // day of week, sunday is day 1
	uint8_t Day;
	uint8_t Month; 
	uint8_t Year;   // offset from 1970; 
} tmElements_t;
```

Meaning this code would print the hour:

```
tmElements_t tm;
time.breakTime(UTC.now(), tm);
Serial.print(tm.Hour);
```

But `Serial.println(UTC.hour())` also works and is much simpler. `breakTime` is used internally and is a part of the original Time library, so it is available for you to use. Mind that the year is a single byte value, years since 1970.

&nbsp;

### time.makeTime

`time_t time.makeTime(tmElements_t &tm);`

This does the opposite of `time.breakTime`: it takes a tmElements_t structure and turns it into a time_t value in seconds since Jan 1st 1970.

`time_t time.makeTime(uint8_t hour, uint8_t minute, uint8_t second,`<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`uint8_t day, uint8_t month, int16_t year);`

This version takes the various numeric elements as arguments. Note that you can pass the year both as years since 1970 and as full four digit years. 

&nbsp;

### makeOrdinalTime

`time_t time.makeOrdinalTime(uint8_t hour, uint8_t minute, uint8_t second,`<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`uint8_t ordinal, uint8_t wday, uint8_t month, int16_t year);`

With `makeOrdinalTime` you can get the `time_t` value for a date written as "the second Tuesday in March". The `ordinal` value is 1 for first, 2 for second, 3 for third, 4 for fourth and either 5 or 0 for the last of that weekday in the month. `wday` is weekdays starting with Sunday as 1. You can use the names of ordinals, months and weekdays in all caps as they are compiler defines. So the following would find the `time_t` value for midnight at the start of the first Thursday of the year in variable `year`.

```
time.makeOrdinalTime(0, 0, 0, FIRST, THURSDAY, JANUARY, year)
```

> *This is actually a fragment of ezTime's own code, as it can print ISO week numbers and the first ISO week in a year is defined as the week that has the first Thursday in it.*

&nbsp;

### compileTime

`time_t time.compileTime(String compile_date = __DATE__, String compile_time = __TIME__);`

&nbsp;

### *tzTime*

`time_t yourTZ.tzTime(TIME)`

This is the internal workhorse function that converts `time_t` in UTC to `time_t` in a timezone or vice versa. It is used by almost all the functions that apply to a timezone, and it takes `TIME` &mdash; meaning nothing for "right now", or a `time_t` value and an optional argument to specify whether that is `LOCAL_TIME` or `UTC_TIME`, and then it will convert to the opposite. `TIME_NOW` and `LAST_READ` are always output as `time_t` in that timezone.

`time_t yourTZ.tzTime(time_t t, ezLocalOrUTC_t local_or_utc, String &tzname, bool &is_dst, int16_t &offset)`

In this second form you have to supply all arguments, and it will fill your `tzname`, `is_dst` and `offset` variables with the appropriate values, the offset is in minutes west of UTC. Note that there are easier functions for you to get this information: `getTimezoneName`, `isDST` and `getOffset` respectively. If your code calls all three in a tight loop you might consider using `tzTime` instead as the other functions each do the whole parsing using `tzTime`, so you would be calling it three times and it does quite a bit. 

&nbsp;

## Various functions

`String urlEncode(String str);`

`String zeropad(uint32_t number, uint8_t length);`


## Errors and debug information

### time.debugLevel

`void time.debugLevel(ezDebugLevel_t level);`

Sets the level of detail at which ezTime outputs messages on the serial port. Can be set to one of:

| debugLevel  | effect  |
|---|---|
| `NONE` | ezTime does not output anything on the serial port |
| `ERROR` | ezTime will show when errors occur. Note that these may be transient errros that ezTime recovers from, such as NTP timeouts. |
| `INFO`  | Essentially shows you what ezTime is doing in the background. Includes messages about NTP updates, initialising timezones, etc etc. |
| `DEBUG`  | Detailed debugging information unlikely to be of much use unless you are trying to get to the bottom of certian internal behaviour of ezTime.  |

*Note:* you can specify which level of debug information whould be compiled into the library. This is especially significant for AVR Arduino users that need to limit the flashindia and RAM footprint of ezTtime. See the "Running on small Arduinos" chapter further down. TODO

&nbsp;

### time.error

`ezError_t time.error();`

A number of functions in ezTime are booleans, meaning they return `true` or `false` as their return value, where `false` means some error ocurred. `time.error` will return an `ezError_t` enumeration, something like `NO_NETWORK` (obvious) or `LOCKED_TO_UTC` (when you try to load some new timezone info to the UTC object). You can test for these specific errors and this document will mention which errors might happen in what functions.

When you call `error()`, it will also reset the error, so you can clear the last error stored.

&nbsp;

### time.errorString

`String time.errorString(ezError_t err = LAST_ERROR);`

This will give you a string representation of the error specified. The pseudo-error `LAST_ERROR`, which is the default, will give you the textual representation of the last error. This will not reset the last error stored.

&nbsp;

## Compatibility with Arduino Time library

## Smaller footprint, AVR Arduinos

## 2036 and 2038

The NTP timestamps used here run until the 7th of February 2036. NTP itself has 128 bits of time precision, I haven't looked into it much. Didn't have to, because just a little later, on the 19th of January 2038, the time_t 32 bit signed integer overflows. This is 20 years from today, in 2018. The Arduino world, if it still exists around then, will have come together around some solution that probably involves 64-bit time like in many operating systems of 2018. If you use this library in your nuclear generating station (**NOOOOO!**), make sure you're not around when these timers wrap around.

Should you be the one doing maintenance on this is some far-ish future: For ezTime I created another overflowing counter: the cache age for the timezone information is written as a single unsigned byte in months after January 2018, so that could theoretically cause problems in 2039, but I think everything will just roll over and use 2039 as the new anchor date.