#include <Arduino.h>
#include "SetUpConf.h"
#include "SPIFFS.h"

static IPAddress apIP(192, 168, 1, 100);
static DNSServer dnsServer;
static WebServer webServer(80);
static String ssid;
static String passwd;
static String tDarkOn;
static String tDarkOff;
static String utcOfst;
static bool bExit, bConnected;

static String ssid_rssi_str[SSIDLIMIT];
static String ssid_str[SSIDLIMIT];

static void writeConfigFile()
{
  	Serial.print("writeConfigFile");
  	File fw = SPIFFS.open(CONFIG_FILE_NAME, "w");
  	fw.println(ssid);
  	fw.println(passwd);
  	fw.println(tDarkOn);
  	fw.println(tDarkOff);
	fw.println(utcOfst);
  	fw.close();
}

static void initConfigFile()
{
	Serial.print("initConfigFile");
    ssid = DEFAULT_SSID;
    passwd = DEFAULT_PASSWD;
    tDarkOn = DEFAULT_T_DARK_ON;
    tDarkOff = DEFAULT_T_DARK_OFF;
	utcOfst = DEFAULT_UTC_OFST;
	writeConfigFile();
}

static void readConfigFile()
{
  	String numstr;
  	File fr = SPIFFS.open(CONFIG_FILE_NAME, "r");
  	if (fr) {
    	ssid = fr.readStringUntil('\n');
    	ssid.trim();
   		if (ssid =="") ssid = DEFAULT_SSID;

    	passwd = fr.readStringUntil('\n');
    	passwd.trim();
    	if(passwd == "") passwd = DEFAULT_PASSWD;

    	tDarkOn = fr.readStringUntil('\n');
    	tDarkOn.trim();
    	if(tDarkOn == "") tDarkOn = DEFAULT_T_DARK_ON;

    	tDarkOff = fr.readStringUntil('\n');
    	tDarkOff.trim();
    	if(tDarkOff == "") tDarkOff = DEFAULT_T_DARK_OFF;

    	utcOfst = fr.readStringUntil('\n');
    	utcOfst.trim();
    	if(utcOfst == "") utcOfst = DEFAULT_UTC_OFST;

    	fr.close();
		Serial.println(ssid);
		Serial.println(passwd);
		Serial.println(tDarkOn);
		Serial.println(tDarkOff);
		Serial.println(utcOfst);
		Serial.println("readConfigFile done!!");
  	} 
	else {
    	Serial.println("read open error");
    	Serial.print("SPIFFS data seems clash. Default load...");
    	initConfigFile();
  	}
}

