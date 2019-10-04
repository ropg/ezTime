	String monthStr(const uint8_t month) {
		switch(month) {
			case 1: return  F("Januar");
			case 2: return  F("Februar");		
			case 3: return  F("März");
			case 4: return  F("April");
			case 5: return  F("Mai");
			case 6: return  F("Juni");
			case 7: return  F("Juli");
			case 8: return  F("August");
			case 9: return  F("September");
			case 10: return F("Oktober");
			case 11: return F("November");
			case 12: return F("Dezember");
		}
		return "";
	}

	String monthShortStr(const uint8_t month) { return monthStr(month).substring(0,3); }

	String dayStr(const uint8_t day) {
		switch(day) {
			case 1: return F("Sonntag");
			case 2: return F("Montag");
			case 3: return F("Dienstag");		
			case 4: return F("Mittwoch");
			case 5: return F("Donnerstag");
			case 6: return F("Freitag");
			case 7: return F("Samstag");
		}
		return "";
	}

	String dayShortStr(const uint8_t day) { return dayStr(day).substring(0,2); }
