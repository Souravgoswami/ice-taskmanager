#pragma GCC optimize("Os")
// Libraries needed: Adafruit_BusIO, Adafruit VEML 6075, Adafruit GFX, Adafruit SSD1306

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C
#define DELAY 750
#define PARTICLES 20
#define FONT Aldrich_Regular5pt7b
#define FONTBIG Aldrich_Regular6pt7b
#define ARGLEN 101
#define ARGLEN_1 102
#define SCANID F("IceTM")
#define EXPIRY 3000
#define BAUDRATE 57600

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) ;

#include "Fonts/aldrich-regular-5.h"
#include "Fonts/aldrich-regular-6.h"
#include "libs/sysData.h"
#include "libs/arts.h"
#include "libs/drawBitmap.h"
#include "libs/substr.h"
#include "libs/particles.h"
#include "libs/initDisplayState.h"

static struct sysData info ;
char disp[10] ;
char recv[ARGLEN_1 + 2] ;
float v ;

unsigned long currTime ;
unsigned long delayTime = millis() + DELAY ;
unsigned long dataReceiveExpiry ;

char *getUnit(unsigned char unit) {
	if (unit == 0) return " B" ;
	else if (unit == 1) return "KB" ;
	else if (unit == 2) return "MB" ;
	else if (unit == 3) return "GB" ;
	else if (unit == 4) return "TB" ;
	else if (unit == 5) return "PB" ;
	else if (unit == 6) return "EB" ;
	else if (unit == 7) return "ZB" ;
	else return "YB" ;
}

unsigned char getData(char *recv, unsigned char len) {
	char temp_ch ;
	char str[2] ;
	unsigned char counter = 0 ;
	temp_ch = Serial.read() ;
	unsigned long timeout = millis() + 100 ;

	if(temp_ch == 33) {
		recv[0] = '\0' ;
		while(true) {
			if(millis() > timeout) {
				return 0 ;
			}

			temp_ch = Serial.read() ;
			if(temp_ch == 126) break ;

			if(temp_ch > 47 && temp_ch < 58 && counter < len) {
				sprintf(str, "%c", temp_ch) ;
				recv[counter++] = str[0] ;
			}
		}

		recv[++counter] = '\0' ;
		if (strlen(recv) == len) {
			return 1 ;
		} else {
			return 0 ;
		}
	}
}

void getDataInit() {
	char temp_ch ;

	while(1) {
		Serial.println(SCANID) ;
		Serial.flush() ;

		initDisplayState() ;
		drawParticles() ;
		display.display() ;

		if(Serial.read()== 33) {
			unsigned long tm = millis() + 100 ;

			display.display() ;

			while(tm > millis()) {
				display.clearDisplay() ;
				display.setCursor(20, 30) ;
				display.println(F("Synchronizing...")) ;
				drawParticles() ;
				display.display() ;

				Serial.write(0x7e) ;
				Serial.flush() ;
			}

			return ;
		}
	}
}

void setup() {
	Serial.begin(BAUDRATE) ;

	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
		while (1) ; // Don't proceed, loop forever
	}

	display.setTextSize(1) ;
	display.setTextWrap(true) ;
	display.setFont(&FONT) ;
	display.setTextColor(WHITE) ;
	display.clearDisplay() ;
	display.setCursor(0, 0) ;

	initParticles() ;
	getDataInit() ;
	dataReceiveExpiry = millis() + EXPIRY ;
}

