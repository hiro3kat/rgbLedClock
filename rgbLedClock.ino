/*
 * file		rgbLedClock.ino
 * autohor  hiro3kat
 * version  V1.0.0
 * date     11 Jan 2023
 * brief    Arduino Clock application for the ESP32 with Full color LED WS2812B x 60
 *          It features setup for WiFi connectivity and time synchronization via NTP.
 */
#include "rgbLedClock.h"
#include "SetUpConf.h"
#include "misc.h"
#include "flushImg.h"
#include "clockImg.h"
#include <ESP32DMASPIMaster.h>
#include <time.h>
#include <sntp.h>

#define ST_NTP_INIT					(0)
#define ST_NTP_WIFI_ON				(1)
#define ST_NTP_WIFI_WAIT			(2)
#define ST_NTP_PROC_WAIT			(3)
#define ST_NTP_NEXT_WAIT			(4)

#define TMS_NTP_TIMEOUT				(40000)

#define IS_NUM_CHAR(c)				('0' <= c && c <= '9')

#define MAX_RETRY_WIFI_CONNECTION	(15)
#define TMS_PRE_WIFI_ON				(15000) //how ling before NTP starts to turn on WIFI [msec]
#define TMS_NTP_INTERVAL			(4*60*60*1000) //NTP interval 4H
#define INIT_SEC					(-1)

ESP32DMASPI::Master master;
static bool bNtpDone, bNtpOk;
static bool bInit, bDarkMode;
static unsigned long tPrv;
static long utcOfsSec;
static int stNtp;
static int tminDarkOn, tminDarkOff;
static char ssid[80];
static char password[80];
static uint32_t tmStartProcNtp, tmLastNtp;
static uint32_t tMsPrv;
static struct tm *pTm;
static int cntSetupInd;
static int setupMode;
static int minPrv;

#define LEN_DMA_BUF			(LEN_LED_BUF * 3 * 3) // * colors * SPI bits array
static RGB ledRgbBuf[LEN_LED_BUF];

static uint8_t* ptSpiTxBuf;

void setup() {
	int len;
    Serial.begin(115200);
	//allocate buffer for DMA transfer
    ptSpiTxBuf = master.allocDMABuffer(LEN_DMA_BUF);
    master.setDataMode(SPI_MODE0);          // default: SPI_MODE0
    master.setFrequency(2800000);           // 28MHz -> tcyc:357ns
    master.setMaxTransferSize(LEN_DMA_BUF); // default: 4092 bytes
    master.begin(HSPI);		//HSPI (CS: 15, CLK: 14, MOSI: 13, MISO: 12)
    //master.begin(VSPI);  // VSPI (CS: 5, CLK: 18, MOSI: 23, MISO: 19)
	memset(ledRgbBuf, 0, sizeof(ledRgbBuf));
	cntSetupInd = 0;
	bNtpDone = false;
#if 1 //setup phase
	SetUpConf sc = SetUpConf();
	int retry = 0;
	setupMode = CONF_WITH_TIMEOUT;
	WiFi.begin(sc.getSsid(), sc.getPasswd());
  	while (WiFi.status() != WL_CONNECTED) {
    	delay(200); //200ms
    	retry++;
    	if (retry >= MAX_RETRY_WIFI_CONNECTION) { 
        	setupMode = CONF_WITHOUT_TIMEOUT;
			break;
		}
    }
	if(CONF_WITH_TIMEOUT == setupMode) {
		sc.setWaitCallback(progress);
	}
	else {
		Serial.print("setup wait");
	}
	sc.webConfig(setupMode);
	strcpy(ssid, sc.getSsid());
	strcpy(password, sc.getPasswd());
	utcOfsSec = sc.getUtcOfst() * 3600;
	tminDarkOn = convH_M2MinuteInt(sc.getTimeDarkOn());
	tminDarkOff = convH_M2MinuteInt(sc.getTimeDarkOff());
	if(tminDarkOn > tminDarkOff) tminDarkOff += (24 * 60);
	stNtp = ST_NTP_INIT;
	do {
		stNtp = procNtp(stNtp);
		delay(1000);
		Serial.println("...NTP");
		if(LEN_LED_BUF > cntSetupInd) {
			ledRgbBuf[cntSetupInd++].g = SETUP_IND_BRIGHTNESS;
			int len = convRgb2SpiBitStream(ptSpiTxBuf, ledRgbBuf, LEN_LED_BUF);
   			master.transfer(ptSpiTxBuf, len);
		}
	} while(ST_NTP_NEXT_WAIT != stNtp);
	Serial.println("done NTP");
#endif
  	tPrv = 0;
  	bInit = true;
	bDarkMode = false;
	tMsPrv = 0;
	minPrv = 0;
	initFlushImg();
	initClockImg();
}

//Callback processing for completion of NTP processing
void timeSyncCallback(struct timeval *tv)
{
	bNtpDone = true;
	Serial.println("ntp ok!!");
}

