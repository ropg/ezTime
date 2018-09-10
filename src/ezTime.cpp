#include <Arduino.h>

#include <ezTime.h>

#ifdef EZTIME_NETWORK_ENABLE
	#ifdef EZTIME_CACHE_NVS
		#include <Preferences.h>		// For timezone lookup cache
	#endif
	#ifdef EZTIME_CACHE_EEPROM
		#include <EEPROM.h>
	#endif	
	#if defined(ESP8266)
		#include <ESP8266WiFi.h>
		#include <WiFiUdp.h>
	#elif defined(EZTIME_ETHERNET)
		#include <SPI.h>
		#include <Ethernet.h>
		#include <EthernetUdp.h>
	#else
		#include <WiFi.h>
	#endif
#endif


const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0


ezError_t EZtime::_last_error = NO_ERROR;
ezDebugLevel_t EZtime::_debug_level = NONE;	
time_t EZtime::_last_sync_time = 0;
time_t EZtime::_last_read_t = 0;
uint32_t EZtime::_last_sync_millis = 0;
uint16_t EZtime::_last_read_ms;
timeStatus_t EZtime::_time_status = timeNotSet;
ezEvent_t EZtime::_events[MAX_EVENTS];
bool EZtime::_initialised = false;

#ifdef EZTIME_NETWORK_ENABLE
	bool EZtime::_ntp_enabled = true;
	uint16_t EZtime::_ntp_interval = NTP_INTERVAL;
	String EZtime::_ntp_server = NTP_SERVER;
#endif


EZtime::EZtime() {
	for (uint8_t n = 0; n < MAX_EVENTS; n++) EZtime::_events[n] = { 0, NULL };
}


////////// Error handing

String EZtime::errorString(ezError_t err /* = LAST_ERROR */) {
	switch (err) {
		case NO_ERROR: return				F("OK");
		case LAST_ERROR: return 			errorString(_last_error);
		case NO_NETWORK: return				F("No network");
		case TIMEOUT: return 				F("Timeout");
		case CONNECT_FAILED: return 		F("Connect Failed");
		case DATA_NOT_FOUND: return			F("Data not found");
		case LOCKED_TO_UTC: return			F("Locked to UTC");
		case NO_CACHE_SET: return			F("No cache set");
		case CACHE_TOO_SMALL: return		F("Cache too small");
		case TOO_MANY_EVENTS: return		F("Too many events");
		default: return						F("Unkown error");
	}
}

String EZtime::debugLevelString(ezDebugLevel_t level) {
	switch (level) {
		case NONE: return 	F("NONE");
		case ERROR: return 	F("ERROR");
		case INFO: return 	F("INFO");
		case DEBUG: return 	F("DEBUG");
	}
}

ezError_t EZtime::error() { 
	ezError_t tmp = _last_error;
	_last_error = NO_ERROR;
	return tmp;
}

void EZtime::error(ezError_t err) {
	_last_error = err;
	if (_last_error) {
		err(F("ERROR: "));
		errln(errorString(err));
	}
}

void EZtime::debugLevel(ezDebugLevel_t level) { 
	_debug_level = level;
	info(F("\r\nezTime debug level set to "));
	infoln(debugLevelString(level));
}

////////////////////////

String EZtime::monthString(uint8_t month) {
	switch(month) {
		case 1: return  F("January");
		case 2: return  F("February");		
		case 3: return  F("March");
		case 4: return  F("April");
		case 5: return  F("May");
		case 6: return  F("June");
		case 7: return  F("July");
		case 8: return  F("August");
		case 9: return  F("September");
		case 10: return F("October");
		case 11: return F("November");
		case 12: return F("December");
	}
	return "";
}

String EZtime::dayString(uint8_t day) {
	switch(day) {
		case 1: return F("Sunday");
		case 2: return F("Monday");
		case 3: return F("Tuesday");		
		case 4: return F("Wednesday");
		case 5: return F("Thursday");
		case 6: return F("Friday");
		case 7: return F("Saturday");
	}
	return "";
}


timeStatus_t EZtime::timeStatus() { return _time_status; }

time_t EZtime::now(bool update_last_read /* = true */) {
	time_t t;
	uint32_t m = millis();
	t = _last_sync_time + ((m - _last_sync_millis) / 1000);
	if (update_last_read) {
		_last_read_t = t;
		_last_read_ms = (m - _last_sync_millis) % 1000;
	}
	return t;
}

void EZtime::events() {
	#ifdef EZTIME_NETWORK_ENABLE
		if (!_initialised) {
			// Start the cycle of updateNTP running and then setting an event for its next run
			updateNTP();
			_initialised = true;
		}
	#endif
	// See if any events are due
	for (uint8_t n = 0; n < MAX_EVENTS; n++) {
		if (_events[n].function && now() >= _events[n].time) {
			debug(F("Running event (#")); debug(n + 1); debug(F(") set for ")); debugln(UTC.dateTime(_events[n].time));
			void (*tmp)() = _events[n].function;
			_events[n] = { 0, NULL };		// reset the event
			(tmp)();						// execute the function
		}
	}
}

void EZtime::deleteEvent(uint8_t event_handle) { 
	debug(F("Deleted event (#")); debug(event_handle); debug(F("), set for ")); debugln(UTC.dateTime(_events[event_handle - 1].time));	
	_events[event_handle - 1] = { 0, NULL };
}

void EZtime::deleteEvent(void (*function)()) { 
	for (uint8_t n = 0; n< MAX_EVENTS; n++) {
		if (_events[n].function == function) {
			debug(F("Deleted event (#")); debug(n + 1); debug(F("), set for ")); debugln(UTC.dateTime(_events[n].time));
			_events[n] = { 0, NULL };
		}
	}
}

