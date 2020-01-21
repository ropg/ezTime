	String monthStr(const uint8_t month) {
		switch(month) {
			case 1: return  F("Gennaio");
			case 2: return  F("Febbraio");		
			case 3: return  F("Marzo");
			case 4: return  F("Aprile");
			case 5: return  F("Maggio");
			case 6: return  F("Giugno");
			case 7: return  F("Luglio");
			case 8: return  F("Agosto");
			case 9: return  F("Settembre");
			case 10: return F("Ottobre");
			case 11: return F("Novembre");
			case 12: return F("Dicembre");
		}
		return "";
	}

	String monthShortStr(const uint8_t month) { return monthStr(month).substring(0,3); }

	String dayStr(const uint8_t day) {
		switch(day) {
			case 1: return F("Domenica");
			case 2: return F("Lunedì");
			case 3: return F("Martedì");		
			case 4: return F("Mercoledì");
			case 5: return F("Giovedì");
			case 6: return F("Venerdì");
			case 7: return F("Sabato");
		}
		return "";
	}

	String dayShortStr(const uint8_t day) { return dayStr(day).substring(0,3); }
