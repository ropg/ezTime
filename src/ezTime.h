#ifndef _EZTIME_H_
#ifdef __cplusplus
#define _EZTIME_H_


// Whether or not to be compatible with Paul Stoffregen's Arduino Time Library.
// (which makes a lot of functions available in your root namespace, additionally to through the
// objects created by ezTime.)
#define ARDUINO_TIMELIB_COMPATIBILITY

// Compiles in NTP updating and timezoneapi.io fetching 
#define EZTIME_NETWORK_ENABLE

// Arduino Ethernet shields
#define EZTIME_ETHERNET

// Uncomment one of the below to only put only messages up to a certain level in the compiled code
// (You still need to turn them on with time.debugLevel(someLevel)  to see them)
// #define EZTIME_MAX_DEBUGLEVEL_NONE
// #define EZTIME_MAX_DEBUGLEVEL_ERROR
// #define EZTIME_MAX_DEBUGLEVEL_INFO

// Cache mechanism, either EEPROM or NVS, not both. (See README)
#define EZTIME_CACHE_EEPROM
// #define EZTIME_CACHE_NVS



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


extern "C++" {

////////// Error handing

typedef enum {
	NO_ERROR,
	LAST_ERROR,			// Pseudo-error: replaced by last error
	NO_NETWORK, 
	TIMEOUT,
	CONNECT_FAILED,
	DATA_NOT_FOUND,
	LOCKED_TO_UTC,
	NO_CACHE_SET,
	CACHE_TOO_SMALL,
	TOO_MANY_EVENTS
} ezError_t;

typedef enum {
	NONE, 
	ERROR,
	INFO,
	DEBUG
} ezDebugLevel_t;

typedef enum {
	LOCAL_TIME,
	UTC_TIME
} ezLocalOrUTC_t;

// Defines that can make your code more readable. For example, if you are looking for the first
// Thursday in a year, you could write:  time.makeOrdinalTime(0, 0, 0, JANUARY, FIRST, THURSDAY, year)
// (As is done within ezTime to calculate ISO weeks)

#define SUNDAY			1
#define MONDAY			2
#define TUESDAY			3
#define WEDNESDAY		4
#define THURSDAY		5
#define FRIDAY			6
#define SATURDAY		7

#define JANUARY			1
#define FEBRUARI		2
#define MARCH			3
#define APRIL			4
#define MAY				5
#define JUNE			6
#define JULY			7
#define AUGUST			8
#define SEPTEMBER		9
#define OCTOBER			10
#define NOVEMBER		11
#define DECEMBER		12

#define	FIRST			1
#define	SECOND			2
#define	THIRD			3
#define FOURTH			4
#define LAST			5

#if defined(EZTIME_MAX_DEBUGLEVEL_NONE)
	#define	err(args...) 		""
	#define	errln(args...) 		""
	#define	info(args...) 		""
	#define	infoln(args...) 	""
	#define	debug(args...) 		""
	#define	debugln(args...) 	""
#elif defined(EZTIME_MAX_DEBUGLEVEL_ERROR)
	#define	err(args...) 		if (time._debug_level >= ERROR) Serial.print(args)
	#define	errln(args...) 		if (time._debug_level >= ERROR) Serial.println(args)
	#define	info(args...) 		""
	#define	infoln(args...) 	""
	#define	debug(args...) 		""
	#define	debugln(args...) 	""
#elif defined(EZTIME_MAX_DEBUGLELEL_INFO)
	#define	err(args...) 		if (time._debug_level >= ERROR) Serial.print(args)
	#define	errln(args...) 		if (time._debug_level >= ERROR) Serial.println(args)
	#define	info(args...) 		if (time._debug_level >= INFO) Serial.print(args)
	#define	infoln(args...) 	if (time._debug_level >= INFO) Serial.println(args)
	#define	debug(args...) 		""
	#define	debugln(args...) 	""
#else		// nothing specified compiles everything in.
	#define	err(args...) 		if (time._debug_level >= ERROR) Serial.print(args)
	#define	errln(args...) 		if (time._debug_level >= ERROR) Serial.println(args)
	#define	info(args...) 		if (time._debug_level >= INFO) Serial.print(args)
	#define	infoln(args...) 	if (time._debug_level >= INFO) Serial.println(args)
	#define	debug(args...) 		if (time._debug_level >= DEBUG) Serial.print(args)
	#define	debugln(args...) 	if (time._debug_level >= DEBUG) Serial.println(args)
#endif

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

typedef struct {
	time_t time;
	void (*function)();
} ezEvent_t;

#define MAX_EVENTS			8

#define TIME_NOW			0xFFFFFFFF
#define LAST_READ			0xFFFFFFFE

#define NTP_PACKET_SIZE		48
#define NTP_LOCAL_TIME_PORT		2342
#define NTP_SERVER			"pool.ntp.org"
#define NTP_TIMEOUT			1500			// milliseconds
#define NTP_INTERVAL		600				// default update interval in seconds
#define NTP_RETRY			5				// Retry after this many seconds on failed NTP
#define NTP_STALE_AFTER		3600			// If update due for this many seconds, set timeStatus to timeNeedsSync

#define TIMEZONEAPI_TIMEOUT	2000			// milliseconds

#define EEPROM_CACHE_LEN		50
#define MAX_CACHE_PAYLOAD		((EEPROM_CACHE_LEN - 3) / 3) * 4 + ( (EEPROM_CACHE_LEN - 3) % 3)	// 2 bytes for len and date, then 4 to 3 (6-bit) compression on rest 
#define MAX_CACHE_AGE_MONTHS	2

// Various date-time formats
#define ATOM 				"Y-m-d\\TH:i:sP"
#define COOKIE				"l, d-M-Y H:i:s T"
#define ISO8601				"Y-m-d\\TH:i:sO"
#define RFC822				"D, d M y H:i:s O"
#define RFC850				COOKIE
#define RFC1036				RFC822
#define RFC1123				RFC822
#define RFC2822				RFC822
#define RFC3339 			ATOM
#define RFC3339_EXT			"Y-m-d\\TH:i:s.vP"
#define RSS					RFC822
#define W3C					ATOM
#define ISO8601_YWD			"X-\\Ww-N"			// Note that ISO-8601 Year/Week/Day notation may be one year + or - one at beginning or end of year
#define DEFAULT_TIMEFORMAT	COOKIE


//
//				E Z t i m e   c l a s s
//

class EZtime {

	friend class Timezone;			// Allow methods from Timezone class to access private methods of this class

	////////// Error handing
	public:
		static ezError_t error();								// Returns ezError_t enumeration of last error, resets _last_error to OK 
		static String errorString(ezError_t err = LAST_ERROR);	// Human-readable form of last error.
		static void debugLevel(ezDebugLevel_t level);			// Sets serial printing of debug info to specified ezDebugLevel_t enumeration
	private:
		static void error(ezError_t err);						// Used to set an error
		static String debugLevelString(ezDebugLevel_t level);	// Human-readable form of debug level.
		static ezError_t _last_error;
		static ezDebugLevel_t _debug_level;	
	///////////
	
	public:
		EZtime();
		static timeStatus_t timeStatus();
		static time_t now(bool update_last_read = true);
		static void events();
		static void breakTime(time_t time, tmElements_t &tm);  // break time_t into elements
		static time_t makeTime(tmElements_t &tm);  // convert time elements into time_t
		static time_t makeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint16_t year);
		static time_t makeOrdinalTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t ordinal, uint8_t wday, uint8_t month, uint16_t year);
		static time_t compileTime(String compile_date = __DATE__, String compile_time = __TIME__);
		static String monthString(uint8_t month);
		static String dayString(uint8_t day);
		static bool secondChanged();
		static bool minuteChanged();
		static void deleteEvent(uint8_t event_handle);
		static void deleteEvent(void (*function)());
		
	private:
		static ezEvent_t _events[MAX_EVENTS];
		static time_t _last_sync_time, _last_read_t;
		static uint32_t _last_sync_millis;
		static bool _ntp_enabled;
		static uint16_t _last_read_ms;
		static timeStatus_t _time_status;
		static bool _initialised;

	#ifdef EZTIME_NETWORK_ENABLE

		public:
			static bool queryNTP(String server, time_t &t, unsigned long &measured_at);	// measured_at: millis() at measurement, t is converted to secs since 1970
			static void updateNTP();
			static void setServer(String ntp_server = NTP_SERVER);
			static void setInterval(uint16_t seconds = 0); 				// 0 = no NTP updates
			static bool waitForSync(uint16_t timeout = 0);				// timeout in seconds

		private:
			static uint16_t _ntp_interval;		// in seconds
			static String _ntp_server;

	#endif // EZTIME_NETWORK_ENABLE
	
	////////// Free extras ...
	public:
		static String urlEncode(String str); 														// Does what you think it does
		static String zeropad(uint32_t number, uint8_t length);										// Returns number as string of given length, zero-padded on left if needed
	//////////
		
};