void EZtime::breakTime(time_t timeInput, tmElements_t &tm){
	// break the given time_t into time components
	// this is a more compact version of the C library localtime function
	// note that year is offset from 1970 !!!

	uint8_t year;
	uint8_t month, monthLength;
	uint32_t time;
	unsigned long days;

	time = (uint32_t)timeInput;
	tm.Second = time % 60;
	time /= 60; // now it is minutes
	tm.Minute = time % 60;
	time /= 60; // now it is hours
	tm.Hour = time % 24;
	time /= 24; // now it is days
	tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1 

	year = 0;  
	days = 0;
	while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
		year++;
	}
	tm.Year = year; // year is offset from 1970 

	days -= LEAP_YEAR(year) ? 366 : 365;
	time  -= days; // now it is days in this year, starting at 0

	days=0;
	month=0;
	monthLength=0;
	for (month=0; month<12; month++) {
		if (month==1) { // february
			if (LEAP_YEAR(year)) {
				monthLength=29;
			} else {
				monthLength=28;
			}
		} else {
			monthLength = monthDays[month];
		}

		if (time >= monthLength) {
			time -= monthLength;
		} else {
			break;
		}
	}
	tm.Month = month + 1;  // jan is month 1  
	tm.Day = time + 1;     // day of month
}

time_t EZtime::makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint16_t year) {
	tmElements_t tm;
	tm.Hour = hour;
	tm.Minute = minute;
	tm.Second = second;
	tm.Day = day;
	tm.Month = month;
	if (year > 68) {			// time_t cannot reach beyond 68 + 1970 anyway, so if bigger user means actual years
		tm.Year = year - 1970;
	} else {
		tm.Year = year;
	}
	return makeTime(tm);
}

time_t EZtime::makeTime(tmElements_t &tm){
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
    
	int i;
	uint32_t seconds;

	// seconds from 1970 till 1 jan 00:00:00 of the given year
	seconds= tm.Year * SECS_PER_DAY * 365UL;

	for (i = 0; i < tm.Year; i++) {
		if (LEAP_YEAR(i)) {
		  seconds +=  SECS_PER_DAY;   // add extra days for leap years
		}
	}

	// add days for this year, months start from 1
	for (i = 1; i < tm.Month; i++) {
		if ( (i == 2) && LEAP_YEAR(tm.Year)) { 
		  seconds += SECS_PER_DAY * 29UL;
		} else {
		  seconds += SECS_PER_DAY * (uint32_t)monthDays[i-1];  //monthDay array starts from 0
		}
	}
	
	seconds+= (tm.Day-1) * SECS_PER_DAY;
	seconds+= tm.Hour * 3600UL;
	seconds+= tm.Minute * 60UL;
	seconds+= tm.Second;
		
	return (time_t)seconds; 
}

// makeOrdinalTime allows you to resolve "second thursday in September in 2018" into a number of seconds since 1970
// (Very useful for the timezone calculations that ezTime does internally) 
// If ordinal is 0 or 5 it is taken to mean "the last $wday in $month"
time_t EZtime::makeOrdinalTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t ordinal, uint8_t wday, uint8_t month, uint16_t year) {
	if (year <= 68 ) year = 1970 + year;		// fix user intent
	if (ordinal == 5) ordinal = 0;
	uint8_t m = month;   
	uint8_t w = ordinal;
	if (w == 0) {			// is this a "Last week" rule?
		if (++m > 12) {		// yes, for "Last", go to the next month
			m = 1;
			++year;
		}
		w = 1;               // and treat as first week of next month, subtract 7 days later
	}
	time_t t = makeTime(hour, minute, second, 1, m, year);
	// add offset from the first of the month to weekday, and offset for the given week
	t += ( (wday - UTC.weekday(t) + 7) % 7 + (w - 1) * 7 ) * SECS_PER_DAY;
	// back up a week if this is a "Last" rule
	if (ordinal == 0) t -= 7 * SECS_PER_DAY;
	return t;
}

String EZtime::urlEncode(String str) {
	String encodedString="";
	char c;
	char code0;
	char code1;
	char code2;
	for (int i = 0; i < str.length(); i++) {
		c = str.charAt(i);
		if (c == ' ') {
			encodedString += '+';
		} else if (isalnum(c)) {
			encodedString += c;
		} else {
			code1 = (c & 0xf)+'0';
			if ((c & 0xf) >9){
				code1 = (c & 0xf) - 10 + 'A';
			}
			c = (c >> 4) & 0xf;
			code0 = c + '0';
			if (c > 9) {
				code0 = c - 10 + 'A';
			}
			encodedString += '%';
			encodedString += code0;
			encodedString += code1;
		}
	}
	return encodedString;    
}

String EZtime::zeropad(uint32_t number, uint8_t length) {
	String out;
	out.reserve(length);
	out = String(number);
	while (out.length() < length) out = "0" + out;
	return out;
}

time_t EZtime::compileTime(String compile_date /* = __DATE__ */, String compile_time /* = __TIME__ */) {
	
	uint8_t hrs = compile_time.substring(0,2).toInt();
	uint8_t min = compile_time.substring(3,5).toInt();
	uint8_t sec = compile_time.substring(6).toInt();
	uint8_t day = compile_date.substring(4,6).toInt();
	int16_t year = compile_date.substring(7).toInt();
	String iterate_month;
	for (uint8_t month = 1; month < 13; month++) {
		iterate_month = monthString(month);
		if ( iterate_month.substring(0,3) == compile_date.substring(0,3) ) {
			return makeTime(hrs, min, sec, day, month, year);
		}
	}
	return 0;
}

bool EZtime::secondChanged() {
	time_t t = now(false);
	if (_last_read_t != t) return true;
	return false;
}

bool EZtime::minuteChanged() {
	time_t t = now(false);
	if (_last_read_t / 60 != t / 60) return true;
	return false;
}


