//SetUpConf.h
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "SPIFFS.h"

#ifndef SETUPCONF_H_INCLUDE
#define SETUPCONF_H_INCLUDE
#define CONF_WITHOUT_TIMEOUT	(0)
#define CONF_WITH_TIMEOUT		(1)
#define CONFIG_FILE_NAME		"/config.txt"
#define DEFAULT_SSID			"abcd"
#define DEFAULT_PASSWD			"1245"
#define DEFAULT_T_DARK_ON		"21:30"
#define DEFAULT_T_DARK_OFF		"05:30"
#define DEFAULT_UTC_OFST		"9"
#define SSIDLIMIT				(30)
#define TIMEOUT_SETUP			(25)
#define WIFIMGR_SSID			"RingClock1"

class SetUpConf {
	public:
		SetUpConf();
        void webConfig(int);
		const char *getSsid();
		const char *getPasswd();
		const char *getTimeDarkOn();
		const char *getTimeDarkOff();
		float getUtcOfst();
		void setWaitCallback(void(*)());
	private:
		void(*waitCallback)();
};

#endif

