

#include <Arduino.h>

#include <ezTime.h>

#ifdef EZTIME_NETWORK_ENABLE

// Caching of namezone data is only available on EZTIME_NVS_ENABLE
#ifdef EZTIME_NVS_ENABLE
#include <Preferences.h>		// For timezone lookup cache
#endif // EZTIME_NVS_ENABLE

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#else
#include <WiFi.h>
#endif

#endif // EZTIME_NETWORK_ENABLE


const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0


EZtime::EZtime() {
	ezError_t _last_error = NO_ERROR;
	ezDebugLevel_t _debug_level = NONE;
	_time_status = timeNotSet;
#ifdef EZTIME_NETWORK_ENABLE
	_ntp_enabled = true;
	_ntp_server = NTP_SERVER;
	_ntp_local_port = NTP_LOCAL_PORT;
	_update_due = 0;
	_update_interval = NTP_INTERVAL;
	_update_backoff = 0;
#endif // ifdef EZTIME_NETWORK_ENABLE
}


////////// Error handing

String EZtime::errorString(ezError_t err) {
	switch (err) {
		case NO_ERROR: return			"OK";
		case LAST_ERROR: return 		errorString(_last_error);
		case NO_NETWORK: return			"No network";
		case TIMEOUT: return 			"Timeout";
		case CONNECT_FAILED: return 	"Connect Failed";
		case DATA_NOT_FOUND: return		"Data not found";
		case LOCKED_TO_UTC: return		"Locked to UTC";
		default: return					"Unkown error";
	}
}

String EZtime::debugLevelString(ezDebugLevel_t level) {
	switch (level) {
		case NONE: return "NONE";
		case ERROR: return "ERROR";
		case INFO: return "INFO";
		case DEBUG: return "DEBUG";
	}
}

ezError_t EZtime::error() { 
	ezError_t tmp = _last_error;
	_last_error = NO_ERROR;
	return tmp;
}

void EZtime::error(ezError_t err) {
	_last_error = err;
	if (_last_error) debugln(ERROR, "ERROR: " + errorString(err));
}

void EZtime::debug(ezDebugLevel_t level) { 
	_debug_level = level;
	debugln(INFO, "\r\nezTime debug level set to " + debugLevelString(level));
}

void EZtime::debug(ezDebugLevel_t level, String str) { 
	if (_debug_level >= level) {
		Serial.print(str); 
	}
}

void EZtime::debugln(ezDebugLevel_t level, String str) {
	if (_debug_level >= level) {
		Serial.println(str); 
	}
}

////////////////////////

String EZtime::monthString(uint8_t month) {
	switch(month) {
		case 1: return "January";
		case 2: return "February";		
		case 3: return "March";
		case 4: return "April";
		case 5: return "May";
		case 6: return "June";
		case 7: return "July";
		case 8: return "August";
		case 9: return "September";
		case 10: return "October";
		case 11: return "November";
		case 12: return "December";
	}
	return "";
}

String EZtime::dayString(uint8_t day) {
	switch(day) {
		case 1: return "Sunday";
		case 2: return "Monday";
		case 3: return "Tuesday";		
		case 4: return "Wednesday";
		case 5: return "Thursday";
		case 6: return "Friday";
		case 7: return "Saturday";
	}
	return "";
}


timeStatus_t EZtime::timeStatus() { return _time_status; }