#ifdef EZTIME_NETWORK_ENABLE

	void EZtime::updateNTP() {
		time.deleteEvent(updateNTP);	// Delete any events pointing here, in case called manually
		time_t t;
		unsigned long measured_at;
		if (queryNTP(_ntp_server, t, measured_at)) {
			int32_t correction = ( (t - _last_sync_time) * 1000 ) - ( measured_at - _last_sync_millis );
			_last_sync_time = t;
			_last_sync_millis = measured_at;
			_last_read_ms = ( millis() - measured_at) % 1000;
			info(F("Received time: "));
			info(UTC.dateTime(_last_sync_time, F("l, d-M-y H:i:s.v T")));
			if (_time_status != timeNotSet) {
				info(F(" (internal clock was "));
				if (!correction) {
					infoln(F("spot on)"));
				} else {
					info(String(abs(correction)));
					if (correction > 0) {
						infoln(F(" ms fast)"));
					} else {
						infoln(F(" ms slow)"));
					}
				}
			} else {
				infoln("");
			}
			if (_ntp_interval) UTC.setEvent(updateNTP, t + _ntp_interval);
			_time_status = timeSet;
		} else {
			UTC.setEvent(updateNTP, now() + NTP_RETRY);
		}
	}

	// This is a nice self-contained NTP routine if you need one: feel free to use it.
	// It gives you the seconds since 1970 (unix epoch) and the millis() on your system when 
	// that happened (by deducting fractional seconds and estimated network latency).
	bool EZtime::queryNTP(String server, time_t &t, unsigned long &measured_at) {
		info(F("Querying "));
		info(server);
		info(F(" ... "));

		#ifndef EZTIME_ETHERNET
			if (!WiFi.isConnected()) { error(NO_NETWORK); return false; }
			WiFiUDP udp;
		#else
			EthernetUDP udp;
		#endif
	
		udp.flush();
		udp.begin(NTP_LOCAL_TIME_PORT);
	
		// Send NTP packet
		byte buffer[NTP_PACKET_SIZE];
		memset(buffer, 0, NTP_PACKET_SIZE);
		buffer[0] = 0b11100011;		// LI, Version, Mode
		buffer[1] = 0;   			// Stratum, or type of clock
		buffer[2] = 9;				// Polling Interval (9 = 2^9 secs = ~9 mins, close to our 10 min default)
		buffer[3] = 0xEC;			// Peer Clock Precision
									// 8 bytes of zero for Root Delay & Root Dispersion
		buffer[12]  = 'X';			// "kiss code", see RFC5905
		buffer[13]  = 'E';			// (codes starting with 'X' are not interpreted)
		buffer[14]  = 'Z';
		buffer[15]  = 'T';
		udp.beginPacket(_ntp_server.c_str(), 123); //NTP requests are to port 123
		udp.write(buffer, NTP_PACKET_SIZE);
		udp.endPacket();

		// Wait for packet or return false with timed out
		unsigned long started = millis();
		uint16_t packetsize = 0;
		while (!udp.parsePacket()) {
			delay (1);
			if (millis() - started > NTP_TIMEOUT) {
				udp.stop();	
				error(TIMEOUT); 
				return false;
			}
		}
		udp.read(buffer, NTP_PACKET_SIZE);
		uint32_t highWord, lowWord;
		highWord = ( buffer[40] << 8 | buffer[41] ) & 0x0000FFFF;	// Must be done in two steps on AVR
		lowWord = ( buffer[42] << 8 | buffer[43] ) & 0x0000FFFF;
		uint32_t secsSince1900 = highWord << 16 | lowWord;
		// Set the t and measured_at variables that were passed by reference
		uint32_t done = millis();
		info(F("success (round trip ")); info(done - started); infoln(F(" ms)"));
		t = secsSince1900 - 2208988800UL;					// Subtract 70 years to get seconds since 1970
		highWord = ( buffer[44] << 8 | buffer[45] ) & 0x0000FFFF;
		lowWord = ( buffer[46] << 8 | buffer[47] ) & 0x0000FFFF;
		uint32_t fraction = highWord << 16 | lowWord;		// Must be done via two words on AVR 
		uint16_t ms = fraction / 4294967UL;					// Turn 32 bit fraction into ms by dividing by 2^32 / 1000 
		measured_at = done - ((done - started) / 2) - ms;	// Assume symmetric network latency and return when we think the whole second was.
		udp.stop();											// On AVR there's only very limited sockets, we want to free them when done.
		return true;
	}

	void EZtime::setInterval(uint16_t seconds /* = 0 */) { 
		deleteEvent(updateNTP);
		_ntp_interval = seconds;
		if (seconds) UTC.setEvent(updateNTP, now() + _ntp_interval);
	}

	void EZtime::setServer(String ntp_server /* = NTP_SERVER */) { _ntp_server = ntp_server; }

	bool EZtime::waitForSync(uint16_t timeout /* = 0 */) {

		unsigned long start = millis();
		
		#ifndef EZTIME_ETHERNET
			if (!WiFi.isConnected()) {
				info(F("Waiting for WiFi ... "));
				while (!WiFi.isConnected()) {
					if ( timeout && (millis() - start) / 1000 > timeout ) { error(TIMEOUT); return false;};
					delay(25);
				}
				infoln(F("connected"));
			}
		#endif

		if (!_time_status != timeSet) {
			infoln(F("Waiting for time sync"));
			while (_time_status != timeSet) {
				if ( timeout && (millis() - start) / 1000 > timeout ) { error(TIMEOUT); return false;};
				delay(250);
				events();
			}
			infoln(F("Time is in sync"));
		}		
	
	}
	
#endif // EZTIME_NETWORK_ENABLE


EZtime time;






//
// Timezone class
//

Timezone::Timezone(bool locked_to_UTC /* = false */) {
	_locked_to_UTC = locked_to_UTC;
	_posix = "UTC";
	#ifdef EZTIME_CACHE_EEPROM
		_cache_month = 0;
		_eeprom_address = -1;
	#endif
	#ifdef EZTIME_CACHE_NVS
		_cache_month = 0;
		_nvs_name = "";
		_nvs_key = "";
	#endif
}

bool Timezone::setPosix(String posix) {
	if (_locked_to_UTC) { time.error(LOCKED_TO_UTC); return false; }
	_posix = posix;
	_olsen = "";
}

time_t Timezone::tzTime(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME) {
	String tzname;
	bool is_dst;
	int16_t offset;
	return tzTime(t, local_or_utc, tzname, is_dst, offset);
}