static String WIFI_Form_str()
{
  	uint8_t ssid_num = WiFi.scanNetworks();
  	if(0 == ssid_num) {
    	Serial.println("no networks found");
  	} 
	else {
    	Serial.printf("%d networks found\r\n", ssid_num);
    	if (ssid_num > SSIDLIMIT) ssid_num = SSIDLIMIT;
    	for (int i = 0; i < ssid_num; ++i) {
      		ssid_str[i] = WiFi.SSID(i);
      		String wifi_auth_open = 
				((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      		ssid_rssi_str[i] = ssid_str[i];
      		Serial.printf("%d: %s\r\n", i, ssid_rssi_str[i].c_str());
      		delay(10);
    	}
  	}
  	String str = "";
  	str += "<form action='/wifiset' method='get'>";
  	str += "SSID<br>";
  	str += "<select name='ssid' id ='ssid'>";
  	for(int i=0; i<ssid_num; i++){
    	str += "<option value=" + ssid_str[i] + ">" + 
							ssid_rssi_str[i] + "</option>";
  	}
  	str += "</select><br>\r\n";

  	str += "<br>Password<br><input type='password' name='passwd' value='" + 
				passwd + "' size='15'>";
  	str += "</select><br>\r\n";

  	str += "<br>Time dark mode ON<br><input type='time' name='tDarkOn' value='" + 
				tDarkOn + "' size='6'><br>";

  	str += "<br>Time dark mode OFF<br><input type='time' name='tDarkOff' value='" + 
				tDarkOff + "' size='6'><br>";

	str += "<br>UTC Offset<br><input type='number' step='0.05' name='utcOfst' \
min='-10.0' max='14.0' value='" + utcOfst + "' size='4'><br>";
	
  	str += "<br><input type='submit' value='set'>";

  	str += "</form><br>";
  	str += "<script>document.getElementById('ssid').value = '"+ 
				ssid +"';</script>";
  	return str;
}

static String Headder_str() {
  	String html = "";
  	html += "<!DOCTYPE html><html><head>";
  	html += "<meta name='viewport' content='width=device-width, initial-scale=1.3'>";
  	html += "<meta http-equiv='Pragma' content='no-cache'>";
  	html += "<meta http-equiv='Cache-Control' content='no-cache'></head>";
  	html += "<meta http-equiv='Expires' content='0'>";
  	html += "<style>";
  	html += "a:link, a:visited { background-color: #009900; color: white; padding: 5px 15px;";
  	html += "text-align: center; text-decoration: none;  display: inline-block;}";
  	html += "a:hover, a:active { background-color: green;}";
  	html += "bo32{ background-color: #EEEEEE;}";
  	html += "input[type=button], input[type=submit], input[type=reset] {";
  	html += "background-color: #000099;  border: none;  color: white;  padding: 5px 20px;";
  	html += "text-decoration: none;  margin: 4px 2px;";
  	html += "</style>";
  	html += "<body>"; 
  	html += "<h2>RING CLOCK SETUP</h2>";
  	return html;
}

static void appStart() 
{
  	String html = Headder_str();
  	html += "<hr><p>";
  	html += "<h3>Please wait for a while.</h3><p>";
  	html += "<hr>";
  	html += "</body></html>";
  	webServer.send(200, "text/html", html);
  	delay(2000); // hold 2 sec
	bExit = true;
}

static void appStartConf() 
{
  	String html = Headder_str();
  	html += "<hr><p>";
  	html += "<h3>Setup Exit?</h3><p>";
  	html += "<center><a href='/appStart'>YES</a> <a href='/'>no</a></center>";
  	html += "<p><hr>";
  	html += "</body></html>";
  	webServer.send(200, "text/html", html);
}

static void wifiinput() 
{
	bConnected = true;
  	String html = Headder_str();
  	html += "<hr><p>";
  	html += WIFI_Form_str();
  	html += "</body></html>";
  	webServer.send(200, "text/html", html);
}

static void wifiset()
{
  	ssid = webServer.arg("ssid");
  	ssid.trim();

  	passwd = webServer.arg("passwd");
  	passwd.trim();

  	tDarkOn = webServer.arg("tDarkOn");
  	tDarkOn.trim();

  	tDarkOff = webServer.arg("tDarkOff");
  	tDarkOff.trim();

	utcOfst = webServer.arg("utcOfst");
	utcOfst.trim();
 
  	writeConfigFile();
	appStartConf();
}

SetUpConf::SetUpConf()
{
	Serial.println("SetUpConf()");
	if(!SPIFFS.begin(true)) {
		Serial.println("SPIFFS mount failed!!");
	}
	readConfigFile();
	Serial.println("SetUpConf exit.");
	
}

void SetUpConf::webConfig(int mode)
{
	bConnected = false;
	if(CONF_WITH_TIMEOUT == mode) {
		Serial.println("webConf: WITH_TIMEOUT");
	}
	else {
		Serial.println("webConf: WITHOUT_TIMEOUT");
	}
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
	delay(100);
	WiFi.mode(WIFI_AP);
	WiFi.softAP(WIFIMGR_SSID);
	delay(200);
	WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(53, "*", apIP);
    webServer.on("/", HTTP_GET, wifiinput);
	webServer.on("/wifiset", HTTP_GET, wifiset);
	webServer.on("/appStart", appStart);
	webServer.onNotFound(wifiinput);
	webServer.begin();
	bExit = false;
	unsigned long tmAct, tmPrv;
	int cnt = TIMEOUT_SETUP;
	tmAct = tmPrv = millis();
	while(!bExit) {
		dnsServer.processNextRequest();
		webServer.handleClient();
		if(CONF_WITH_TIMEOUT == mode) {
			if(bConnected) {
				mode = CONF_WITHOUT_TIMEOUT;
			}
			else {
				tmAct = millis();
				if(1000 <= tmAct - tmPrv) {
					tmPrv = tmAct;
					Serial.print(".");
					this->waitCallback();
					if(0 >= --cnt) break;
				}
			}	
		}
	}
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);	
}

const char* SetUpConf::getSsid(void)
{
	return ssid.c_str();
}

const char* SetUpConf::getPasswd(void)
{
	return passwd.c_str();
}

const char *SetUpConf::getTimeDarkOn(void)
{
	return tDarkOn.c_str();
}

const char *SetUpConf::getTimeDarkOff(void)
{
	return tDarkOff.c_str();
}

float SetUpConf::getUtcOfst(void)
{
	return utcOfst.toFloat();
}

void SetUpConf::setWaitCallback(void(*pt)())
{
	this->waitCallback = pt;
}