time_t EZtime::now() {
	unsigned long m  = millis();
	time_t t;
#ifdef EZTIME_NETWORK_ENABLE	
	if (_update_interval && (m >= _update_due + (_update_backoff * 1000)) ) {
		if (m - _update_due > NTP_STALE_AFTER * 1000) _time_status = timeNeedsSync;	// If unable to sync for an hour, timeStatus = timeNeedsSync
		unsigned long measured_at;
		if (queryNTP(_ntp_server, t, measured_at)) {
			time_t old_time = _last_sync_time + ((m - _last_sync_millis) / 1000);	//
			uint16_t old_ms = (m - _last_sync_millis) % 1000;						//
//			time_t old_time = _last_sync_time + ( millisElapsed(m) / 1000 );
//			uint16_t old_ms = millisElapsed(m) % 1000;
			_last_sync_time = t;
			_last_sync_millis = measured_at;
			uint16_t new_ms = (m - measured_at) % 1000;
			int32_t correction = (t - old_time) * 1000 + new_ms - old_ms;
			_last_read_ms = new_ms;
			_update_due = m + _update_interval * 1000;
			_update_backoff = 0;
			debug(INFO, "Received time: " + UTC.dateTime(_last_sync_time, "l, d-M-y H:i:s.v T"));
			if (_time_status != timeNotSet) {
				debugln(INFO, " (internal clock was " + ( correction == 0 ? "spot on)" : String(abs(correction)) + " ms " + ( correction > 0 ? "slow)" : "fast)" ) ) );
//				_fudge = 0 - correction;		// If we are corrected forward we have to fudge backward again to where we were and then slowly diminish fudgeing in millisElapsed
			} else {
				debugln(INFO, "");
//				_fudge = 0;
			}
			_time_status = timeSet;
		} else {
			_update_backoff += NTP_RETRY;
		}
	}
#endif // EZTIME_NETWORK_ENABLE
	t = _last_sync_time + ((m - _last_sync_millis) / 1000); 		//
	_last_read_ms = (m - _last_sync_millis) % 1000;					//
//	int32_t elapsed = millisElapsed(m);
//	t = _last_sync_time + (elapsed / 1000); 
//	_last_read_ms = elapsed % 1000;
	if (m < ezTime._last_sync_millis) t += 0xFFFFFFFF / 1000;				// millis() rolled over, we're assuming just once :)
	return t;
}

int32_t EZtime::millisElapsed(unsigned long m) {							// can return negative value just after NTP update if fudging
	unsigned long elapsed = m - _last_sync_millis;
	int32_t fudge_remaining = abs(_fudge) - (elapsed / 100);				// one ms every 100 ms, i.e. 1% clock speed change 
	if (fudge_remaining < 0) return elapsed;
	if (_fudge < 0) fudge_remaining = 0 - fudge_remaining;
	elapsed += fudge_remaining;
	return elapsed;
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

time_t EZtime::makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, int16_t year) {
	tmElements_t tm;
	tm.Hour = hour;
	tm.Minute = minute;
	tm.Second = second;
	tm.Day = day;
	tm.Month = month;
	tm.Year = year - 1970;
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

// makeUmpteenthTime allows you to resolve "second thursday in September in 2018" into a number of seconds since 1970
// (Very useful for the timezone calculations that ezTime does internally) 
// If umpteenth is 0 or 5 it is taken to mean "the last $wday in $month"
time_t EZtime::makeUmpteenthTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t umpteenth, uint8_t wday, uint8_t month, int16_t year) {
	if (umpteenth == 5) umpteenth = 0;
	uint8_t m = month;   
	uint8_t w = umpteenth;
	if (w == 0) {			// is this a "Last week" rule?
		if (++m > 12) {		// yes, for "Last", go to the next month
			m = 1;
			++year;
		}
		w = 1;               // and treat as first week of next month, subtract 7 days later
	}
	time_t t = makeTime(hour, minute, second, 1, m, year);
	// add offset from the first of the month to weekday, and offset for the given week
	t += ( (wday - weekday(t) + 7) % 7 + (w - 1) * 7 ) * SECS_PER_DAY;
	// back up a week if this is a "Last" rule
	if (umpteenth == 0) t -= 7 * SECS_PER_DAY;
	return t;
}