time_t Timezone::tzTime(time_t t, ezLocalOrUTC_t local_or_utc, String &tzname, bool &is_dst, int16_t &offset) {

	if (t == TIME_NOW) {
		t = time.now(); 
		local_or_utc = UTC_TIME;
	} else if (t == LAST_READ) {
		t = time._last_read_t;
		local_or_utc = UTC_TIME;
	}
	
	int8_t offset_hr = 0;
	uint8_t offset_min = 0;
	int8_t dst_shift_hr = 1;
	uint8_t dst_shift_min = 0;
	
	uint8_t start_month = 0, start_week = 0, start_dow = 0, start_time_hr = 2, start_time_min = 0;
	uint8_t end_month = 0, end_week = 0, end_dow = 0, end_time_hr = 2, end_time_min = 0;
	
	enum posix_state_e {STD_NAME, OFFSET_HR, OFFSET_MIN, DST_NAME, DST_SHIFT_HR, DST_SHIFT_MIN, START_MONTH, START_WEEK, START_DOW, START_TIME_HR, START_TIME_MIN, END_MONTH, END_WEEK, END_DOW, END_TIME_HR, END_TIME_MIN};
	posix_state_e state = STD_NAME; 

	bool ignore_nums = false;
	char c = 1; // Dummy value to get while(newchar) started
	uint8_t strpos = 0;
	uint8_t stdname_end = _posix.length() - 1;
	uint8_t dstname_begin = _posix.length();
	uint8_t dstname_end = _posix.length();

	while (strpos < _posix.length()) {
		c = (char)_posix[strpos];

		// Do not replace the code below with switch statement: evaluation of state that 
		// changes while this runs. (Only works because this state can only go forward.)

		if (c && state == STD_NAME) {
			if (c == '<') ignore_nums = true;
			if (c == '>') ignore_nums = false;
			if (!ignore_nums && (isDigit(c) || c == '-'  || c == '+')) {
				state = OFFSET_HR;
				stdname_end = strpos - 1;
			}
		}
		if (c && state == OFFSET_HR) {
			if (c == '+') {
				// Ignore the plus
			} else if (c == ':') {
				state = OFFSET_MIN;
				c = 0;
			} else if (c != '-' && !isDigit(c)) {
				state = DST_NAME;
				dstname_begin = strpos;
			} else {
				if (!offset_hr) offset_hr = atoi(_posix.c_str() + strpos);
			}
		}			
		if (c && state == OFFSET_MIN) {
			if (!isDigit(c)) {
				state = DST_NAME;
				ignore_nums = false;
			} else {
				if (!offset_min) offset_min = atoi(_posix.c_str() + strpos);
			}
		}				
		if (c && state == DST_NAME) {
			if (c == '<') ignore_nums = true;
			if (c == '>') ignore_nums = false;
			if (c == ',') {
				state = START_MONTH;
				c = 0;
				dstname_end = strpos - 1;
			} else if (!ignore_nums && (c == '-' || isDigit(c))) {
				state = DST_SHIFT_HR;
				dstname_end = strpos - 1;
			}
		}		
		if (c && state == DST_SHIFT_HR) {
			if (c == ':') {
				state = DST_SHIFT_MIN;
				c = 0;
			} else if (c == ',') {
				state = START_MONTH;
				c = 0;
			} else if (dst_shift_hr == 1) dst_shift_hr = atoi(_posix.c_str() + strpos);
		}			
		if (c && state == DST_SHIFT_MIN) {
			if (c == ',') {
				state = START_MONTH;
				c = 0;
			} else if (!dst_shift_min) dst_shift_min = atoi(_posix.c_str() + strpos);
		}			
		if (c && state == START_MONTH) {
			if (c == '.') {
				state = START_WEEK;
				c = 0;
			} else if (c != 'M' && !start_month) start_month = atoi(_posix.c_str() + strpos);	
		}			
		if (c && state == START_WEEK) {
			if (c == '.') {
				state = START_DOW;
				c = 0;
			} else start_week = c - '0';
		}		
		if (c && state == START_DOW) {
			if (c == '/') {
				state = START_TIME_HR;
				c = 0;
			} else if (c == ',') {
				state = END_MONTH;
				c = 0;
			} else start_dow = c - '0';				
		}
		if (c && state == START_TIME_HR) {
			if (c == ':') {
				state = START_TIME_MIN;
				c = 0;
			} else if (c == ',') {
				state = END_MONTH;
				c = 0;
			} else if (start_time_hr == 2) start_time_hr = atoi(_posix.c_str() + strpos);
		}		
		if (c && state == START_TIME_MIN) {
			if (c == ',') {
				state = END_MONTH;
				c = 0;
			} else if (!start_time_min) start_time_min = atoi(_posix.c_str() + strpos);
		}		
		if (c && state == END_MONTH) {
			if (c == '.') {
				state = END_WEEK;
				c = 0;
			} else if (c != 'M') if (!end_month) end_month = atoi(_posix.c_str() + strpos);
		}			
		if (c && state == END_WEEK) {
			if (c == '.') {
				state = END_DOW;
				c = 0;
			} else end_week = c - '0';
		}		
		if (c && state == END_DOW) {
			if (c == '/') {
				state = END_TIME_HR;
				c = 0;			
			} else end_dow = c - '0';
		}
		if (c && state == END_TIME_HR) {
			if (c == ':') {
				state = END_TIME_MIN;
				c = 0;
			}  else if (end_time_hr == 2) end_time_hr = atoi(_posix.c_str() + strpos);
		}		
		if (c && state == END_TIME_MIN) {
			if (!end_time_min) end_time_min = atoi(_posix.c_str() + strpos);
		}
		strpos++;
	}	
	
	int16_t std_offset = (offset_hr < 0) ? offset_hr * 60 - offset_min : offset_hr * 60 + offset_min;
	
	tzname = _posix.substring(0, stdname_end + 1);	// Overwritten with dstname later if needed
	if (!start_month) {
		if (tzname == "UTC" && std_offset) tzname = "???";
		is_dst = false;
		offset = std_offset;
		return t - std_offset * 60;
	}

	int16_t dst_offset = std_offset - dst_shift_hr * 60 - dst_shift_min;
	// to find the year
	tmElements_t tm;
	time.breakTime(t, tm);	
	
	// in local time
	time_t dst_start = time.makeOrdinalTime(start_time_hr, start_time_min, 0, start_week, start_dow, start_month, tm.Year + 1970);
	time_t dst_end = time.makeOrdinalTime(end_time_hr, end_time_min, 0, end_week, end_dow, end_month, tm.Year + 1970);
	
	if (local_or_utc == UTC_TIME) {
		dst_start -= std_offset;
		dst_end -= dst_offset;
	}
	
    if (dst_end > dst_start) {
        is_dst = (t >= dst_start && t < dst_end);		// northern hemisphere
    } else {
        is_dst = !(t >= dst_end && t < dst_start);		// southern hemisphere
    }

	if (is_dst) {
		offset = dst_offset;
		tzname = _posix.substring(dstname_begin, dstname_end + 1);
	} else {
		offset = std_offset;
	}

	if (local_or_utc == LOCAL_TIME) {
		return t + offset * 60LL;
	} else {
		return t - offset * 60LL;
	}
}

