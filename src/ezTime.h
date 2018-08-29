#ifndef _EZTIME_H_
#define _EZTIME_H_

#define ARDUINO_TIMELIB_COMPATIBILITY

// Compiles in NTP updating and timezoneapi.io fetching 
#define EZTIME_NETWORK_ENABLE

// On espressif turn on NVS storage (with "Preferences")
#if defined (ESP32)		// This syntax so we can add " || defined (SOMEOTHER"
#define EZTIME_NVS_ENABLE
#endif  



#if !defined(__time_t_defined) // avoid conflict with newlib or other posix libc
typedef unsigned long time_t;
#endif

#include <inttypes.h>
#ifndef __AVR__
#include <sys/types.h> // for __time_t_defined, but avr libc lacks sys/types.h
#endif

#if !defined(__time_t_defined) // avoid conflict with newlib or other posix libc
typedef unsigned long time_t;
#endif


////////// Error handing

typedef enum {
	NO_ERROR,
	LAST_ERROR,			// Pseudo error: replaced by last error
	NO_NETWORK, 
	TIMEOUT,
	CONNECT_FAILED,
	DATA_NOT_FOUND,
	LOCKED_TO_UTC
} ezError_t;

typedef enum {
	NONE, 
	ERROR,
	INFO,
	DEBUG
} ezDebugLevel_t;

////////////////////////


#define LEAP_YEAR(Y)	( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
#define SECS_PER_DAY	(86400UL)

typedef struct  { 
	uint8_t Second; 
	uint8_t Minute; 
	uint8_t Hour; 
	uint8_t Wday;   // day of week, sunday is day 1
	uint8_t Day;
	uint8_t Month; 
	uint8_t Year;   // offset from 1970; 
} tmElements_t;

typedef enum { 
	timeNotSet,
	timeNeedsSync,
	timeSet
} timeStatus_t;

typedef struct  { 
	String std_tzname;
	String dst_tzname;
	int32_t std_offset;
	int32_t dst_offset;
	time_t dst_start_in_local;
	time_t dst_end_in_local;
	time_t dst_start_in_utc;
	time_t dst_end_in_utc;
} tzData_t;

#define TIME_NOW			0xFFFFFFFF
#define LAST_READ			0xFFFFFFFE

#define NTP_PACKET_SIZE		48
#define NTP_LOCAL_PORT		2342
#define NTP_SERVER			"pool.ntp.org"
#define NTP_TIMEOUT			1500			// milliseconds
#define NTP_INTERVAL		600				// default update interval in seconds
#define NTP_RETRY			3				// Retry after this many seconds on failed NTP
#define NTP_STALE_AFTER		3600			// If update due for this many seconds, set timeStatus to timeNeedsSync

// Various date-time formats
#define ATOM 				"Y-m-d\\TH:i:sP"
#define COOKIE				"l, d-M-Y H:i:s T"
#define ISO8601				"Y-m-d\\TH:i:sO"
#define RFC822				"D, d M y H:i:s O"
#define RFC850				"l, d-M-y H:i:s T"
#define RFC1036				"D, d M y H:i:s O"
#define RFC1123				"D, d M Y H:i:s O"
#define RFC2822				"D, d M Y H:i:s O"
#define RFC3339 			"Y-m-d\\TH:i:sP"
#define RFC3339_EXT			"Y-m-d\\TH:i:s.vP"
#define RSS					"D, d M Y H:i:s O"
#define W3C					"Y-m-d\\TH:i:sP"
#define DEFAULT_TIMEFORMAT	RFC850

class EZtime {

	friend class Timezone;

	////////// Error handing
	public:
		ezError_t error();									// Returns ezError_t enumeration of last error, resets _last_error to OK 
		String errorString(ezError_t err);					// Human-readable form of last error.
		void debug(ezDebugLevel_t level);					// Sets serial printing of debug info to specified ezDebugLevel_t enumeration
	private:
		void error(ezError_t err);							// Used to set an error
		void debug(ezDebugLevel_t level, String str); 		// Used to print debug info
		void debugln(ezDebugLevel_t level, String str); 	// same, just adds \r\n
		String debugLevelString(ezDebugLevel_t level); 		// Human-readable form of debug level.
		ezError_t _last_error;
		ezDebugLevel_t _debug_level;	
	///////////
	
	public:
		EZtime();
		timeStatus_t timeStatus();
		time_t now();
		void breakTime(time_t time, tmElements_t &tm);  // break time_t into elements
		time_t makeTime(tmElements_t &tm);  // convert time elements into time_t
		time_t makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, int16_t year);
		time_t makeUmpteenthTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t umpteenth, uint8_t wday, uint8_t month, int16_t year);
		time_t compileTime(String compile_date = __DATE__, String compile_time = __TIME__);
		String monthString(uint8_t month);
		String dayString(uint8_t day);
		
	private:
		tzData_t parsePosix(String posix, int16_t year);
		int32_t millisElapsed(unsigned long m);
		time_t _last_sync_time;
		int32_t _fudge;
		unsigned long _last_sync_millis;
		bool _debug_enabled, _ntp_enabled;
		uint16_t _last_read_ms;
		timeStatus_t _time_status;