tzData_t EZtime::parsePosix(String posix, int16_t year) {

	tzData_t r;

	r.dst_start_in_local = 0;
	r.dst_end_in_local = 0;
	r.dst_start_in_utc = 0;
	r.dst_end_in_utc = 0;

	int8_t offset_hr = 0;
	uint8_t offset_min = 0;
	int8_t dst_shift_hr = 1;
	uint8_t dst_shift_min = 0;
	
	uint8_t start_month = 0, start_week = 0, start_dow = 0, start_time_hr = 2, start_time_min = 0;
	uint8_t end_month = 0, end_week = 0, end_dow = 0, end_time_hr = 2, end_time_min = 0;
	
	enum posix_state_e {STD_NAME, OFFSET_HR, OFFSET_MIN, DST_NAME, DST_SHIFT_HR, DST_SHIFT_MIN, START_MONTH, START_WEEK, START_DOW, START_TIME_HR, START_TIME_MIN, END_MONTH, END_WEEK, END_DOW, END_TIME_HR, END_TIME_MIN};
	posix_state_e state = STD_NAME; 

	uint8_t offset = 0;
	String cur_str = "";
	bool ignore_nums = false;
	
	for (uint8_t offset = 0; offset < posix.length(); offset++) {
		String newchar = posix.substring(offset, offset + 1);
	
		// Do not replace the code below with switch statement: evaluation of state that 
		// changes while this runs. (Only works because this state can only go forward.)
	
		if (state == STD_NAME) {
			if (newchar == "<") ignore_nums = true;
			if (newchar == ">") ignore_nums = false;
			if (!ignore_nums && (isDigit((int)newchar.c_str()[0]) || newchar == "-"  || newchar == "+")) {
				state = OFFSET_HR;
				cur_str = "";
			} else {
				cur_str += newchar;
				r.std_tzname = cur_str;
			}
		}
		if (state == OFFSET_HR) {
			if (newchar == "+") {
				newchar = "";
			} else if (newchar == ":") {
				state = OFFSET_MIN;
				cur_str = "";
				newchar = ""; 					// ignore the ":"
			} else if (newchar != "-" && !isDigit((int)newchar.c_str()[0])) {
				state = DST_NAME;
				cur_str = "";
			} else {
				cur_str += newchar;
				offset_hr = cur_str.toInt();
			}
		}			
		if (state == OFFSET_MIN) {
			if (newchar != "" && !isDigit((int)newchar.c_str()[0])) {
				state = DST_NAME;
				ignore_nums = false;
				cur_str = "";
			} else {
				cur_str += newchar;
				offset_min = cur_str.toInt();
			}
		}				
		if (state == DST_NAME) {
			if (newchar == "<") ignore_nums = true;
			if (newchar == ">") ignore_nums = false;
			if (newchar == ",") {
				state = START_MONTH;
				cur_str = "";
				newchar = "";					// ignore the ","
			} else if (!ignore_nums && (newchar == "-" || isDigit((int)newchar.c_str()[0]))) {
				state = DST_SHIFT_HR;
				cur_str = "";
			} else {
				cur_str += newchar;
				r.dst_tzname = cur_str;
			}
		}		
		if (state == DST_SHIFT_HR) {
			if (newchar == ":") {
				state = DST_SHIFT_MIN;
				cur_str = "";
				newchar = ""; 					// ignore the ":"
			} else if (newchar == ",") {
				state = START_MONTH;
				cur_str = "";
				newchar="";
			} else {
				cur_str += newchar;
				dst_shift_hr = cur_str.toInt();
			}
		}			
		if (state == DST_SHIFT_MIN) {
			if (newchar == ",") {
				state = START_MONTH;
				cur_str = "";
				newchar="";
			} else {
				cur_str += newchar;
				dst_shift_min = cur_str.toInt();
			}
		}			
		if (state == START_MONTH) {
			if (newchar == ".") {
				state = START_WEEK;
				cur_str = "";
				newchar = "";
			} else if (newchar != "M") {
				cur_str += newchar;
				start_month = cur_str.toInt();				
			}
		}			
		if (state == START_WEEK) {
			if (newchar == ".") {
				state = START_DOW;
				cur_str = "";
				newchar = "";
			} else {
				cur_str += newchar;
				start_week = cur_str.toInt();				
			}
		}		
		if (state == START_DOW) {
			if (newchar == "/") {
				state = START_TIME_HR;
				cur_str = "";
				newchar = "";
			} else if (newchar == ",") {
				state = END_MONTH;
				cur_str = "";
				newchar = "";				
			} else {
				cur_str += newchar;
				start_dow = cur_str.toInt();				
			}
		}
		if (state == START_TIME_HR) {
			if (newchar == ":") {
				state = START_TIME_MIN;
				cur_str = "";
				newchar = ""; 					// ignore the ":"
			} else if (newchar == ",") {
				state = END_MONTH;
				cur_str = "";
				newchar = ""; 					// ignore the ":"
			} else {
				cur_str += newchar;
				start_time_hr = cur_str.toInt();
			}
		}		
		if (state == START_TIME_MIN) {
			if (newchar == ",") {
				state = END_MONTH;
				cur_str = "";
				newchar = "";
			} else {
				cur_str += newchar;
				start_time_min = cur_str.toInt();
			}
		}		
		if (state == END_MONTH) {
			if (newchar == ".") {
				state = END_WEEK;
				cur_str = "";
				newchar = "";
			} else if (newchar != "M") {
				cur_str += newchar;
				end_month = cur_str.toInt();				
			}
		}			
		if (state == END_WEEK) {
			if (newchar == ".") {
				state = END_DOW;
				cur_str = "";
				newchar = "";
			} else {
				cur_str += newchar;
				end_week = cur_str.toInt();				
			}
		}		
		if (state == END_DOW) {
			if (newchar == "/") {
				state = END_TIME_HR;
				cur_str = "";
				newchar = "";			
			} else {
				cur_str += newchar;
				end_dow = cur_str.toInt();				
			}
		}
		if (state == END_TIME_HR) {
			if (newchar == ":") {
				state = END_TIME_MIN;
				cur_str = "";
				newchar = ""; 					// ignore the ":"
			}  else {
				cur_str += newchar;
				end_time_hr = cur_str.toInt();
			}
		}		
		if (state == END_TIME_MIN) {
			cur_str += newchar;
			end_time_min = cur_str.toInt();
		}
	}	
	r.std_offset = (offset_hr < 0) ? offset_hr * 3600 - offset_min * 60 : offset_hr * 3600 + offset_min * 60;
	r.dst_offset = r.std_offset - dst_shift_hr * 3600 - dst_shift_min * 60;
	if (start_month) {
		r.dst_start_in_local = ezTime.makeUmpteenthTime(start_time_hr, start_time_min, 0, start_week, start_dow, start_month, year);
		r.dst_end_in_local = ezTime.makeUmpteenthTime(end_time_hr, end_time_min, 0, end_week, end_dow, end_month, year);
		r.dst_start_in_utc = r.dst_start_in_local - r.std_offset;
		r.dst_end_in_utc = r.dst_end_in_local - r.dst_offset;
	}
	return r;
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
	String out = String(number);
	while (out.length() < length) out = "0" + out;
	return out;
}