//NTP processing
static int procNtp(int state)
{
	uint32_t tm = millis();
	switch(state) {
	case ST_NTP_INIT:
		configTime(utcOfsSec, 0, "ntp.nict.jp", 
								 "time.google.com", 
								 "ntp.jst.mfeed.ad.jp");
		sntp_set_sync_interval(TMS_NTP_INTERVAL);		
		sntp_set_time_sync_notification_cb(timeSyncCallback);
		tmStartProcNtp = 0;
		bNtpOk = false;
		//no break;
	case ST_NTP_WIFI_ON:
		WiFi.begin(ssid, password);
		state = ST_NTP_WIFI_WAIT;
		bNtpDone = false;
		tmStartProcNtp = tm;
		break;
	case ST_NTP_WIFI_WAIT:
		if(WL_CONNECTED == WiFi.status()) {
			state = ST_NTP_PROC_WAIT;
		}
		break;
	case ST_NTP_PROC_WAIT:
		if(bNtpDone) {
			WiFi.disconnect(true);
			WiFi.mode(WIFI_OFF);
			state = ST_NTP_NEXT_WAIT;
			tmLastNtp = tm;
			tmStartProcNtp = 0;
			bNtpOk = true;
		}
		break;
	case ST_NTP_NEXT_WAIT:
		if((tm - tmLastNtp) >= (TMS_NTP_INTERVAL - TMS_PRE_WIFI_ON)) {
			state = ST_NTP_WIFI_ON;
		}
		break;
	}
	if(0 != tmStartProcNtp && bNtpOk) { 
		if(tm - tmStartProcNtp >= TMS_NTP_TIMEOUT) {
			WiFi.disconnect(true);
			WiFi.mode(WIFI_OFF);
			tmLastNtp += TMS_NTP_INTERVAL;
			state = ST_NTP_NEXT_WAIT;
			tmStartProcNtp = 0;
		}
	}
	return state;
}

//Convert hour:minute string to integer value
static int convH_M2MinuteInt(const char *phm)
{
	int h, m;
	if(IS_NUM_CHAR(*phm)) {
		h = *phm - '0';
		phm++;
		if(IS_NUM_CHAR(*phm)) {
			h = h * 10 + *phm - '0';
			phm++;
		}
		if(':' == *phm) {
			phm++;
			if(IS_NUM_CHAR(*phm)) {
				m = *phm - '0';
				phm++;
				if(IS_NUM_CHAR(*phm)) {
					m = m * 10 + *phm - '0';
				}
			}
			else {
				h = m = 0;
			}
		}
		else {
			h = m = 0;
		}
	}
	else {
		h = m = 0;
	}
	return h * 60 + m;
}

//Check if the current time is dark mode ON
static bool chkDarkMode(struct tm *pTm, int tminOn, int tminOff)
{
	bool res = false;
	int tminAct = pTm->tm_hour * 60 + pTm->tm_min;
	if(24*60 <= tminOff) {
		if(tminAct >= tminOn && tminAct <= 24*60) {
			res = true;
		}
		else if(tminAct < tminOff-24*60 && tminAct+24*60 < tminOff) {
			res = true;
		}
	}
	else if(tminAct >= tminOn && tminAct < tminOff) {
		res = true;
	}
	return res;
}

//Setup mode progress callback processing
static void progress(void)
{
	Serial.print("*");
	if(LEN_LED_BUF > cntSetupInd) {
		if(CONF_WITHOUT_TIMEOUT == setupMode) {
			ledRgbBuf[cntSetupInd++].r = SETUP_IND_BRIGHTNESS;
		}
		else {
			ledRgbBuf[cntSetupInd++].b = SETUP_IND_BRIGHTNESS;
		}
		int len = convRgb2SpiBitStream(ptSpiTxBuf, ledRgbBuf, LEN_LED_BUF);
   		master.transfer(ptSpiTxBuf, len);
	}
}

void loop() {
	time_t t;
	char buf[80];
	int len, hourPos;
	uint32_t tMs = millis();
	bool bLedDataChanged = false;
	stNtp = procNtp(stNtp);
	t = time(NULL);
	if(tMs - tMsPrv >= TMS_INTERVAL) {
		tMsPrv = tMs;
		pTm = localtime(&t);
		if(minPrv != pTm->tm_min) {
			minPrv = pTm->tm_min;
			if(tminDarkOn != tminDarkOff) {
				if(chkDarkMode(pTm, tminDarkOn, tminDarkOff)) {
					if(!bDarkMode) {
						setClockImgDarkMode(true);
						setFlushImgDarkMode(true);
						bDarkMode = true;
					}
				}
				else if(bDarkMode) {
					setClockImgDarkMode(false);
					setFlushImgDarkMode(false);
					bDarkMode = false;
				}
			}
		}
		setFlushImg(ledRgbBuf);
		setMinImg(ledRgbBuf, pTm->tm_min);
		hourPos = ((pTm->tm_hour * 5) + (pTm->tm_min * 5 / 60)) % 60;
		setHourImg(ledRgbBuf, hourPos);
		setSecImg(ledRgbBuf, pTm->tm_sec);
		len = convRgb2SpiBitStream(ptSpiTxBuf, ledRgbBuf, LEN_LED_BUF);
   		master.transfer(ptSpiTxBuf, len);
	}
    delay(1);
}

