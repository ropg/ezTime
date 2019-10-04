	String monthStr(const uint8_t month) {
		switch(month) {
			case 1: return  F("janvier");
			case 2: return  F("fevrier");		
			case 3: return  F("mars");
			case 4: return  F("avril");
			case 5: return  F("mai");
			case 6: return  F("juin");
			case 7: return  F("juillet");
			case 8: return  F("aout");
			case 9: return  F("septembre");
			case 10: return F("octobre");
			case 11: return F("novembre");
			case 12: return F("decembre");
		}
		return "";
	}

	String monthShortStr(const uint8_t month) { 
		switch(month) {
			case 6: return  F("jun");
			case 7: return  F("jul");
		}	
		return monthStr(month).substring(0,3);
	}

	String dayStr(const uint8_t day) {
		switch(day) {
			case 1: return F("dimanche");
			case 2: return F("lundi");
			case 3: return F("mardi");		
			case 4: return F("mercredi");
			case 5: return F("jeudi");
			case 6: return F("vendredi");
			case 7: return F("samedi");
		}
		return "";
	}

	String dayShortStr(const uint8_t day) { return dayStr(day).substring(0,3); }