String EZtime::getBetween(String &haystack, String before_needle, String after_needle /* = "" */) {
	int16_t start = haystack.indexOf(before_needle);
	if (start == -1) return "";
	start += before_needle.length();
	if (after_needle == "") return haystack.substring(start);
	int16_t end =  haystack.indexOf(after_needle, start);
	if (end == -1) return "";
	return haystack.substring(start, end);
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


#ifdef EZTIME_NETWORK_ENABLE

// This is a nice self-contained NTP routine if you need one: feel free to use it.
// It gives you the seconds since 1970 (unix epoch) and the millis() on your system when 
// that happened (by deducting fractional seconds and estimated network latency).
bool EZtime::queryNTP(String server, time_t &t, unsigned long &measured_at) {
	debug(INFO, "Querying " + server + " ... ");

	if (!WiFi.isConnected()) { error(NO_NETWORK); return false; }

	WiFiUDP udp;
	udp.begin(_ntp_local_port);
	
	// Send NTP packet
	byte buffer[NTP_PACKET_SIZE];
	memset(buffer, 0, NTP_PACKET_SIZE);
	buffer[0] = 0b11100011;		// LI, Version, Mode
	buffer[1] = 0;   			// Stratum, or type of clock
	buffer[2] = 6;				// Polling Interval
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
	while (!udp.parsePacket()) {
		delay (1);
		if (millis() - started > NTP_TIMEOUT) { error(TIMEOUT); return false; }
	}
	
	// Set the t and measured_at variables that were passed by reference
	unsigned long done = millis();
	debugln(INFO, "success (round trip " + String(done - started) + " ms)");
	udp.read(buffer, NTP_PACKET_SIZE);
	unsigned long secsSince1900 = buffer[40] << 24 | buffer[41] << 16 | buffer[42] << 8 | buffer[43];
	t = secsSince1900 - 2208988800UL;					// Subtract 70 years to get seconds since 1970
	unsigned long fraction = buffer[44] << 24 | buffer[45] << 16 | buffer[46] << 8 | buffer[47];
	uint16_t ms = fraction / 4294967UL;					// Turn 32 bit fraction into ms by dividing by 2^32 / 1000 
	measured_at = done - ((done - started) / 2) - ms;	// Assume symmetric network latency and return when we think the whole second was.
	return true;
}

void EZtime::setInterval(uint16_t seconds /* = 0 */) { _update_interval = seconds; }

void EZtime::setServer(String ntp_server /* = NTP_SERVER */) { _ntp_server = ntp_server; }

void EZtime::updateNow() { _update_due = millis(); now(); }

bool EZtime::waitForSync(uint16_t timeout /* = 0 */) {

	unsigned long start = millis();

	if (!WiFi.isConnected()) {
		debug(INFO, "Waiting for WiFi ... ");
		while (!WiFi.isConnected()) {
			if ( timeout && (millis() - start) / 1000 > timeout ) { error(TIMEOUT); return false;};
			delay(25);
		}
		debugln(INFO, "connected");
	}

	if (!_time_status != timeSet) {
		debugln(INFO, "Waiting for time sync");
		while (_time_status != timeSet) {
			if ( timeout && (millis() - start) / 1000 > timeout ) { error(TIMEOUT); return false;};
			delay(250);
			now();
		}
		debugln(INFO, "Time is in sync");
	}		
	
}

#ifdef EZTIME_NVS_ENABLE
void EZtime::clearCache() {
	Preferences preferences;
	preferences.begin("ezTime", false);			// read-write
	preferences.clear();
	preferences.end();
}
#endif // EZTIME_NVS_ENABLE

bool EZtime::timezoneAPI(String location, String &olsen, String &posix) {

	if (!WiFi.isConnected()) { error(NO_NETWORK); return false; }

	String path;
	if (location.indexOf("/") != -1) { 
		path = "/api/timezone/?" + ezTime.urlEncode(location);
	} else if (location != "") {
		path = "/api/address/?" + ezTime.urlEncode(location);
	} else {
		path = "/api/ip";
	}

	WiFiClient client;
	if (!client.connect("timezoneapi.io", 80)) { error(CONNECT_FAILED);	return false; }

	client.println("GET " + path + " HTTP/1.1");
	client.println("Host: timezoneapi.io");
	client.println("Connection: close");
	client.println();
	client.setTimeout(3000);
	String reply = client.readString();
	debugln(DEBUG, "Sent request for http://timezoneapi.io" + path);
	debugln(DEBUG, "Reply from server in full:\r\n\r\n" + reply + "\r\n\r\n");
	
	// The below should not be mistaken for Json parsing...
	posix = getBetween(reply, "\"tz_string\":\"", "\"");
	posix.replace("\\/", "/");
	olsen = getBetween(reply, "\"id\":\"", "\"");
	olsen.replace("\\/", "/");
	if (olsen != "" && posix != "") {
		return true;
	} else {
		error(DATA_NOT_FOUND); 
		return false;
	}
}

#endif // EZTIME_NETWORK_ENABLE


EZtime ezTime;






//
// Timezone class
//

Timezone::Timezone(bool locked_to_UTC /* = false */) {
	_locked_to_UTC = locked_to_UTC;
	_tzdata.std_tzname = "UTC";
	_tzdata.std_offset = 0;
}

bool Timezone::setPosix(String posix) {
	if (_locked_to_UTC) { ezTime.error(LOCKED_TO_UTC); return false; }
	_tzdata = ezTime.parsePosix(posix, UTC.year());
	_olsen = "";			// Might be manually set, so delete _olsen as to not suggest a link
}	

String Timezone::getPosix() { return _posix;}

#ifdef EZTIME_NETWORK_ENABLE

bool Timezone::setLocation(String location, bool force_lookup /* = false */) {
	ezTime.debugln(INFO, "Timezone lookup for: " + location);
	if (_locked_to_UTC) { ezTime.error(LOCKED_TO_UTC); return false; }
#ifdef EZTIME_NVS_ENABLE
	// Caching only on EZTIME_NVS_ENABLE

	Preferences preferences;

	location.replace(":", "_");		// ":" is our record separator, cannot be in location

	String cache, cache_location; 
	String hit_olsen = "";
	String hit_posix = "";
	int16_t x, hit_year; 
	uint8_t first_free_entry = 0;
	uint8_t cache_number = 0;
	preferences.begin("ezTime", true);		// read-only
	for (uint8_t n = 1; n < 100; n++) {
		char key[] = "lookupcache-xx";
		key[12] = '0' + (n / 10);
		key[13] = '0' + (n % 10);
		cache = preferences.getString(key);
		if (cache == "") {
			if(!first_free_entry) first_free_entry = n;
		} else {
			x = cache.indexOf(":"); cache_location = cache.substring(0, x); cache = cache.substring(x + 1);
			if (cache_location == location) {
				cache_number = n;		
				x = cache.indexOf(":"); hit_olsen = cache.substring(0, x); cache = cache.substring(x + 1);
				x = cache.indexOf(":"); hit_year = cache.substring(0, x).toInt();
				hit_posix = cache.substring(x + 1);
				break;
			}
		}
	}
	preferences.end();
	if (!cache_number) {
		if (first_free_entry) {
			cache_number = first_free_entry;
		} else {
			// Instead of writing whole logic for expiring cache, just pick a random location.
			// If there's only a few used entries, odds of it overwriting something that's still
			// used are low-ish, and re-fetching is not that expensive.
			ezTime.debugln(INFO, "Cache full, next write will be at random location.");
			cache_number = ( esp_random() % 100 ) + 1;
		}
	}

	// Return cache hit
	if ( !force_lookup && hit_posix != "" && ( hit_year == year())) {
		ezTime.debugln(INFO, "Cache hit: " + hit_olsen + " (" + hit_posix + ") from " + String(hit_year));
		_posix = hit_posix;
		setPosix(hit_posix);
		_olsen = hit_olsen;		// Has to happen after setPosix because that sets _olsen to "";
		return true;
	}
#endif // ESP32
	ezTime.error();		// Resets last error to OK
	ezTime.debug(INFO, "timezoneapi.io lookup ... ");
	String olsen, posix;
	if (ezTime.timezoneAPI(location, olsen, posix)) {
		ezTime.debugln(INFO, "success");
		ezTime.debugln(INFO, "    Olsen: " + olsen);
		ezTime.debugln(INFO, "    Posix: " + posix);
		_posix = posix;
		setPosix(posix);
		_olsen = olsen;		// Has to happen after setPosix because that sets _olsen to "";
		
#ifdef EZTIME_NVS_ENABLE
		ezTime.debugln(INFO, "Storing to cache(" + String(cache_number) + "): " + location + ":" + olsen + ":" + String(year()) + ":" + posix);
		char key[] = "lookupcache-xx";
		key[12] = '0' + (cache_number / 10);
		key[13] = '0' + (cache_number % 10);
		preferences.begin("ezTime", false);			// read-write
		preferences.putString(key, location + ":" + olsen + ":" + String(year()) + ":" + posix);
		preferences.end();
		return true;
	} else {		
		if (!force_lookup && hit_posix != "") {
			ezTime.debugln(INFO, "Using cache hit: " + hit_olsen + " (" + hit_posix + ") from " + String(hit_year));
			_posix = hit_posix;
			setPosix(hit_posix);
			_olsen = hit_olsen;		// Has to happen after setPosix because that sets _olsen to "";
			return true;
		}	
#endif // EZTIME_NVS_ENABLE
	}
	ezTime.error(ezTime.error());		// This throws the last error (as generated by timezoneAPI) again.
	return false;

}

String Timezone::getOlsen() { return _olsen; }

#endif // EZTIME_NETWORK_ENABLE


void Timezone::setDefault() {
	defaultTZ = this;
}

bool Timezone::isDST() {
	time_t t = ezTime.now();
    if (_tzdata.dst_start_in_utc == _tzdata.dst_end_in_utc) return false;			// No DST observed here
    
    if (_tzdata.dst_end_in_utc > _tzdata.dst_start_in_utc) {
        return (t >= _tzdata.dst_start_in_utc && t < _tzdata.dst_end_in_utc);		// northern hemisphere
    } else {
        return !(t >= _tzdata.dst_end_in_utc && t < _tzdata.dst_start_in_utc);		// southern hemisphere
    }
}

bool Timezone::isDST_UTC(time_t t /*= TIME_NOW */) {
	if (t == TIME_NOW) t = ezTime.now();
	
	tzData_t tz;
	if (year(t) != year()) {
		tz = ezTime.parsePosix(_posix, year(t));
	} else {
		tz = _tzdata;
	}

    if (tz.dst_start_in_utc == tz.dst_end_in_utc) return false;			// No DST observed here
    
    if (tz.dst_end_in_utc > tz.dst_start_in_utc) {
        return (t >= tz.dst_start_in_utc && t < tz.dst_end_in_utc);		// northern hemisphere
    } else {
        return !(t >= tz.dst_end_in_utc && t < tz.dst_start_in_utc);	// southern hemisphere
    }
}

bool Timezone::isDST_local(time_t t /*= TIME_NOW */) {
	if (t == TIME_NOW) return isDST_UTC(TIME_NOW);						//Prevent loops where timezone's now() tries to find offset
	
	t = _readTime(t);
	tzData_t tz;
	if (year(t) != year()) {
		tz = ezTime.parsePosix(_posix, year(t));
	} else {
		tz = _tzdata;
	}

    if (tz.dst_start_in_local == tz.dst_end_in_local) return false;			// No DST observed here
    
    if (tz.dst_end_in_utc > tz.dst_start_in_utc) {
        return (t >= tz.dst_start_in_local && t < tz.dst_end_in_local);		// northern hemisphere
    } else {
        return !(t >= tz.dst_end_in_local && t < tz.dst_start_in_local);	// southern hemisphere
    }
}


String Timezone::getTimezoneName(time_t t /*= TIME_NOW */) {
	if (isDST_local(t)) {
		return _tzdata.dst_tzname;
	} else {
		return _tzdata.std_tzname;
	}
}

int32_t Timezone::getOffset(time_t t /*= TIME_NOW */) {
	
	if (isDST_local(t)) {
		return _tzdata.dst_offset;
	} else {
		return _tzdata.std_offset;
	}
}

time_t Timezone::now(bool update_last_read /* = true */) {

	time_t t;	
	t = ezTime.now();
    if (_tzdata.dst_start_in_utc == _tzdata.dst_end_in_utc) {
    	t -= _tzdata.std_offset;
    } else {
    	if (_tzdata.dst_end_in_utc > _tzdata.dst_start_in_utc) {
        	t -= (t >= _tzdata.dst_start_in_utc && t < _tzdata.dst_end_in_utc) ? _tzdata.dst_offset : _tzdata.std_offset;
    	} else {
        	t -= (t >= _tzdata.dst_end_in_utc && t < _tzdata.dst_start_in_utc) ? _tzdata.dst_offset : _tzdata.std_offset;
        }
    }
	if (update_last_read) _last_read_t = t;	
	return t;
}

time_t Timezone::_readTime(time_t t) {
	switch (t) {
		case TIME_NOW: return now();
		case LAST_READ: return _last_read_t;
		default: return (t);
	}
}	

void Timezone::setTime(time_t t) {
	t += getOffset(t);
	ezTime._last_sync_time = t;
	ezTime._last_sync_millis = millis();
	ezTime._time_status = timeSet;
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
	setTime(ezTime.makeTime(tm));
}

String Timezone::dateTime(String format /* = DEFAULT_TIMEFORMAT */) {
	return dateTime(TIME_NOW, format);
}

String Timezone::dateTime(time_t t, String format /* = DEFAULT_TIMEFORMAT */) {
	t = _readTime(t);
	
	String tmpstr;
	uint8_t tmpint8;
	String out = "";

	tmElements_t tm;
	ezTime.breakTime(t, tm);

	int8_t hour12 = tm.Hour % 12;
	if (hour12 == 0) hour12 = 12;
	
	int32_t o;

	bool escape_char = false;
	
	for (int8_t n = 0; n < format.length(); n++) {
	
		char c = (char) format.substring(n, n + 1).c_str()[0];
		
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
					out += ezTime.zeropad(tm.Day, 2);
					break;
				case 'D':	// A textual representation of a day, three letters
					tmpstr = ezTime.dayString(tm.Wday);
					out += tmpstr.substring(0,3);
					break;
				case 'j':	// Day of the month without leading zeros
					out += String(tm.Day);
					break;
				case 'l':	// (lowercase L) A full textual representation of the day of the week
					out += ezTime.dayString(tm.Wday);
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
							out += "st"; break;
						case 2:
						case 22:
							out += "nd"; break;
						case 3:
						case 23:
							out += "rd"; break;
						default:
							out += "th"; break;
					}
					break;
				case 'w':	// Numeric representation of the day of the week ( 0 = Sunday )
					out += String(tm.Wday);
					break;
				case 'F':	// A full textual representation of a month, such as January or March
					out += ezTime.monthString(tm.Month);
					break;
				case 'm':	// Numeric representation of a month, with leading zeros
					out += ezTime.zeropad(tm.Month, 2);
					break;
				case 'M':	// A short textual representation of a month, three letters
					tmpstr = ezTime.monthString(tm.Month);
					out += tmpstr.substring(0,3);
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
					out += ezTime.zeropad((tm.Year + 1970) % 100, 2);
					break;
				case 'a':	// am or pm
					out += (tm.Hour < 12) ? "am" : "pm";
					break;
				case 'A':	// AM or PM
					out += (tm.Hour < 12) ? "AM" : "PM";
					break;
				case 'g':	// 12-hour format of an hour without leading zeros
					out += String(hour12);
					break;
				case 'G':	// 24-hour format of an hour without leading zeros
					out += String(tm.Hour);
					break;
				case 'h':	// 12-hour format of an hour with leading zeros
					out += ezTime.zeropad(hour12, 2);
					break;
				case 'H':	// 24-hour format of an hour with leading zeros
					out += ezTime.zeropad(tm.Hour, 2);
					break;
				case 'i':	// Minutes with leading zeros
					out += ezTime.zeropad(tm.Minute, 2);
					break;
				case 's':	// Seconds with leading zeros
					out += ezTime.zeropad(tm.Second, 2);
					break;
				case 'T':	// abbreviation for timezone
					out += getTimezoneName(LAST_READ);	
					break;
				case 'v':	// milliseconds as three digits
					out += ezTime.zeropad(ezTime._last_read_ms, 3);				
					break;
				case 'e':	// Timezone identifier (Olsen or if not available current TZ abbreviation)
					if (_olsen != "") {
						out += _olsen;
					} else {
						out += getTimezoneName(LAST_READ);
					}
					break;
				case 'O':	// Difference to Greenwich time (GMT) in hours and minutes written together (+0200)
				case 'P':	// Difference to Greenwich time (GMT) in hours and minutes written with colon (+02:00)
					o = getOffset(LAST_READ);
					out += (o >= 0) ? "+" : "-";
					if (o < 0) o = 0 - o;
					out += ezTime.zeropad(o / 3600, 2);
					out += (c == 'P') ? ":" : "";
					out += ezTime.zeropad(o / 60, 2);
					break;	
				case 'Z':	//Timezone offset in seconds. West of UTC is negative, east of UTC is positive.
					out+= String(0 - getOffset(LAST_READ));
					break;
				default:
					out += String(c);

	
				// z -> The day of the year (starting from 0)
		
				// W -> ISO-8601 week number of year, weeks starting on Monday

			}
		}
	}
	
	return out;
}