String Timezone::getPosix() { return _posix; }

#ifdef EZTIME_NETWORK_ENABLE

	bool Timezone::setLocation(String location /* = "" */) {
	
		info(F("Timezone lookup for: "));
		infoln(location);
		if (_locked_to_UTC) { time.error(LOCKED_TO_UTC); return false; }
		
		#ifndef EZTIME_ETHERNET
			if (!WiFi.isConnected()) { time.error(NO_NETWORK); return false; }
		#endif

		String path;
		if (location.indexOf("/") != -1) { 
			path = F("/api/timezone/?"); path += time.urlEncode(location);
		} else if (location != "") {
			path = F("/api/address/?"); path += time.urlEncode(location);
		} else {
			path = F("/api/ip");
		}

		#ifndef EZTIME_ETHERNET
			WiFiClient client;
		#else
			EthernetClient client;
		#endif	

		if (!client.connect("timezoneapi.io", 80)) { time.error(CONNECT_FAILED);	return false; }

		client.print(F("GET "));
		client.print(path);
		client.println(F(" HTTP/1.1"));
		client.println(F("Host: timezoneapi.io"));
		client.println(F("Connection: close"));
		client.println();
		client.setTimeout(3000);
		
		debug(F("Sent request for http://timezoneapi.io")); debugln(path);
		debugln(F("Reply from server:\r\n"));
		
		// This "JSON parser" (bwahaha!) fits in the small memory of the AVRs
		String tzinfo = "";
		String needle = "\"id\":\"";
		uint8_t search_state = 0;
		uint8_t char_found = 0;
		uint32_t start = millis();
		while ( search_state < 4  && millis() - start < TIMEZONEAPI_TIMEOUT) {
			if (client.available()) {
				char c = client.read();
				debug(c);
				if (c == needle.charAt(char_found)) {
					char_found++;
					if (char_found == needle.length()) {
						search_state++;
						c = 0;
					}
				} else {
					char_found = 0;
				}
				if (search_state == 1 || search_state == 3) {
					if (c == '"') {
						search_state++;
						if (search_state == 2) {
							needle = "\"tz_string\":\"";
							tzinfo += ' ';
						}
					} else if (c && c != '\\') {
						tzinfo += c;
					}
				}
			}
		}
		debugln(F("\r\n\r\n"));
		if (search_state != 4 || tzinfo == "") { time.error(DATA_NOT_FOUND); return false; }
		
		infoln(F("success."));
		info(F("Found: ")); infoln(tzinfo);

		#if defined(EZTIME_CACHE_EEPROM) || defined(EZTIME_CACHE_NVS)
			writeCache(tzinfo);
		#endif		

		_posix = tzinfo.substring(tzinfo.indexOf(' ') + 1);
		return true;
	}

	#ifdef EZTIME_CACHE_EEPROM
		bool Timezone::setCache(const int16_t address) {
			if (address + EEPROM_CACHE_LEN > EEPROM.length()) { time.error(CACHE_TOO_SMALL); return false; }
			_eeprom_address = address;
			return setCache();
		}
	#endif
	
	#ifdef EZTIME_CACHE_NVS
		bool Timezone::setCache(const String name, const String key) {
			_nvs_name = name;
			_nvs_key = key;
			return setCache();
		}
	#endif
	
	#if defined(EZTIME_CACHE_EEPROM) || defined(EZTIME_CACHE_NVS)

		bool Timezone::setCache() {
			String olsen, posix;
			uint8_t months_since_jan_2018;
			if (readCache(olsen, posix, months_since_jan_2018)) {
				_posix = posix;
				_olsen = olsen;
				_cache_month = months_since_jan_2018;
				if ( (year() - 2018) * 12 + month(LAST_READ) - months_since_jan_2018 > MAX_CACHE_AGE_MONTHS) {
					setLocation(olsen);
				}
				return true;
			}
			return false;
		}
		
		void Timezone::clearCache(bool delete_section /* = false */) {
		
			#ifdef EZTIME_CACHE_EEPROM
				if (_eeprom_address < 0) { time.error(NO_CACHE_SET); return; }
				for (int16_t n = _eeprom_address; n < _eeprom_address + EEPROM_CACHE_LEN; n++) EEPROM.write(n, 0);
			#endif

			#ifdef EZTIME_CACHE_NVS
				if (_nvs_name = "" || _nvs_key = "") { time.error(NO_CACHE_SET); return; }
				Preferences prefs;
				prefs.begin(_nvs_name, false);
				if (delete_section) {
					prefs.clear();
				} else {
					prefs.remove(_nvs_key);
				}
				prefs.end();
			#endif
		}

		String Timezone::getOlsen() {
			return _olsen;
		}	

		bool Timezone::writeCache(const String &str) {
			uint8_t months_since_jan_2018 = 0;
			if (year() >= 2018) months_since_jan_2018 = (year(LAST_READ) - 2018) * 12 + month(LAST_READ) - 1;

			#ifdef EZTIME_CACHE_EEPROM
				if (_eeprom_address < 0) return false;
				info(F("Caching timezone data  "));
				if (str.length() > MAX_CACHE_PAYLOAD) { time.error(CACHE_TOO_SMALL); return false; }
				
				uint16_t last_byte = _eeprom_address + EEPROM_CACHE_LEN - 1;	
				uint16_t addr = _eeprom_address;
				
				// First byte is cache age, in months since 2018
				EEPROM.write(addr++, months_since_jan_2018);
				
				// Second byte is length of payload
				EEPROM.write(addr++, str.length());
				
				// Followed by payload, compressed. Every 4 bytes to three by encoding only 6 bits, ASCII all-caps
				str.toUpperCase();
				uint8_t store = 0;
				for (uint8_t n = 0; n < str.length(); n++) {
					unsigned char c = str.charAt(n) - 32;
					if ( c > 63) c = 0;
					switch (n % 4) {
						case 0:
							store = c << 2;					//all of 1st
							break;
						case 1:
							store |= c >> 4;				//high two of 2nd
							EEPROM.write(addr++, store);	 
							store = c << 4;					//low four of 2nd
							break;
						case 2:
							store |= c >> 2;				//high four of 3rd
							EEPROM.write(addr++, store);
							store = c << 6;					//low two of third
							break;
						case 3:
							store |= c;						//all of 4th
							EEPROM.write(addr++, store);
							store = 0;
					}
				}
				if (store) EEPROM.write(addr++, store);
				
				// Fill rest of cache (except last byte) with zeroes
				for (; addr < last_byte; addr++) EEPROM.write(addr, 0);

				// Add all bytes in cache % 256 and add 42, that is the checksum written to last byte.
				// The 42 is because then checksum of all zeroes then isn't zero.
				uint8_t checksum = 0;
				for (uint16_t n = _eeprom_address; n < last_byte; n++) checksum += EEPROM.read(n);
				checksum += 42;
				EEPROM.write(last_byte, checksum);
				infoln();
				return true;
			#endif
			
			#ifdef EZTIME_CACHE_NVS
				if (_nvs_name = "" || _nvs_key = "") return false;
				infoln(F("Caching timezone data"));
				Preferences prefs;
				prefs.begin(_nvs_name, false);
				prefs.putString(_nvs_key, String(months_since_jan_2018) + " " + str);
				prefs.end();
				return true;
			#endif
		}
	

		bool Timezone::readCache(String &olsen, String &posix, uint8_t &months_since_jan_2018) {

			#ifdef EZTIME_CACHE_EEPROM
				if (_eeprom_address < 0) { time.error(NO_CACHE_SET); return false; }
				
				uint16_t last_byte = _eeprom_address + EEPROM_CACHE_LEN - 1;			
				
				for (uint16_t n = _eeprom_address; n <= last_byte; n++) {
					debug(n);
					debug(F(" "));
					debugln(EEPROM.read(n), HEX);
				}
				
				// return false if checksum incorrect
				uint8_t checksum = 0;
				for (uint16_t n = _eeprom_address; n < last_byte; n++) checksum += EEPROM.read(n);
				checksum += 42;				
				if (checksum != EEPROM.read(last_byte)) return false;
				debugln(F("Checksum OK"));
				
				// Return false if length impossible
				uint8_t len = EEPROM.read(_eeprom_address + 1);
				debug("Length: "); debugln(len);
				if (len > MAX_CACHE_PAYLOAD) return false;
				
				// OK, we're gonna decompress
				olsen.reserve(len + 3);		// Everything goes in olsen first. Decompression might overshoot 3 
				months_since_jan_2018 = EEPROM.read(_eeprom_address);
				uint8_t outlen = 0;
				uint8_t n = 0;
				
				for (uint8_t n = 0; n < EEPROM_CACHE_LEN - 3; n++) {
					uint16_t addr = n + _eeprom_address + 2;
					uint8_t c = EEPROM.read(addr);
					uint8_t p = EEPROM.read(addr - 1);	// previous byte
					switch (n % 3) {
						case 0:
							olsen += (char)( ((c & 0b11111100) >> 2) + 32 );
							break;
						case 1:
							olsen += (char)( ((p & 0b00000011) << 4) + ((c & 0b11110000) >> 4) + 32 );
							break;
						case 2:
							olsen += (char)( ((p & 0b00001111) << 2) + ((c & 0b11000000) >> 6) + 32 );
							olsen += (char)( (c & 0b00111111) + 32 );
					}
					if (olsen.length() >= len) break;
				}
				
				uint8_t first_space = olsen.indexOf(' ');
				posix = olsen.substring(first_space + 1, len);
				olsen = olsen.substring(0, first_space);
				
				// Restore case of olsen (best effort)
				String olsen_lowercase = olsen;
				olsen_lowercase.toLowerCase();
				for (uint8_t n = 1; n < olsen.length(); n++) {
					unsigned char p = olsen.charAt(n - 1);	// previous character
					if (p != '_' && p != '/' && p != '-') {
						olsen.setCharAt(n, olsen_lowercase[n]);
					}
				}
				info(F("Cache read. Olsen: ")); info(olsen); info (F("  Posix: ")); infoln(posix);
				return true;
			#endif						
			
			#ifdef EZTIME_CACHE_NVS
				if (_nvs_name = "" || _nvs_key = "") { time.error(NO_CACHE_SET); return; }
				
				Preferences prefs;
				prefs.begin(_nvs_name, true);
				String olsen = prefs.getString(_nvs_key);
				prefs.end();
				if (!olsen) return false;
				
				uint8_t first_space = olsen.indexOf(' ');
				uint8_t second_space = olsen.indexOf(' ', first_space + 1);
				months_since_jan_2018 = olsen.toInt();
				posix = olsen.substring(second_space + 1);
				olsen = substring(olsen, first_space + 1, second_space);
				info(F("Cache read. Olsen: ")); info(olsen); info (F("  Posix: ")); infoln(posix);
				return true;
			#endif
		}
		
	#endif	// defined(EZTIME_CACHE_EEPROM) || defined(EZTIME_CACHE_NVS)