#ifdef EZTIME_NETWORK_ENABLE

	public:
		bool queryNTP(String server, time_t &t, unsigned long &measured_at);	// measured_at: millis() at measurement, t is converted to secs since 1970
		void updateNow();
		void setServer(String ntp_server = NTP_SERVER);
		void setInterval(uint16_t seconds = 0); 				// 0 = no NTP updates
		bool waitForSync(uint16_t timeout = 0);					// timeout in seconds
#ifdef ESP32
		void clearCache();
#endif // ESP32

	private:
		uint16_t _update_interval;		// in seconds
		time_t _update_due;
		uint16_t _update_backoff;		// seconds to add to _update_due to retry NTP
		uint16_t _ntp_local_port, _ntp_interval, _ntp_max_drift;
		String _ntp_server;

#endif // EZTIME_NETWORK_ENABLE
	
	////////// Free extras ...
	public:
		String urlEncode(String str); 															// Does what you think it does
		String zeropad(uint32_t number, uint8_t length);										// Returns number as string of given length, zero-padded on left if needed
		String getBetween(String &haystack, String before_needle, String after_needle = "");	// Returns what's between before_needle and after_needle in haystack, or "" if not found. Returns until end of string if after_needle is empty
		bool timezoneAPI(String location, String &olsen, String &posix);
	//////////
		
};

extern EZtime ezTime;


class Timezone {

	public:
		Timezone(bool locked_to_UTC = false);
		bool setPosix(String posix);
		String getPosix();
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
		
	private:
		time_t _readTime(time_t t);
		bool _locked_to_UTC;
		tzData_t _tzdata;
		String _posix, _olsen;
		time_t _last_read_t;

#ifdef EZTIME_NETWORK_ENABLE

	public:
		bool setLocation(String location, bool force_lookup = false);
		String getOlsen();

#endif // EZTIME_NETWORK_ENABLE	

};

extern Timezone UTC;

extern Timezone& defaultTZ;

	

#ifdef ARDUINO_TIMELIB_COMPATIBILITY

/*==============================================================================*/
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k
 
/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define dayOfWeek(_time_)  ((( _time_ / SECS_PER_DAY + 4)  % DAYS_PER_WEEK)+1) // 1 = Sunday
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  // this is number of days since Jan 1 1970
#define elapsedSecsToday(_time_)  (_time_ % SECS_PER_DAY)   // the number of seconds since last midnight 
// The following macros are used in calculating alarms and assume the clock is set to a date later than Jan 1 1971
// Always set the correct time before settting alarms
#define previousMidnight(_time_) (( _time_ / SECS_PER_DAY) * SECS_PER_DAY)  // time at the start of the given day
#define nextMidnight(_time_) ( previousMidnight(_time_)  + SECS_PER_DAY )   // time at the end of the given day 
#define elapsedSecsThisWeek(_time_)  (elapsedSecsToday(_time_) +  ((dayOfWeek(_time_)-1) * SECS_PER_DAY) )   // note that week starts on day 1
#define previousSunday(_time_)  (_time_ - elapsedSecsThisWeek(_time_))      // time at the start of the week for the given time
#define nextSunday(_time_) ( previousSunday(_time_)+SECS_PER_WEEK)          // time at the end of the week for the given time


/* Useful Macros for converting elapsed time to a time_t */
#define minutesToTime_t ((M)) ( (M) * SECS_PER_MIN)  
#define hoursToTime_t   ((H)) ( (H) * SECS_PER_HOUR)  
#define daysToTime_t    ((D)) ( (D) * SECS_PER_DAY) // fixed on Jul 22 2011
#define weeksToTime_t   ((W)) ( (W) * SECS_PER_WEEK) 


time_t now();
uint8_t second(time_t t /* = TIME_NOW */);
uint8_t minute(time_t t /* = TIME_NOW */);
uint8_t hour(time_t t /* = TIME_NOW */);
uint8_t day(time_t t /* = TIME_NOW */); 
uint8_t weekday(time_t t /* = TIME_NOW */);
uint8_t month(time_t t /* = TIME_NOW */); 
uint16_t year(time_t t /* = TIME_NOW */); 
uint8_t hourFormat12(time_t t /* = TIME_NOW */);
bool isAM(time_t t /* = TIME_NOW */);
bool isPM(time_t t /* = TIME_NOW */); 
String monthStr(const uint8_t month);
String monthShortStr(const uint8_t month);
String dayStr(const uint8_t month);
String dayShortStr(const uint8_t month);
void setTime(time_t t);
void setTime(const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t month, const uint16_t yr);
void breakTime(time_t t, tmElements_t &tm);
time_t makeTime(tmElements_t &tm);
time_t makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, int16_t year);
timeStatus_t timeStatus();

#endif //ARDUINO_TIMELIB_COMPATIBILITY


#endif //_EZTIME_H_