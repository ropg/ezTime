	String monthStr(const uint8_t month) {
		switch(month) {
			case 1: return  F("januari");
			case 2: return  F("februari");		
			case 3: return  F("maart");
			case 4: return  F("april");
			case 5: return  F("mei");
			case 6: return  F("juni");
			case 7: return  F("juli");
			case 8: return  F("augustus");
			case 9: return  F("september");
			case 10: return F("oktober");
			case 11: return F("november");
			case 12: return F("december");
		}
		return "";
	}

	String monthShortStr(const uint8_t month) { return monthStr(month).substring(0,3); }

	String dayStr(const uint8_t day) {
		switch(day) {
			case 1: return F("zondag");
			case 2: return F("maandag");
			case 3: return F("dinsdag");		
			case 4: return F("woensdag");
			case 5: return F("donderdag");
			case 6: return F("vrijdag");
			case 7: return F("zaterdag");
		}
		return "";
	}

	String dayShortStr(const uint8_t day) { return dayStr(day).substring(0,2); }