uint8_t Timezone::hour(time_t t /* = TIME_NOW */) {
	t = _readTime(t);
	return t / 3600 % 24;
}

uint8_t Timezone::minute(time_t t /*= TIME_NOW */) {
	t = _readTime(t);
	return t / 60 % 60;
}

uint8_t Timezone::second(time_t t /* = TIME_NOW */) {
	t = _readTime(t);
	return t % 60;
}

uint16_t Timezone::ms(time_t t /* = TIME_NOW */) {
	// Note that here passing anything but TIME_NOW or LAST_READ is pointless
	t = _readTime(t);
	return ezTime._last_read_ms;
}

uint8_t Timezone::day(time_t t /* = TIME_NOW */) {
	tmElements_t tm;
	ezTime.breakTime(t, tm);
	return tm.Day;
}

uint8_t Timezone::weekday(time_t t /* = TIME_NOW */) {
	t = _readTime(t);
	tmElements_t tm;
	ezTime.breakTime(t, tm);
	return tm.Wday;
}

uint8_t Timezone::month(time_t t /* = TIME_NOW */) {
	t = _readTime(t);
	tmElements_t tm;
	ezTime.breakTime(t, tm);
	return tm.Month;
}

uint16_t Timezone::year(time_t t /* = TIME_NOW */) {
	t = _readTime(t);
	tmElements_t tm;
	ezTime.breakTime(t, tm);
	return tm.Year + 1970;
}

