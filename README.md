# ezTime Documentation

## A brief history of ezTime

## Getting started

## Some quick examples

# ezTime: complete documentation

## NTP, staying in sync

**`void ezTime.setServer(String ntp_server = NTP_SERVER)`**

**`void ezTime.setInterval(uint16_t seconds = 0)`**

**`bool ezTime.waitForSync(uint16_t timeout = 0)`**

**`void ezTime.updateNow()`**

**`timeStatus_t ezTime.timeStatus()`**

**`bool ezTime.queryNTP(String server, time_t &t, unsigned long &measured_at)`**



**`time_t ezTime.now()`**

**`void ezTime.breakTime(time_t time, tmElements_t &tm)`**

**`time_t ezTime.makeTime(tmElements_t &tm)`**

**`time_t ezTime.makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, int16_t year)`**

**`time_t ezTime.makeUmpteenthTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t umpteenth, uint8_t wday, uint8_t month, int16_t year)`**

## Timezones

		bool setPosix(String posix);
		String getPosix();
		bool setLocation(String location, bool force_lookup = false);
		String getOlsen();
		void setDefault();
		bool isDST_local(time_t t = TIME_NOW);
		bool isDST_UTC(time_t t = TIME_NOW);
		bool isDST();
		String getTimezoneName(time_t t = TIME_NOW);
		int32_t getOffset(time_t t = TIME_NOW);
		time_t now(bool update_last_read = true);
		void setTime(time_t t);
		void setTime(const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t mnth, uint16_t yr);
		String dateTime(String format = DEFAULT_TIMEFORMAT);				// http://php.net/manual/en/function.date.php for conversion
		String dateTime(time_t t, String format = DEFAULT_TIMEFORMAT);
		uint8_t hour(time_t t = TIME_NOW);			// 0-23
		uint8_t minute(time_t t = TIME_NOW);		// 0-59
		uint8_t second(time_t t = TIME_NOW);		// 0-59
		uint16_t ms(time_t t = TIME_NOW);			// 0-999
		uint8_t day(time_t t = TIME_NOW);			// 1-31
		uint8_t weekday(time_t t = TIME_NOW);		// Day of the week (1-7), Sunday is day 1
		uint8_t month(time_t t = TIME_NOW);			// 1-12
		uint16_t year(time_t t = TIME_NOW);			// four digit year		
		bool secondChanged();	// Since last call to something that caused a time read, to avoid excessive calling of, eg, timeString
		bool minuteChanged();

**`void ezTime.clearCache()`**

## Error handling, debug information

**`ezError_t ezTime.error()`**

**`String ezTime.errorString(ezError_t err)`**

**`void ezTime.debug(ezDebugLevel_t level)`**


## Free extras

		String urlEncode(String str); 															// Does what you think it does
		String zeropad(uint32_t number, uint8_t length);										// Returns number as string of given length, zero-padded on left if needed
		String getBetween(String &haystack, String before_needle, String after_needle = "");	// Returns what's between before_needle and after_needle in haystack, or "" if not found. Returns until end of string if after_needle is empty
		bool timezoneAPI(String location, String &olsen, String &posix);