#endif // EZTIME_NETWORK_ENABLE


void Timezone::setDefault() {
	defaultTZ = this;
}

bool Timezone::isDST(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	String tzname;
	bool is_dst;
	int16_t offset;
	t = tzTime(t, local_or_utc, tzname, is_dst, offset);
	return is_dst;
}

String Timezone::getTimezoneName(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	String tzname;
	bool is_dst;
	int16_t offset;
	t = tzTime(t, local_or_utc, tzname, is_dst, offset);
	return tzname;
}

int16_t Timezone::getOffset(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	String tzname;
	bool is_dst;
	int16_t offset;
	t = tzTime(t, local_or_utc, tzname, is_dst, offset);
	return offset;
}

uint8_t Timezone::setEvent(void (*function)(), const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t mnth, uint16_t yr) {
	time_t t = time.makeTime(hr, min, sec, day, mnth, yr);
	return setEvent(function, t);
}

uint8_t Timezone::setEvent(void (*function)(), time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME) {
	t = tzTime(t, local_or_utc);
	for (uint8_t n = 0; n < MAX_EVENTS; n++) {
		if (!time._events[n].function) {
			time._events[n].function = function;
			time._events[n].time = t;
			debug(F("Set event (#")); debug(n + 1); debug(F(") to trigger on: ")); debugln(UTC.dateTime(t));
			return n + 1;
		}
	}
	time.error(TOO_MANY_EVENTS);
	return 0;
}