extern EZtime time;			// declares the "time" instance of the "EZtime" class


//
//				T i m e z o n e   c l a s s
//

class Timezone {

	public:
		Timezone(bool locked_to_UTC = false);
		bool setPosix(String posix);
		String getPosix();
		time_t tzTime(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);
		time_t tzTime(time_t t, ezLocalOrUTC_t local_or_utc, String &tzname, bool &is_dst, int16_t &offset);		
		void setDefault();
		bool isDST(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);
		String getTimezoneName(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);
		int16_t getOffset(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);
		time_t now();
		uint8_t setEvent(void (*function)(), time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);
		uint8_t setEvent(void (*function)(), const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t mnth, uint16_t yr);
		void setTime(time_t t, uint16_t ms = 0);
		void setTime(const uint8_t hr, const uint8_t min, const uint8_t sec, const uint8_t day, const uint8_t mnth, uint16_t yr);
		String dateTime(String format = DEFAULT_TIMEFORMAT);
		String dateTime(time_t t, String format = DEFAULT_TIMEFORMAT);
		String dateTime(time_t t, ezLocalOrUTC_t local_or_utc, String format = DEFAULT_TIMEFORMAT);
		uint8_t hour(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);			// 0-23
		uint8_t minute(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);		// 0-59
		uint8_t second(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);		// 0-59
		uint16_t ms(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);			// 0-999
		uint8_t day(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);			// 1-31
		uint8_t weekday(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);		// Day of the week (1-7), Sunday is day 1
		uint8_t month(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);		// 1-12
		uint16_t year(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);		// four digit year
		uint16_t dayOfYear(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);	// days from start of year, jan 1st = 0
		uint8_t weekISO(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);		// ISO-8601 week number (weeks starting on Monday)
		uint16_t yearISO(time_t t = TIME_NOW, ezLocalOrUTC_t local_or_utc = LOCAL_TIME);		// ISO-8601 year, can differ from actual year, plus or minus one
	private:
		String _posix, _olsen;
		bool _locked_to_UTC;
 		
	#ifdef EZTIME_NETWORK_ENABLE
		public:
			bool setLocation(String location = "");
			String getOlsen();
		#ifdef EZTIME_CACHE_EEPROM
			public:
				bool setCache(const int16_t address);
			private:
				int16_t _eeprom_address;
		#endif
		#ifdef EZTIME_CACHE_NVS
			public:
				bool setCache(const String name, const String key);
			private:
				String _nvs_name, _nvs_key;
		#endif
 		#if defined(EZTIME_CACHE_EEPROM) || defined(EZTIME_CACHE_NVS)
 			public:
				void clearCache(bool delete_section = false);
 			private:
 				bool setCache();
  				bool writeCache(const String &str);
 				bool readCache(String &olsen, String &posix, uint8_t &months_since_jan_2018);
 				uint8_t _cache_month;
		#endif
	#endif	

};

extern Timezone UTC;
extern Timezone& defaultTZ;

	

#ifdef ARDUINO_TIMELIB_COMPATIBILITY

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

} // extern "C++"
#endif // __cplusplus
#endif //_EZTIME_H_