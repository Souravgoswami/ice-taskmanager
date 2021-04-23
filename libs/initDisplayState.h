void initDisplayState() {
	display.clearDisplay() ;
	display.setCursor(0, 0) ;

	drawBitmap(iceTMLogoArt, 30, 8) ;
	drawBitmap(iceTMArt, 52, 14) ;

	display.setCursor(20, 34) ;
	display.println(F("Device Initialized")) ;

	display.setCursor(18, 46) ;
	display.println(F("Please run ice-tm")) ;
	display.setCursor(38, 56) ;
	display.println(F("command")) ;
}