void Timezone::setTime(time_t t, uint16_t ms /* = 0 */) {
	int16_t offset;
	offset = getOffset(t);
	time._last_sync_time = t + offset * 60;
	time._last_sync_millis = millis() - ms;
	time._time_status = timeSet;
}

void Timezone::setTime(const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t mnth, uint16_t yr) {
	tmElements_t tm;
	// year can be given as full four digit year or two digts (2010 or 10 for 2010);  
	// it is converted to years since 1970
	if( yr > 99) {
		yr = yr - 1970;
	} else {
		yr += 30; 
	}
	tm.Year = yr;
	tm.Month = mnth;
	tm.Day = day;
	tm.Hour = hr;
	tm.Minute = min;
	tm.Second = sec;
	setTime(time.makeTime(tm));
}

String Timezone::dateTime(String format /* = DEFAULT_TIMEFORMAT */) {
	return dateTime(TIME_NOW, format);
}

String Timezone::dateTime(time_t t, String format /* = DEFAULT_TIMEFORMAT */) {
	return dateTime(t, LOCAL_TIME, format);
}

String Timezone::dateTime(time_t t, ezLocalOrUTC_t local_or_utc, String format /* = DEFAULT_TIMEFORMAT */) {

	String tzname;
	bool is_dst;
	int16_t offset;
	t = tzTime(t, LOCAL_TIME, tzname, is_dst, offset);

	String tmpstr;
	uint8_t tmpint8;
	String out = "";

	tmElements_t tm;
	time.breakTime(t, tm);

	int8_t hour12 = tm.Hour % 12;
	if (hour12 == 0) hour12 = 12;
	
	int32_t o;

	bool escape_char = false;
	
	for (int8_t n = 0; n < format.length(); n++) {
	
		char c = (char) format.c_str()[n];
		
		if (escape_char) {
			out += String(c);
			escape_char = false;
		} else {
		
			switch (c) {
		
				case '\\':	// Escape character, ignore this one, and let next through as literal character
				case '~':	// Same but easier without all the double escaping
					escape_char = true;
					break;
				case 'd':	// Day of the month, 2 digits with leading zeros
					out += time.zeropad(tm.Day, 2);
					break;
				case 'D':	// A textual representation of a day, three letters
					out += time.dayString(tm.Wday).substring(0,3);
					break;
				case 'j':	// Day of the month without leading zeros
					out += String(tm.Day);
					break;
				case 'l':	// (lowercase L) A full textual representation of the day of the week
					out += time.dayString(tm.Wday);
					break;
				case 'N':	// ISO-8601 numeric representation of the day of the week. ( 1 = Monday, 7 = Sunday )
					tmpint8 = tm.Wday - 1;
					if (tmpint8 == 0) tmpint8 = 7;
					out += String(tmpint8);
					break;
				case 'S':	// English ordinal suffix for the day of the month, 2 characters (st, nd, rd, th)
					switch (tm.Day) {
						case 1:
						case 21:
						case 31:
							out += F("st"); break;
						case 2:
						case 22:
							out += F("nd"); break;
						case 3:
						case 23:
							out += F("rd"); break;
						default:
							out += F("th"); break;
					}
					break;
				case 'w':	// Numeric representation of the day of the week ( 0 = Sunday )
					out += String(tm.Wday);
					break;
				case 'F':	// A full textual representation of a month, such as January or March
					out += time.monthString(tm.Month);
					break;
				case 'm':	// Numeric representation of a month, with leading zeros
					out += time.zeropad(tm.Month, 2);
					break;
				case 'M':	// A short textual representation of a month, three letters
					out += time.monthString(tm.Month).substring(0,3);
					break;
				case 'n':	// Numeric representation of a month, without leading zeros
					out += String(tm.Month);
					break;
				case 't':	// Number of days in the given month
					out += String(monthDays[tm.Month - 1]);
					break;
				case 'Y':	// A full numeric representation of a year, 4 digits
					out += String(tm.Year + 1970);
					break;
				case 'y':	// A two digit representation of a year
					out += time.zeropad((tm.Year + 1970) % 100, 2);
					break;
				case 'a':	// am or pm
					out += (tm.Hour < 12) ? F("am") : F("pm");
					break;
				case 'A':	// AM or PM
					out += (tm.Hour < 12) ? F("AM") : F("PM");
					break;
				case 'g':	// 12-hour format of an hour without leading zeros
					out += String(hour12);
					break;
				case 'G':	// 24-hour format of an hour without leading zeros
					out += String(tm.Hour);
					break;
				case 'h':	// 12-hour format of an hour with leading zeros
					out += time.zeropad(hour12, 2);
					break;
				case 'H':	// 24-hour format of an hour with leading zeros
					out += time.zeropad(tm.Hour, 2);
					break;
				case 'i':	// Minutes with leading zeros
					out += time.zeropad(tm.Minute, 2);
					break;
				case 's':	// Seconds with leading zeros
					out += time.zeropad(tm.Second, 2);
					break;
				case 'T':	// abbreviation for timezone
					out += tzname;	
					break;
				case 'v':	// milliseconds as three digits
					out += time.zeropad(time._last_read_ms, 3);				
					break;
 				case 'e':	// Timezone identifier (Olsen)
 					out += getOlsen();
 					break;
				case 'O':	// Difference to Greenwich time (GMT) in hours and minutes written together (+0200)
				case 'P':	// Difference to Greenwich time (GMT) in hours and minutes written with colon (+02:00)
					o = offset;
					out += (o < 0) ? "+" : "-";		// reversed from our offset
					if (o < 0) o = 0 - o;
					out += time.zeropad(o / 60, 2);
					out += (c == 'P') ? ":" : "";
					out += time.zeropad(o % 60, 2);
					break;	
				case 'Z':	//Timezone offset in seconds. West of UTC is negative, east of UTC is positive.
					out += String(0 - offset * 60);
					break;
				case 'z':
					out += String(dayOfYear(t)); // The day of the year (starting from 0)
					break;
				case 'W':
					out += time.zeropad(weekISO(t), 2); // ISO-8601 week number of year, weeks starting on Monday
					break;
				case 'X':
					out += String(yearISO(t)); // ISO-8601 year-week notation year, see https://en.wikipedia.org/wiki/ISO_week_date
					break;
				
				default:
					out += String(c);

			}
		}
	}
	
	return out;
}

