#include "rgbLedClock.h"
#include "misc.h"

#define INIT_SEC				(-1)
#define INIT_MIN				(-1)
#define INIT_HOUR				(-1)

static int secPrv, minPrv, hourPrv, cntSecImg, cntMinImg, cntHourImg, minSteps, hourSteps;
static uint8_t maxVal;

void initClockImg(void)
{
	secPrv = INIT_SEC;
	minPrv = INIT_MIN;
	hourPrv = INIT_HOUR;
	maxVal = STD_BRIGHTNESS;
	cntSecImg = cntMinImg, cntHourImg = 0;
	minSteps = STEPS_MINUTE_UPDATE;
	hourSteps = STEPS_HOUR_UPDATE;
}

void setClockImgDarkMode(bool b)
{
	maxVal = b ? DARK_BRIGHTNESS : STD_BRIGHTNESS;
}

void setSecImg(RGB *pt, int sec)
{
	int pos;
	HSV hsv;
	if(INIT_SEC != secPrv) {
		if(secPrv != sec) {
			cntSecImg = 0;
		}
		pos = sec;
		hsv.h = HUE_SEC;
		hsv.s = CLOCK_IMG_SATURATION;
		hsv.v = maxVal * (STEPS_IN_SEC - cntSecImg) / STEPS_IN_SEC;
		hsv2rgb(&pt[pos], &hsv);
		pos++;
		if(60 <= pos) pos = 0;
		hsv.v = maxVal * cntSecImg / STEPS_IN_SEC;
		hsv2rgb(&pt[pos], &hsv);
		cntSecImg++;
	}
	secPrv = sec;	
}


void setMinImg(RGB *pt, int min)
{
	int pos;
	HSV hsv;
	bool bSet = true;
	if(INIT_MIN != minPrv) {
		if(minPrv != min) {
			cntMinImg = 0;
		}
		if(minSteps >= cntMinImg) {
			pos = min - 2;
			hsv.h = HUE_MIN;	
			hsv.s = CLOCK_IMG_SATURATION;
			if(0 > pos) pos += 60;
			hsv.v = (maxVal / 4) * (minSteps - cntMinImg) / minSteps;
			hsv2rgb(&pt[pos], &hsv);

			pos++;
			if(60 <= pos) pos -= 60;
			hsv.v = (maxVal / 4) + (maxVal * 3 / 4) * (minSteps - cntMinImg) / minSteps; 
			hsv2rgb(&pt[pos], &hsv);

			pos++;
			if(60 <= pos) pos -= 60;
			hsv.v = (maxVal / 4) + (maxVal * 3 / 4) * cntMinImg / minSteps;
			hsv2rgb(&pt[pos], &hsv);

			pos++;
			if(60 <= pos) pos -= 60;
			hsv.v = (maxVal / 4) * cntMinImg / minSteps;
			hsv2rgb(&pt[pos], &hsv);

			cntMinImg++;
			bSet = false;
		}
	}
	if(bSet) {
		pos = min - 1;
		hsv.h = HUE_MIN;	
		hsv.s = CLOCK_IMG_SATURATION;

		if(0 > pos) pos += 60;
		hsv.v = maxVal / 4;
		hsv2rgb(&pt[pos], &hsv);

		pos++;
		if(60 <= pos) pos -= 60;
		hsv.v = maxVal;
		hsv2rgb(&pt[pos], &hsv);

		pos++;
		if(60 <= pos) pos -= 60;
		hsv.v = maxVal / 4;
		hsv2rgb(&pt[pos], &hsv);
	}
	minPrv = min;
}

void setHourImg(RGB *pt, int hour)
{
	int pos;
	HSV hsv;
	bool bSet = true;
	if(INIT_HOUR != hourPrv) {
		if(hourPrv != hour) {
			cntHourImg = 0;
		}
		if(hourSteps >= cntHourImg) {
			pos = hour - 2;
			hsv.h = HUE_HOUR;
			hsv.s = CLOCK_IMG_SATURATION;
			if(0 > pos) pos += 60;
			hsv.v = (maxVal / 4) * (hourSteps - cntHourImg) / hourSteps;
			hsv2rgb(&pt[pos], &hsv);

			pos++;
			if(60 <= pos) pos -= 60;
			hsv.v = (maxVal / 4) + (maxVal * 3 / 4) * (hourSteps - cntHourImg) / hourSteps; 
			hsv2rgb(&pt[pos], &hsv);

			pos++;
			if(60 <= pos) pos -= 60;
			hsv.v = (maxVal / 4) + (maxVal * 3 / 4) * cntHourImg / hourSteps;
			hsv2rgb(&pt[pos], &hsv);

			pos++;
			if(60 <= pos) pos -= 60;
			hsv.v = (maxVal / 4) * cntHourImg / hourSteps;
			hsv2rgb(&pt[pos], &hsv);

			cntHourImg++;
			bSet = false;
		}
	}
	if(bSet) {
		pos = hour - 1;
		hsv.h = HUE_HOUR;	
		hsv.s = CLOCK_IMG_SATURATION;

		if(0 > pos) pos += 60;
		hsv.v = maxVal / 4;
		hsv2rgb(&pt[pos], &hsv);

		pos++;
		if(60 <= pos) pos -= 60;
		hsv.v = maxVal;
		hsv2rgb(&pt[pos], &hsv);

		pos++;
		if(60 <= pos) pos -= 60;
		hsv.v = maxVal / 4;
		hsv2rgb(&pt[pos], &hsv);
	}
	hourPrv = hour;
}