void loop() {
	display.clearDisplay() ;
	display.setCursor(0, 8) ;

	// cpu(01234) memUsed(999993) swapUsed(999992) io_active(0)
	// netUpload(999991) netDownload(999990)
	// disktotal(999990) diskused(999990) diskAvail(999990) totalProcess(12345678)
	// memPercent(01234) swapPercent(01234) diskPercent(01234) diskAvailPercent(01234)
	// psR(65536) psS(65536) psI(65536) psT(65536) psZ(65536)
	if (getData(recv, ARGLEN)) {
		// CPU
		substr(recv, disp, 0, 2) ;
		info.cpu_usage = atoi(disp) ;
		substr(recv, disp, 3, 4) ;
		v = atoi(disp) ;
		info.cpu_usage += v / 100.0 ;

		// Mem
		substr(recv, disp, 5, 7) ;
		info.mem_usage = atoi(disp) ;
		substr(recv, disp, 8, 9) ;
		v = atoi(disp) ;
		info.mem_usage += v / 100.0 ;
		substr(recv, disp, 10, 10) ;
		info.mem_unit = getUnit(atoi(disp)) ;

		// Swap
		substr(recv, disp, 11, 13) ;
		info.swap_usage = atoi(disp) ;
		substr(recv, disp, 14, 15) ;
		v = atoi(disp) ;
		info.swap_usage += v / 100.0 ;
		substr(recv, disp, 16, 16) ;
		info.swap_unit = getUnit(atoi(disp)) ;

		// IO Read
		substr(recv, disp, 17, 17) ;
		info.io_active = atoi(disp) > 0 ? 1 : 0 ;

		// Net Upload
		substr(recv, disp, 18, 20) ;
		info.net_upload = atoi(disp) ;
		substr(recv, disp, 21, 22) ;
		v = atoi(disp) ;
		info.net_upload += v / 100.0 ;
		substr(recv, disp, 23, 23) ;
		info.net_upload_unit = getUnit(atoi(disp)) ;

		// Net Download
		substr(recv, disp, 24, 26) ;
		info.net_download = atoi(disp) ;
		substr(recv, disp, 27, 28) ;
		v = atoi(disp) ;
		info.net_download += v / 100.0 ;
		substr(recv, disp, 29, 29) ;
		info.net_download_unit = getUnit(atoi(disp)) ;

		// Disk total
		substr(recv, disp, 30, 32) ;
		info.disk_total = atoi(disp) ;
		substr(recv, disp, 33, 34) ;
		v = atoi(disp) ;
		info.disk_total += v / 100.0 ;
		substr(recv, disp, 35, 35) ;
		info.disk_total_unit = getUnit(atoi(disp)) ;

		// Disk Used
		substr(recv, disp, 36, 38) ;
		info.disk_used = atoi(disp) ;
		substr(recv, disp, 39, 40) ;
		v = atoi(disp) ;
		info.disk_used += v / 100.0 ;
		substr(recv, disp, 41, 41) ;
		info.disk_used_unit = getUnit(atoi(disp)) ;

		// Disk Available
		substr(recv, disp, 42, 44) ;
		info.disk_avail = atoi(disp) ;
		substr(recv, disp, 45, 46) ;
		v = atoi(disp) ;
		info.disk_avail += v / 100.0 ;
		substr(recv, disp, 47, 47) ;
		info.disk_avail_unit = getUnit(atoi(disp)) ;

		// Total Process
		substr(recv, disp, 48, 55) ;
		info.ps_count = strtoul(disp, NULL, 10) ;

		// Mem Percent
		substr(recv, disp, 56, 58) ;
		info.mem_percent = atoi(disp) ;
		substr(recv, disp, 59, 60) ;
		v = atoi(disp) ;
		info.mem_percent += v / 100.0 ;

		// Swap Percent
		substr(recv, disp, 61, 63) ;
		info.swap_percent = atoi(disp) ;
		substr(recv, disp, 64, 65) ;
		v = atoi(disp) ;
		info.swap_percent += v / 100.0 ;

		// Disk Used Percent
		substr(recv, disp, 66, 68) ;
		info.disk_used_percent = atoi(disp) ;
		substr(recv, disp, 69, 70) ;
		v = atoi(disp) ;
		info.disk_used_percent += v / 100.0 ;

		// Disk Avail Percent
		substr(recv, disp, 71, 73) ;
		info.disk_avail_percent = atoi(disp) ;
		substr(recv, disp, 74, 75) ;
		v = atoi(disp) ;
		info.disk_avail_percent += v / 100.0 ;

		substr(recv, disp, 76, 80) ;
		info.process_running = (unsigned int)strtoul(disp, NULL, 10) ;

		substr(recv, disp, 81, 85) ;
		info.process_sleeping = (unsigned int)strtoul(disp, NULL, 10) ;

		substr(recv, disp, 86, 90) ;
		info.process_idle = (unsigned int)strtoul(disp, NULL, 10) ;

		substr(recv, disp, 91, 95) ;
		info.process_stopped = (unsigned int)strtoul(disp, NULL, 10) ;

		substr(recv, disp, 96, 100) ;
		info.process_zombie = (unsigned int)strtoul(disp, NULL, 10) ;

		dataReceiveExpiry = millis() + EXPIRY ;
	}

	currTime = millis() ;
	if (currTime > dataReceiveExpiry) {
		getDataInit() ;
		dataReceiveExpiry = millis() + EXPIRY ;
	} else {
		repeatFirstPage:
		if (currTime < delayTime) {
			// Show page
			drawBitmap(oneArt, 118, 56) ;

			// CPU Usage
			dtostrf(info.cpu_usage, 5, 2, disp) ;
			sprintf(disp, "%s%%", disp) ;
			drawBitmap(cpuArt) ;
			display.setCursor(16, 8) ;
			display.setFont(&FONTBIG) ;
			display.println(F("CPU")) ;
			display.setFont(&FONT) ;
			display.println(disp) ;
			disp[0] = '\0' ;

			display.println(F("")) ;

			// Process Count
			drawBitmap(processArt, 0, 32) ;
			display.setFont(&FONTBIG) ;
			display.print(F("   PS (")) ;
			display.print(info.ps_count) ;
			display.println(F(")")) ;
			display.setFont(&FONT) ;

			// Process states
			sprintf(disp, "R: %u | ", info.process_running) ;
			display.print(disp) ;
			disp[0] = '\0' ;

			sprintf(disp, "S: %u | ", info.process_sleeping) ;
			display.print(disp) ;
			disp[0] = '\0' ;

			sprintf(disp, "I: %u", info.process_idle) ;
			display.println(disp) ;
			disp[0] = '\0' ;

			sprintf(disp, "St: %u | ", info.process_stopped) ;
			display.print(disp) ;
			disp[0] = '\0' ;

			sprintf(disp, "Z: %u", info.process_zombie) ;
			display.println(disp) ;
			disp[0] = '\0' ;

			display.println(F("")) ;
		} else if (currTime < delayTime + DELAY) {
			// Show page
			drawBitmap(twoArt, 118, 56) ;

			// Memory Usage
			drawBitmap(memArt) ;
			display.setCursor(20, 8) ;

			dtostrf((info.mem_usage), 5, 2, disp) ;
			sprintf(disp, "%s %s", disp, info.mem_unit) ;
			display.setFont(&FONTBIG) ;
			display.println(F("MEMORY")) ;
			display.setFont(&FONT) ;
			display.print(disp) ;
			disp[0] = '\0' ;
			dtostrf(info.mem_percent, 5, 2, disp) ;
			sprintf(disp, "%s %%", disp) ;
			display.print(F(" (")) ;
			display.print(disp) ;
			display.println(")") ;
			disp[0] = '\0' ;

			display.println(F("")) ;

			// Swap Usage
			drawBitmap(swapArt, 0, 30) ;
			dtostrf(info.swap_usage, 5, 2, disp) ;
			sprintf(disp, "%s %s", disp, info.swap_unit) ;
			display.setFont(&FONTBIG) ;
			display.println(F("     SWAP")) ;
			display.setFont(&FONT) ;
			display.print(disp) ;
			disp[0] = '\0' ;
			dtostrf(info.swap_percent, 5, 2, disp) ;
			sprintf(disp, "%s %%", disp) ;
			display.print(" (") ;
			display.print(disp) ;
			display.println(")") ;
			disp[0] = '\0' ;

			display.println(F("")) ;
		} else if (currTime < delayTime + DELAY * 2) {
			// Show page
			drawBitmap(threeArt, 118, 56) ;

			// IO Usage
			drawBitmap(ioArt) ;
			display.setCursor(18, 8) ;
			display.setFont(&FONTBIG) ;
			display.println(F("IO")) ;
			display.setFont(&FONT) ;
			display.println(info.io_active == 0 ? F("Idle") : F("Busy")) ;

			display.println(F("")) ;

			// Network Usage
			dtostrf(info.net_upload, 5, 2, disp) ;
			sprintf(disp, "%s %s/S", disp, info.net_upload_unit) ;
			drawBitmap(netArt, 0, 34) ;
			display.setFont(&FONTBIG) ;
			display.println(F("    NET")) ;
			display.setFont(&FONT) ;
			display.print(F("Up: ")) ;
			display.println(disp) ;
			disp[0] = '\0' ;
			dtostrf(info.net_download, 5, 2, disp) ;
			sprintf(disp, "%s %s/S", disp, info.net_download_unit) ;
			display.print(F("Dl: ")) ;
			display.println(disp) ;
			disp[0] = '\0' ;

			display.println(F("")) ;
		} else if (currTime < delayTime + DELAY * 3) {
			// Show page
			drawBitmap(fourArt, 118, 56) ;

			// Disk
			drawBitmap(diskArt) ;
			display.setCursor(16, 9) ;
			display.setFont(&FONTBIG) ;
			display.println(F("DISK")) ;
			display.setFont(&FONT) ;
			dtostrf(info.disk_total, 5, 2, disp) ;
			sprintf(disp, "%s %s", disp, info.disk_total_unit) ;
			display.print(F("T: ")) ;
			display.print(disp) ;
			disp[0] = '\0' ;
			display.println(F("")) ;

			dtostrf(info.disk_used, 5, 2, disp) ;
			sprintf(disp, "%s %s", disp, info.disk_used_unit) ;
			display.print(F("U: ")) ;
			display.print(disp) ;
			disp[0] = '\0' ;

			// Disk Used Percent
			dtostrf(info.disk_used_percent, 5, 2, disp) ;
			sprintf(disp, "%s %%", disp) ;
			display.print(F(" (")) ;
			display.print(disp) ;
			display.println(F(")")) ;
			disp[0] = '\0' ;

			// Disk Available
			dtostrf(info.disk_avail, 5, 2, disp) ;
			sprintf(disp, "%s %s", disp, info.disk_avail_unit) ;
			display.print(F("Av: ")) ;
			display.print(disp) ;
			disp[0] = '\0' ;

			// Disk available percent
			dtostrf(info.disk_avail_percent, 5, 2, disp) ;
			sprintf(disp, "%s %%", disp) ;
			display.print(F(" (")) ;
			display.print(disp) ;
			display.print(F(")")) ;
			disp[0] = '\0' ;
		} else {
			delayTime = millis() + DELAY ;
			goto repeatFirstPage ;
		}
	}

	drawParticles() ;
	display.display() ;
}