uint8_t Timezone::hour(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	return t / 3600 % 24;
}

uint8_t Timezone::minute(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	return t / 60 % 60;
}

uint8_t Timezone::second(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	return t % 60;
}

uint16_t Timezone::ms(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	// Note that here passing anything but TIME_NOW or LAST_READ is pointless
	if (t == TIME_NOW) { time.now(); return time._last_read_ms; }
	if (t == LAST_READ) return time._last_read_ms;
	return 0;
}

uint8_t Timezone::day(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	tmElements_t tm;
	time.breakTime(t, tm);
	return tm.Day;
}

uint8_t Timezone::weekday(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	tmElements_t tm;
	time.breakTime(t, tm);
	return tm.Wday;
}

uint8_t Timezone::month(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	tmElements_t tm;
	time.breakTime(t, tm);
	return tm.Month;
}

uint16_t Timezone::year(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	tmElements_t tm;
	time.breakTime(t, tm);
	return tm.Year + 1970;
}

uint16_t Timezone::dayOfYear(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	time_t jan_1st = time.makeTime(0, 0, 0, 1, 1, year(t));
	return (t - jan_1st) / SECS_PER_DAY;
}

// Now this is where this gets a little obscure. The ISO year can be different from the
// actual (Gregorian) year. That is: you can be in january and still be in week 53 of past
// year, _and_ you can be in december and be in week one of the next. The ISO 8601 
// definition for week 01 is the week with the Gregorian year's first Thursday in it.  
// See https://en.wikipedia.org/wiki/ISO_week_date
//
#define startISOyear(year...) time.makeOrdinalTime(0, 0, 0, FIRST, THURSDAY, JANUARY, year) - 3UL * SECS_PER_DAY;
uint8_t Timezone::weekISO(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	int16_t yr = year(t);
	time_t this_year = startISOyear(yr);
	time_t prev_year = startISOyear(yr - 1);
	time_t next_year = startISOyear(yr + 1);
	if (t < this_year) this_year = prev_year;
	if (t > next_year) this_year = next_year;
	return (t - this_year) / ( SECS_PER_DAY * 7UL) + 1;
}

uint16_t Timezone::yearISO(time_t t /*= TIME_NOW */, ezLocalOrUTC_t local_or_utc /* = LOCAL_TIME */) {
	t = tzTime(t, local_or_utc);
	int16_t yr = year(LAST_READ);
	time_t this_year = startISOyear(yr);
	time_t prev_year = startISOyear(yr - 1);
	time_t next_year = startISOyear(yr + 1);
	if (t < this_year) return yr - 1;
	if (t > next_year) return yr + 1;
	return yr;
}


Timezone UTC;
Timezone& defaultTZ = UTC;


#ifdef ARDUINO_TIMELIB_COMPATIBILITY

	// Can't be macros because then it would also expand the explicit class member calls

	time_t now() { return  (defaultTZ.now()); }
	uint8_t second(time_t t = TIME_NOW) { return (defaultTZ.second(t)); } 
	uint8_t minute(time_t t = TIME_NOW) { return (defaultTZ.minute(t)); }
	uint8_t hour(time_t t = TIME_NOW) { return (defaultTZ.hour(t)); }
	uint8_t day(time_t t = TIME_NOW) { return (defaultTZ.day(t)); } 
	uint8_t weekday(time_t t = TIME_NOW) { return (defaultTZ.weekday(t)); }
	uint8_t month(time_t t = TIME_NOW) { return (defaultTZ.month(t)); } 
	uint16_t year(time_t t = TIME_NOW) { return (defaultTZ.year(t)); } 
	uint8_t hourFormat12(time_t t = TIME_NOW) { return (defaultTZ.hour(t) % 12); }
	bool isAM(time_t t = TIME_NOW) { return (defaultTZ.hour(t) < 12) ? true : false; }
	bool isPM(time_t t = TIME_NOW) { return (defaultTZ.hour(t) >= 12) ? true : false; } 
	String monthStr(const uint8_t month) { return time.monthString(month); }
	String monthShortStr(const uint8_t month) { return time.monthString(month).substring(0,3); }
	String dayStr(const uint8_t day) { return time.dayString(day); }
	String dayShortStr(const uint8_t day) { return time.dayString(day).substring(0,3); }
	void setTime(time_t t) { defaultTZ.setTime(t); }
	void setTime(const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t month, const uint16_t yr) { defaultTZ.setTime(hr, min, sec, day, month, yr); }
	void breakTime(time_t t, tmElements_t &tm) { time.breakTime(t, tm); }
	time_t makeTime(tmElements_t &tm) { return time.makeTime(tm); }
	time_t makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, int16_t year) { return time.makeTime(hour, minute, second, day, month, year); }
	timeStatus_t timeStatus() { return time.timeStatus(); }

#endif