bool Timezone::secondChanged() {
	time_t t = now(false);
	if (_last_read_t != t) return true;
	return false;
}

bool Timezone::minuteChanged() {
	time_t t = now(false);
	if (_last_read_t / 60 != t / 60) return true;
	return false;
}

Timezone UTC;
Timezone& defaultTZ = UTC;


#ifdef ARDUINO_TIMELIB_COMPATIBILITY

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

String monthStr(const uint8_t month) { return ezTime.monthString(month); }
String monthShortStr(const uint8_t month) { String tmp = ezTime.monthString(month); return tmp.substring(0,3); }
String dayStr(const uint8_t day) { return ezTime.dayString(day); }
String dayShortStr(const uint8_t day) { String tmp = ezTime.dayString(day); return tmp.substring(0,3); }

void setTime(time_t t) { defaultTZ.setTime(t); }
void setTime(const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t month, const uint16_t yr) { defaultTZ.setTime(hr, min, sec, day, month, yr); }
void breakTime(time_t t, tmElements_t &tm) { ezTime.breakTime(t, tm); }

time_t makeTime(tmElements_t &tm) { return ezTime.makeTime(tm); }
time_t makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, int16_t year) { return ezTime.makeTime(hour, minute, second, day, month, year); }

timeStatus_t timeStatus() { ezTime.timeStatus(); }

#endif //ARDUINO_TIMELIB_COMPATIBILITY